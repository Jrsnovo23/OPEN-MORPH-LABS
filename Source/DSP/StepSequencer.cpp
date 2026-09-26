#include "StepSequencer.h"

void StepSequencer::reset()
{
    pendingNote = -1;
    samplesUntilNoteOff = -1;
    currentStep.store (-1);
}

int StepSequencer::computeSeqPos (int absoluteStep) const
{
    const int len = juce::jlimit (1, numSteps, length.load());
    const int dir = juce::jlimit (0, 3, direction.load());
    const int pos = absoluteStep % len;

    switch (dir)
    {
        case 0: return pos;
        case 1: return len - 1 - pos;
        case 2:
        {
            const int period = juce::jmax (2, 2 * len - 2);
            const int p = absoluteStep % period;
            return (p < len) ? p : (period - p);
        }
        case 3:
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

    auto tickScheduledOff = [&] ()
    {
        if (pendingNote >= 0 && samplesUntilNoteOff >= 0)
        {
            if (samplesUntilNoteOff < numSamples)
            {
                Event off;
                off.type = Event::NoteOff;
                off.sampleOffset = samplesUntilNoteOff;
                off.midiNote = pendingNote;
                off.velocity = 0.0f;
                outEvents.push_back (off);
                pendingNote = -1;
                samplesUntilNoteOff = -1;
            }
            else
            {
                samplesUntilNoteOff -= numSamples;
            }
        }
    };

    if (! on)
    {
        if (pendingNote >= 0)
        {
            Event off;
            off.type = Event::NoteOff;
            off.sampleOffset = 0;
            off.midiNote = pendingNote;
            off.velocity = 0.0f;
            outEvents.push_back (off);
            pendingNote = -1;
            samplesUntilNoteOff = -1;
        }
        currentStep.store (-1);
        return;
    }

    tickScheduledOff();

    // 13 divisiones (mismo orden que Arpeggiator)
    static const double rateBeats[] = {
        4.0, 2.0, 1.0, 0.5, 0.25,
        1.0 * 2.0/3.0, 0.5 * 2.0/3.0, 0.25 * 2.0/3.0,
        1.5, 0.75,
        1.0 / 3.0,
        0.125, 0.0625
    };
    const int ri = juce::jlimit (0, 12, rateIndex.load());
    const double beatsPerStep = rateBeats[ri];

    const double sr       = currentSampleRate;
    const double beatSec  = 60.0 / juce::jmax (1.0, bpm);
    const double stepSec  = beatsPerStep * beatSec;
    const double blockBeats = (double) numSamples / (sr * beatSec);

    const double ppqStart = ppqPosition;
    const double ppqEnd   = ppqPosition + blockBeats;

    const int firstStep = (int) std::floor (ppqStart / beatsPerStep);
    const int lastStep  = (int) std::floor (ppqEnd   / beatsPerStep);

    const int   base      = baseNote.load();
    const float swingAmt  = swing.load();

    juce::Random rng;

    for (int s = firstStep; s <= lastStep; ++s)
    {
        const double stepStartBeat = s * beatsPerStep;
        double stepStartBeatSwung = stepStartBeat;

        if ((s & 1) && swingAmt > 0.0f)
            stepStartBeatSwung += beatsPerStep * 0.5 * swingAmt;

        if (stepStartBeatSwung < ppqStart || stepStartBeatSwung >= ppqEnd)
            continue;

        const int seqPos = computeSeqPos (s);
        currentStep.store (seqPos);

        const double beatOffset = stepStartBeatSwung - ppqStart;
        const int sampleOffset = juce::jlimit (0, juce::jmax (0, numSamples - 1),
            (int) std::round (beatOffset * sr * beatSec));

        if (pendingNote >= 0)
        {
            Event off;
            off.type = Event::NoteOff;
            off.sampleOffset = sampleOffset;
            off.midiNote = pendingNote;
            off.velocity = 0.0f;
            outEvents.push_back (off);
            pendingNote = -1;
            samplesUntilNoteOff = -1;
        }

        const auto& step = steps[seqPos];
        if (! step.active.load()) continue;

        const float prob = juce::jlimit (0.0f, 1.0f, step.probability.load());
        if (rng.nextFloat() > prob) continue;

        const int midiNote = juce::jlimit (0, 127, base + step.pitch.load());
        const float vel    = juce::jlimit (0.0f, 1.0f, step.velocity.load());
        const float gateAmt = juce::jlimit (0.1f, 2.0f, step.gate.load());

        Event noteOn;
        noteOn.type = Event::NoteOn;
        noteOn.sampleOffset = sampleOffset;
        noteOn.midiNote = midiNote;
        noteOn.velocity = vel;
        outEvents.push_back (noteOn);

        pendingNote = midiNote;
        samplesUntilNoteOff = (int) std::round (stepSec * gateAmt * sr);
    }
}

void StepSequencer::randomizeAll()
{
    juce::Random rng;

    for (int i = 0; i < numSteps; ++i)
    {
        const bool active = rng.nextFloat() < 0.6f;
        steps[i].active.store (active);

        const int pitch = rng.nextInt (25) - 12;
        steps[i].pitch.store (pitch);

        const float vel = 0.5f + rng.nextFloat() * 0.5f;
        steps[i].velocity.store (vel);

        const float gate = 0.2f + rng.nextFloat() * 0.8f;
        steps[i].gate.store (gate);

        const float prob = 0.5f + rng.nextFloat() * 0.5f;
        steps[i].probability.store (prob);
    }
}

void StepSequencer::clearAll()
{
    for (int i = 0; i < numSteps; ++i)
    {
        steps[i].active.store      (false);
        steps[i].pitch.store       (0);
        steps[i].velocity.store    (0.8f);
        steps[i].gate.store        (1.0f);
        steps[i].probability.store (1.0f);
    }
}

void StepSequencer::shiftLeft()
{
    const bool  lastActive = steps[0].active.load();
    const int   lastPitch  = steps[0].pitch.load();
    const float lastVel    = steps[0].velocity.load();
    const float lastGate   = steps[0].gate.load();
    const float lastProb   = steps[0].probability.load();

    for (int i = 0; i < numSteps - 1; ++i)
    {
        steps[i].active.store      (steps[i + 1].active.load());
        steps[i].pitch.store       (steps[i + 1].pitch.load());
        steps[i].velocity.store    (steps[i + 1].velocity.load());
        steps[i].gate.store        (steps[i + 1].gate.load());
        steps[i].probability.store (steps[i + 1].probability.load());
    }

    steps[numSteps - 1].active.store      (lastActive);
    steps[numSteps - 1].pitch.store       (lastPitch);
    steps[numSteps - 1].velocity.store    (lastVel);
    steps[numSteps - 1].gate.store        (lastGate);
    steps[numSteps - 1].probability.store (lastProb);
}

void StepSequencer::shiftRight()
{
    const bool  lastActive = steps[numSteps - 1].active.load();
    const int   lastPitch  = steps[numSteps - 1].pitch.load();
    const float lastVel    = steps[numSteps - 1].velocity.load();
    const float lastGate   = steps[numSteps - 1].gate.load();
    const float lastProb   = steps[numSteps - 1].probability.load();

    for (int i = numSteps - 1; i > 0; --i)
    {
        steps[i].active.store      (steps[i - 1].active.load());
        steps[i].pitch.store       (steps[i - 1].pitch.load());
        steps[i].velocity.store    (steps[i - 1].velocity.load());
        steps[i].gate.store        (steps[i - 1].gate.load());
        steps[i].probability.store (steps[i - 1].probability.load());
    }

    steps[0].active.store      (lastActive);
    steps[0].pitch.store       (lastPitch);
    steps[0].velocity.store    (lastVel);
    steps[0].gate.store        (lastGate);
    steps[0].probability.store (lastProb);
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
        s.setProperty ("index",      i,                            nullptr);
        s.setProperty ("active",     steps[i].active.load(),       nullptr);
        s.setProperty ("pitch",      steps[i].pitch.load(),        nullptr);
        s.setProperty ("velocity",   steps[i].velocity.load(),     nullptr);
        s.setProperty ("gate",       steps[i].gate.load(),         nullptr);
        s.setProperty ("probability",steps[i].probability.load(),  nullptr);
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

        steps[idx].active.store      ((bool)  c.getProperty ("active",      false));
        steps[idx].pitch.store       ((int)   c.getProperty ("pitch",       0));
        steps[idx].velocity.store    ((float) c.getProperty ("velocity",    0.8f));
        steps[idx].gate.store        ((float) c.getProperty ("gate",        1.0f));
        steps[idx].probability.store ((float) c.getProperty ("probability", 1.0f));
    }
}
