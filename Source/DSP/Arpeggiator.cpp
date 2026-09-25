#include "Arpeggiator.h"

void Arpeggiator::reset()
{
    currentNote = -1;
    samplesUntilNoteOff = -1;
    currentSequenceIndex = 0;
    upDownDirection = 1;
    currentStepIndex.store (-1);
    latchedNotes.fill (false);
    anyNoteHeldPrevBlock = false;
}

std::vector<int> Arpeggiator::buildNoteList (const juce::MidiKeyboardState& state) const
{
    std::vector<int> notes;
    const bool useLatch = latch.load();

    // 1) Recolectar notas sostenidas
    for (int n = 0; n < 128; ++n)
    {
        bool isOn = state.isNoteOn (1, n);

        if (isOn && useLatch)
            const_cast<Arpeggiator*> (this)->latchedNotes[(size_t) n] = true;

        const bool keep = isOn || (useLatch && latchedNotes[(size_t) n]);
        if (keep) notes.push_back (n);
    }

    // Si latch está activo y no hay ninguna nota nueva pulsada, usar el latched
    if (useLatch && notes.empty())
    {
        for (int n = 0; n < 128; ++n)
            if (latchedNotes[(size_t) n])
                notes.push_back (n);
    }

    // Si no hay latch y no hay notas, limpiar todo
    if (! useLatch && notes.empty())
    {
        // No hay nada que reproducir
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

    // 2) Detectar si hay notas nuevas
    bool anyHeldNow = false;
    for (int n = 0; n < 128; ++n)
    {
        if (keyboardState.isNoteOn (1, n)) { anyHeldNow = true; break; }
    }

    // Si latch y no hay ninguna pulsada, permitimos que siga sonando el latched.
    // Si latch está apagado y no hay notas, limpiamos latchedNotes para que
    // la próxima nota empiece de cero.
    if (! latch.load() && ! anyHeldNow)
    {
        latchedNotes.fill (false);
    }

    // Si estamos en latch y el usuario acaba de pulsar (transición de "nada" a "algo"),
    // limpiamos el latched para empezar un nuevo acorde.
    if (latch.load() && anyHeldNow && ! anyNoteHeldPrevBlock)
    {
        latchedNotes.fill (false);
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

    // Detectar cruce de frontera de paso (entre ppqStart y ppqEnd)
    const double blockBeats = (double) numSamples / (sr * beatSec);
    const double ppqStart = ppqPosition;
    const double ppqEnd   = ppqPosition + blockBeats;

    const int firstStep = (int) std::floor (ppqStart / beatsPerStep);
    const int lastStep  = (int) std::floor (ppqEnd   / beatsPerStep);

    bool advance = false;
    int  sampleOffset = 0;

    for (int s = firstStep; s <= lastStep; ++s)
    {
        const double stepStartBeat = s * beatsPerStep;
        if (stepStartBeat < ppqStart || stepStartBeat >= ppqEnd) continue;

        const double beatOffset = stepStartBeat - ppqStart;
        sampleOffset = juce::jlimit (0, juce::jmax (0, numSamples - 1),
            (int) std::round (beatOffset * sr * beatSec));
        advance = true;
        break;
    }

    if (! advance) return;

    // 4) Construir lista de notas y elegir la siguiente
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

    const int modeIdx = juce::jlimit (0, 3, mode.load());

    int chosenIndex = 0;
    switch (modeIdx)
    {
        case 0: // Up
            chosenIndex = currentSequenceIndex % totalLen;
            currentSequenceIndex = (currentSequenceIndex + 1) % totalLen;
            break;

        case 1: // Down
            chosenIndex = (totalLen - 1) - (currentSequenceIndex % totalLen);
            currentSequenceIndex = (currentSequenceIndex + 1) % totalLen;
            break;

        case 2: // UpDown
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

        case 3: // Random
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

    // 6) Disparar nuevo NoteOn
    const float vel = 0.85f;

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
    latch.store     ((bool)  v.getProperty ("latch",     false));
}
