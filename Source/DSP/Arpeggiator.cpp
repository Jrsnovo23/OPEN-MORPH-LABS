#include "Arpeggiator.h"

void Arpeggiator::reset()
{
    currentNote = -1;
    samplesUntilNoteOff = -1;
    currentSequenceIndex = 0;
    upDownDirection = 1;
    currentStepIndex.store (-1);
    latchedNotes.fill (false);
    playOrder.fill (0);
    playOrderCount = 0;
    chordNotesHeld.clear();
    anyNoteHeldPrevBlock = false;
}

std::vector<int> Arpeggiator::buildNoteList (const juce::MidiKeyboardState& state)
{
    std::vector<int> notes;

    const bool useLatch = latch.load();
    const int  modeIdx  = juce::jlimit (0, 6, mode.load());

    for (int n = 0; n < 128; ++n)
    {
        const bool isOn = state.isNoteOn (1, n);
        if (isOn && useLatch)
            latchedNotes[(size_t) n] = true;
    }

    for (int n = 0; n < 128; ++n)
    {
        const bool isOn = state.isNoteOn (1, n);
        const bool keep = isOn || (useLatch && latchedNotes[(size_t) n]);
        if (keep) notes.push_back (n);
    }

    switch (modeIdx)
    {
        case (int) Mode::Up:
        case (int) Mode::UpDown:
            break;

        case (int) Mode::Down:
        case (int) Mode::DownUp:
            std::reverse (notes.begin(), notes.end());
            break;

        case (int) Mode::Random:
            break;

        case (int) Mode::AsPlayed:
        {
            std::vector<int> ordered;
            for (int i = 0; i < playOrderCount; ++i)
            {
                const int n = playOrder[(size_t) i];
                if (std::find (notes.begin(), notes.end(), n) != notes.end())
                    ordered.push_back (n);
            }
            for (int n : notes)
                if (std::find (ordered.begin(), ordered.end(), n) == ordered.end())
                    ordered.push_back (n);

            if (! ordered.empty()) notes = ordered;
            break;
        }

        case (int) Mode::Chord:
            break;
    }

    return notes;
}

int Arpeggiator::getSequenceLength (int numBaseNotes) const
{
    const int octs = juce::jlimit (1, 4, octaves.load());
    return numBaseNotes * octs;
}

int Arpeggiator::getNoteAtIndex (const std::vector<int>& baseNotes, int seqIndex) const
{
    if (baseNotes.empty()) return -1;

    const int N = (int) baseNotes.size();
    const int octs = juce::jlimit (1, 4, octaves.load());
    const int totalLen = N * octs;

    if (totalLen <= 0) return -1;

    const int idx = ((seqIndex % totalLen) + totalLen) % totalLen;

    const int octIdx = idx / N;
    const int baseIdx = idx % N;

    const int baseNote = baseNotes[(size_t) baseIdx];
    const int shifted = baseNote + octIdx * 12;

    return juce::jlimit (0, 127, shifted);
}

void Arpeggiator::process (double bpm,
                           double ppqPosition,
                           bool isPlaying,
                           int numSamples,
                           const juce::MidiKeyboardState& keyboardState,
                           std::vector<Event>& outEvents)
{
    outEvents.clear();

    const bool on = enabled.load() && isPlaying && ppqPosition >= 0.0;

    // Función auxiliar: apagar todas las notas activas (single + chord)
    auto flushAllNotes = [&] (int atSample)
    {
        if (currentNote >= 0)
        {
            Event off;
            off.type = Event::NoteOff;
            off.sampleOffset = atSample;
            off.midiNote = currentNote;
            off.velocity = 0.0f;
            outEvents.push_back (off);
            currentNote = -1;
            samplesUntilNoteOff = -1;
        }

        for (int n : chordNotesHeld)
        {
            Event off;
            off.type = Event::NoteOff;
            off.sampleOffset = atSample;
            off.midiNote = n;
            off.velocity = 0.0f;
            outEvents.push_back (off);
        }
        chordNotesHeld.clear();
    };

    if (! on)
    {
        flushAllNotes (0);
        currentStepIndex.store (-1);
        return;
    }

    // 1) Tick del NoteOff pendiente (solo notas individuales)
    if (currentNote >= 0 && samplesUntilNoteOff >= 0)
    {
        if (samplesUntilNoteOff < numSamples)
        {
            Event off;
            off.type = Event::NoteOff;
            off.sampleOffset = samplesUntilNoteOff;
            off.midiNote = currentNote;
            off.velocity = 0.0f;
            outEvents.push_back (off);
            currentNote = -1;
            samplesUntilNoteOff = -1;
        }
        else
        {
            samplesUntilNoteOff -= numSamples;
        }
    }

    // 2) Detectar cambios en las notas sostenidas
    bool anyHeldNow = false;
    for (int n = 0; n < 128; ++n)
    {
        if (keyboardState.isNoteOn (1, n)) { anyHeldNow = true; break; }
    }

    if (! latch.load() && ! anyHeldNow)
    {
        latchedNotes.fill (false);
        playOrderCount = 0;
    }

    if (latch.load() && anyHeldNow && ! anyNoteHeldPrevBlock)
    {
        latchedNotes.fill (false);
        playOrderCount = 0;
    }

    for (int n = 0; n < 128; ++n)
    {
        if (keyboardState.isNoteOn (1, n))
        {
            bool already = false;
            for (int i = 0; i < playOrderCount; ++i)
                if (playOrder[(size_t) i] == n) { already = true; break; }

            if (! already && playOrderCount < 128)
                playOrder[(size_t) playOrderCount++] = n;
        }
    }

    anyNoteHeldPrevBlock = anyHeldNow;

    // 3) Rates (13 divisiones)
    static const double rateBeats[] = {
        4.0, 2.0, 1.0, 0.5, 0.25,                 // 1/1, 1/2, 1/4, 1/8, 1/16
        1.0 * 2.0/3.0, 0.5 * 2.0/3.0, 0.25 * 2.0/3.0,  // 1/4T, 1/8T, 1/16T
        1.5, 0.75,                                 // 1/4., 1/8.
        1.0 / 3.0,                                 // 1/12
        0.125, 0.0625                              // 1/32, 1/64
    };
    const int ri = juce::jlimit (0, 12, rateIndex.load());
    const double beatsPerStep = rateBeats[ri];

    const double sr = currentSampleRate;
    const double beatSec = 60.0 / juce::jmax (1.0, bpm);
    const double stepSec = beatsPerStep * beatSec;

    const double blockBeats = (double) numSamples / (sr * beatSec);
    const double ppqStart = ppqPosition;
    const double ppqEnd   = ppqPosition + blockBeats;

    const int firstStep = (int) std::floor (ppqStart / beatsPerStep);
    const int lastStep  = (int) std::floor (ppqEnd   / beatsPerStep);

    const float swingAmt = swing.load();

    bool advance = false;
    int  sampleOffset = 0;

    for (int s = firstStep; s <= lastStep; ++s)
    {
        double stepStartBeat = s * beatsPerStep;
        if ((s & 1) && swingAmt > 0.0f)
            stepStartBeat += beatsPerStep * 0.5 * swingAmt;

        if (stepStartBeat < ppqStart || stepStartBeat >= ppqEnd) continue;

        const double beatOffset = stepStartBeat - ppqStart;
        sampleOffset = juce::jlimit (0, juce::jmax (0, numSamples - 1),
            (int) std::round (beatOffset * sr * beatSec));
        advance = true;
        break;
    }

    if (! advance) return;

    // 4) Construir lista de notas
    const auto baseNotes = buildNoteList (keyboardState);
    if (baseNotes.empty())
    {
        currentStepIndex.store (-1);
        currentSequenceIndex = 0;
        return;
    }

    const int totalLen = getSequenceLength ((int) baseNotes.size());
    if (totalLen <= 0)
    {
        currentStepIndex.store (-1);
        return;
    }

    const int modeIdx = juce::jlimit (0, 6, mode.load());

    // 5) Apagar notas anteriores en el mismo sample
    //    (individual + chord)
    if (currentNote >= 0)
    {
        Event off;
        off.type = Event::NoteOff;
        off.sampleOffset = sampleOffset;
        off.midiNote = currentNote;
        off.velocity = 0.0f;
        outEvents.push_back (off);
        currentNote = -1;
        samplesUntilNoteOff = -1;
    }
    for (int n : chordNotesHeld)
    {
        Event off;
        off.type = Event::NoteOff;
        off.sampleOffset = sampleOffset;
        off.midiNote = n;
        off.velocity = 0.0f;
        outEvents.push_back (off);
    }
    chordNotesHeld.clear();

    const float vel = 0.85f;

    // 6) CHORD: disparar todas las notas y guardarlas para apagarlas luego
    if (modeIdx == (int) Mode::Chord)
    {
        const int octs = juce::jlimit (1, 4, octaves.load());

        for (size_t i = 0; i < baseNotes.size(); ++i)
        {
            for (int o = 0; o < octs; ++o)
            {
                const int n = juce::jlimit (0, 127, baseNotes[i] + o * 12);

                Event noteOn;
                noteOn.type = Event::NoteOn;
                noteOn.sampleOffset = sampleOffset;
                noteOn.midiNote = n;
                noteOn.velocity = vel;
                outEvents.push_back (noteOn);

                chordNotesHeld.push_back (n);
            }
        }
        currentStepIndex.store (0);
        return;
    }

    // 7) Resto de modos: nota individual con NoteOff programado
    int chosenIndex = 0;
    switch (modeIdx)
    {
        case (int) Mode::Up:
        case (int) Mode::AsPlayed:
            chosenIndex = currentSequenceIndex % totalLen;
            currentSequenceIndex = (currentSequenceIndex + 1) % totalLen;
            break;

        case (int) Mode::Down:
            chosenIndex = (totalLen - 1) - (currentSequenceIndex % totalLen);
            currentSequenceIndex = (currentSequenceIndex + 1) % totalLen;
            break;

        case (int) Mode::UpDown:
        {
            chosenIndex = juce::jlimit (0, totalLen - 1, currentSequenceIndex);
            if (totalLen > 1)
            {
                if (upDownDirection > 0)
                {
                    if (chosenIndex >= totalLen - 1) { upDownDirection = -1; chosenIndex = totalLen - 2; }
                    else chosenIndex++;
                }
                else
                {
                    if (chosenIndex <= 0) { upDownDirection = 1; chosenIndex = 1; }
                    else chosenIndex--;
                }
            }
            currentSequenceIndex = chosenIndex;
            break;
        }

        case (int) Mode::DownUp:
        {
            chosenIndex = juce::jlimit (0, totalLen - 1, currentSequenceIndex);
            if (totalLen > 1)
            {
                if (upDownDirection > 0)
                {
                    if (chosenIndex <= 0) { upDownDirection = -1; chosenIndex = 1; }
                    else chosenIndex--;
                }
                else
                {
                    if (chosenIndex >= totalLen - 1) { upDownDirection = 1; chosenIndex = totalLen - 2; }
                    else chosenIndex++;
                }
            }
            currentSequenceIndex = chosenIndex;
            break;
        }

        case (int) Mode::Random:
        {
            juce::Random r;
            chosenIndex = r.nextInt (totalLen);
            currentSequenceIndex = chosenIndex;
            break;
        }
    }

    currentStepIndex.store (chosenIndex);

    const int midiNote = getNoteAtIndex (baseNotes, chosenIndex);
    if (midiNote < 0) return;

    Event noteOn;
    noteOn.type = Event::NoteOn;
    noteOn.sampleOffset = sampleOffset;
    noteOn.midiNote = midiNote;
    noteOn.velocity = vel;
    outEvents.push_back (noteOn);

    currentNote = midiNote;

    const float gateAmt = juce::jlimit (0.1f, 1.0f, gate.load());
    samplesUntilNoteOff = (int) std::round (stepSec * gateAmt * sr);
}

juce::ValueTree Arpeggiator::toValueTree() const
{
    juce::ValueTree v ("ARPEGGIATOR");
    v.setProperty ("enabled",   enabled.load(),   nullptr);
    v.setProperty ("mode",      mode.load(),      nullptr);
    v.setProperty ("octaves",   octaves.load(),   nullptr);
    v.setProperty ("rateIndex", rateIndex.load(), nullptr);
    v.setProperty ("gate",      gate.load(),      nullptr);
    v.setProperty ("swing",     swing.load(),     nullptr);
    v.setProperty ("latch",     latch.load(),     nullptr);
    return v;
}

void Arpeggiator::fromValueTree (const juce::ValueTree& v)
{
    if (! v.hasType ("ARPEGGIATOR")) return;

    enabled.store   ((bool)  v.getProperty ("enabled",   false));
    mode.store      ((int)   v.getProperty ("mode",      0));
    octaves.store   ((int)   v.getProperty ("octaves",   1));
    rateIndex.store ((int)   v.getProperty ("rateIndex", 4));
    gate.store      ((float) v.getProperty ("gate",      0.5f));
    swing.store     ((float) v.getProperty ("swing",     0.0f));
    latch.store     ((bool)  v.getProperty ("latch",     false));
}
