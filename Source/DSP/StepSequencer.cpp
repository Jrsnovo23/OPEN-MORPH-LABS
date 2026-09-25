#include "StepSequencer.h"

void StepSequencer::reset()
{
    pendingNote = -1;
    currentStep.store (-1);
}

int StepSequencer::computeSeqPos (int absoluteStep) const
{
    const int len = juce::jlimit (1, numSteps, length.load());
    const int dir = juce::jlimit (0, 3, direction.load());
    const int pos = absoluteStep % len;

    switch (dir)
    {
        case 0: return pos;                         // Fwd
        case 1: return len - 1 - pos;               // Rev
        case 2:                                     // PingPong
        {
            const int period = juce::jmax (2, 2 * len - 2);
            const int p = absoluteStep % period;
            return (p < len) ? p : (period - p);
        }
        case 3:                                     // Random
        {
            juce::uint32 h = (juce::uint32) absoluteStep * 2654435761u;
            h ^= h >> 16;
            return (int) (h % (juce::uint32) len);
        }
    }
    return 0;
}

void StepSequencer::process (double bpm,
                             double ppqPosition,
                             bool isPlaying,
                             int numSamples,
                             std::vector<Event>& outEvents)
{
    outEvents.clear();

    const bool on = enabled.load() && isPlaying && ppqPosition >= 0.0;

    if (! on)
    {
        // Si había una nota pendiente, la apagamos
        if (pendingNote >= 0)
        {
            Event off;
            off.type = Event::NoteOff;
            off.sampleOffset = 0;
            off.midiNote = pendingNote;
            off.velocity = 0.0f;
            outEvents.push_back (off);
            pendingNote = -1;
        }
        currentStep.store (-1);
        return;
    }

    static const double rateBeats[] = {
        4.0, 2.0, 1.0, 0.5, 0.25,
        1.0 * 2.0/3.0, 0.5 * 2.0/3.0, 0.25 * 2.0/3.0,
        1.5
    };
    const int ri = juce::jlimit (0, 8, rateIndex.load());
    const double beatsPerStep = rateBeats[ri];

    const double sr       = currentSampleRate;
    const double beatSec  = 60.0 / juce::jmax (1.0, bpm);
    const double blockBeats = (double) numSamples / (sr * beatSec);

    const double ppqStart = ppqPosition;
    const double ppqEnd   = ppqPosition + blockBeats;

    const int firstStep = (int) std::floor (ppqStart / beatsPerStep);
    const int lastStep  = (int) std::floor (ppqEnd   / beatsPerStep);

    const int   base      = baseNote.load();
    const float swingAmt  = swing.load();

    for (int s = firstStep; s <= lastStep; ++s)
    {
        const double stepStartBeat = s * beatsPerStep;
        double stepStartBeatSwung = stepStartBeat;

        // Swing: retrasa los pasos impares
        if ((s & 1) && swingAmt > 0.0f)
            stepStartBeatSwung += beatsPerStep * 0.5 * swingAmt;

        if (stepStartBeatSwung < ppqStart || stepStartBeatSwung >= ppqEnd)
            continue;

        const int seqPos = computeSeqPos (s);
        currentStep.store (seqPos);

        const double beatOffset = stepStartBeatSwung - ppqStart;
        const int sampleOffset = juce::jlimit (0, numSamples - 1,
            (int) std::round (beatOffset * sr * beatSec));

        // Apagar nota anterior en el mismo sample (monofónico)
        if (pendingNote >= 0)
        {
            Event off;
            off.type = Event::NoteOff;
            off.sampleOffset = sampleOffset;
            off.midiNote = pendingNote;
            off.velocity = 0.0f;
            outEvents.push_back (off);
            pendingNote = -1;
        }

        const auto& step = steps[seqPos];
        if (! step.active.load()) continue;

        const int midiNote = juce::jlimit (0, 127, base + step.pitch.load());
        const float vel    = juce::jlimit (0.0f, 1.0f, step.velocity.load());

        Event on;
        on.type = Event::NoteOn;
        on.sampleOffset = sampleOffset;
        on.midiNote = midiNote;
        on.velocity = vel;
        outEvents.push_back (on);

        pendingNote = midiNote;
    }
}

juce::ValueTree StepSequencer::toValueTree() const
{
    juce::ValueTree v ("SEQUENCER");
    v.setProperty ("enabled",   enabled.load(),   nullptr);
    v.setProperty ("rateIndex", rateIndex.load(), nullptr);
    v.setProperty ("swing",     swing.load(),     nullptr);
    v.setProperty ("length",    length.load(),    nullptr);
    v.setProperty ("direction", direction.load(), nullptr);
    v.setProperty ("baseNote",  baseNote.load(),  nullptr);

    for (int i = 0; i < numSteps; ++i)
    {
        juce::ValueTree s ("STEP");
        s.setProperty ("index",    i,                        nullptr);
        s.setProperty ("active",   steps[i].active.load(),   nullptr);
        s.setProperty ("pitch",    steps[i].pitch.load(),    nullptr);
        s.setProperty ("velocity", steps[i].velocity.load(), nullptr);
        v.addChild (s, -1, nullptr);
    }

    return v;
}

void StepSequencer::fromValueTree (const juce::ValueTree& v)
{
    if (! v.hasType ("SEQUENCER")) return;

    enabled.store   ((bool)  v.getProperty ("enabled",   false));
    rateIndex.store ((int)   v.getProperty ("rateIndex", 3));
    swing.store     ((float) v.getProperty ("swing",     0.0f));
    length.store    ((int)   v.getProperty ("length",    16));
    direction.store ((int)   v.getProperty ("direction", 0));
    baseNote.store  ((int)   v.getProperty ("baseNote",  60));

    for (int i = 0; i < v.getNumChildren(); ++i)
    {
        auto c = v.getChild (i);
        if (! c.hasType ("STEP")) continue;
        const int idx = (int) c.getProperty ("index", -1);
        if (idx < 0 || idx >= numSteps) continue;

        steps[idx].active.store   ((bool)  c.getProperty ("active",   false));
        steps[idx].pitch.store    ((int)   c.getProperty ("pitch",    0));
        steps[idx].velocity.store ((float) c.getProperty ("velocity", 0.8f));
    }
}
