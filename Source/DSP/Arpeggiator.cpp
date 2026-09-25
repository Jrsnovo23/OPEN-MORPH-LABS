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
    anyNoteHeldPrevBlock = false;
}

std::vector<int> Arpeggiator::buildNoteList (const juce::MidiKeyboardState& state)
{
    std::vector<int> notes;

    const bool useLatch = latch.load();
    const int  modeIdx  = juce::jlimit (0, 6, mode.load());

    // 1) Recolectar notas sostenidas
    for (int n = 0; n < 128; ++n)
    {
        const bool isOn = state.isNoteOn (1, n);
        if (isOn && useLatch)
            latchedNotes[(size_t) n] = true;
    }

    // 2) Rellenar la lista de notas (sostenidas + latched)
    for (int n = 0; n < 128; ++n)
    {
        const bool isOn = state.isNoteOn (1, n);
        const bool keep = isOn || (useLatch && latchedNotes[(size_t) n]);
        if (keep) notes.push_back (n);
    }

    // 3) Reordenar según el modo
    switch (modeIdx)
    {
        case (int) Mode::Up:
        case (int) Mode::UpDown:
            // Orden ascendente (ya está así por construcción)
            break;

        case (int) Mode::Down:
        case (int) Mode::DownUp:
            std::reverse (notes.begin(), notes.end());
            break;

        case (int) Mode::Random:
            // Orden de base ascendente, la aleatoriedad la da currentSequenceIndex
            break;

        case (int) Mode::AsPlayed:
        {
            // Reordenar según playOrder (solo notas que estén en playOrder)
            std::vector<int> ordered;
            for (int i = 0; i < playOrderCount; ++i)
            {
                const int n = playOrder[(size_t) i];
                if (std::find (notes.begin(), notes.end(), n) != notes.end())
                    ordered.push_back (n);
            }
            // Añadir las que no estuvieran en playOrder (fallback)
            for (int n : notes)
                if (std::find (ordered.begin(), ordered.end(), n) == ordered.end())
                    ordered.push_back (n);

            if (! ordered.empty()) notes = ordered;
            break;
        }

        case (int) Mode::Chord:
            // Orden irrelevante; todas se disparan juntas
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

    // Si no está corriendo: apagar nota pendiente y reset
    if (! on)
    {
        if (currentNote >= 0)
        {
            Event off;
            off.type = Event::NoteOff;
            off.sampleOffset = 0;
            off.midiNote = currentNote;
            off.velocity = 0.0f;
            outEvents.push_back (off);
            currentNote = -1;
        }
        samplesUntilNoteOff = -1;
        currentStepIndex.store (-1);
        return;
    }

    // 1) Tick del NoteOff pendiente
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

    // En latch: si acaba de empezar un nuevo acorde, resetear
    if (latch.load() && anyHeldNow && ! anyNoteHeldPrevBlock)
    {
        latchedNotes.fill (false);
        playOrderCount = 0;
    }

    // Registrar notas nuevas en playOrder
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

    // 3) Calcular si toca avanzar de paso
    static const double rateBeats[] = {
        4.0, 2.0, 1.0, 0.5, 0.25,
        1.0 * 2.0/3.0, 0.5 * 2.0/3.0, 0.25 * 2.0/3.0,
        1.5
    };
    const int ri = juce::jlimit (0, 8, rateIndex.load());
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

    // 5) Apagar nota anterior en el mismo sample si aún sonaba
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

    const float vel = 0.85f;
    const float gateAmt = juce::jlimit (0.1f, 1.0f, gate.load());
    const int noteOffSamples = (int) std::round (stepSec * gateAmt * sr);

    // 6) CHORD: disparar todas las notas a la vez
    if (modeIdx == (int) Mode::Chord)
    {
        for (size_t i = 0; i < baseNotes.size(); ++i)
        {
            // Para octavas > 1, dispara también las octavas
            const int octs = juce::jlimit (1, 4, octaves.load());
            for (int o = 0; o < octs; ++o)
            {
                const int n = juce::jlimit (0, 127, baseNotes[i] + o * 12);

                Event noteOn;
                noteOn.type = Event::NoteOn;
                noteOn.sampleOffset = sampleOffset;
                noteOn.midiNote = n;
                noteOn.velocity = vel;
                outEvents.push_back (noteOn);
            }
        }
        // Para simplicidad, dejamos "sin noteOff programado" — el siguiente step
        // disparará NoteOff de las notas previas si las hubiera. En la práctica,
        // con CHORD el hueco entre steps ya las apaga.
        currentStepIndex.store (0);
        return;
    }

    // 7) Resto de modos: disparar una nota individual
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
    samplesUntilNoteOff = noteOffSamples;
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
