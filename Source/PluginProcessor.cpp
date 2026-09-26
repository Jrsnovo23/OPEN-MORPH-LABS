#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Params/ParameterLayout.h"
#include "Synth/SynthSound.h"
#include "Synth/SynthVoice.h"
#include "ParameterIDs.h"

PPGWave3Processor::PPGWave3Processor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", Params::createLayout())
{
    for (int i = 0; i < 8; ++i)
    {
        auto* voice = new synth::SynthVoice (apvts);
        voice->setBpmSource (&currentBpm);
        voice->setPhaseTargets (&osc1Phase, &osc2Phase, &lfo1Phase, &lfo2Phase);
        synth.addVoice (voice);
    }

    synth.addSound (new synth::SynthSound());
}

void PPGWave3Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    synth.setCurrentPlaybackSampleRate (sampleRate);
    effects.prepare (sampleRate, samplesPerBlock, 2);

    // Analizador de espectro
    fftFifoIndex.store (0);
    std::fill (fftFifo.begin(), fftFifo.end(), 0.0f);
    std::fill (fftBuffer.begin(), fftBuffer.end(), 0.0f);
    for (auto& m : spectrumMagnitudes) m.store (-90.0f);

    // Secuenciador
    sequencer.prepare (sampleRate);
    sequencer.reset();

    // Arpegiador
    arpeggiator.prepare (sampleRate);
    arpeggiator.reset();

    // FASE 13d: resetear el estado del clock
    ppqVirtual = 0.0;
    freeStartTime = 0.0;
    lastClockModeSeen = clockMode.load();
}

bool PPGWave3Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo()
        || out == juce::AudioChannelSet::mono();
}

void PPGWave3Processor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // ===== Leer info del host =====
    double hostBpm      = 120.0;
    double hostPpq      = -1.0;
    bool   hostPpqValid = false;
    bool   hostPlaying  = false;

    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto bpm = pos->getBpm())       { hostBpm = *bpm; }
            if (auto ppq = pos->getPpqPosition()) { hostPpq = *ppq; hostPpqValid = true; }
            hostPlaying = pos->getIsPlaying();
        }
    }

    // ===== FASE 13d: resolver BPM/PPQ/Playing efectivos =====
    const int mode = clockMode.load();

    if (mode != lastClockModeSeen)
    {
        ppqVirtual    = 0.0;
        freeStartTime = 0.0;
        lastClockModeSeen = mode;
    }

    double effectiveBpm;
    double effectivePpq;
    bool   effectivePlaying;

    if (mode == 0)  // LINK
    {
        effectiveBpm     = hostBpm;
        effectivePpq     = hostPpqValid ? hostPpq : -1.0;
        effectivePlaying = hostPlaying;
    }
    else            // FREE
    {
        effectiveBpm = juce::jlimit (20.0, 300.0, (double) freeBpm.load());

        // PPQ virtual basado en el reloj real del sistema
        const double now = juce::Time::getMillisecondCounterHiRes() / 1000.0;

        if (freeStartTime == 0.0)
            freeStartTime = now;

        const double elapsed = now - freeStartTime;
        const double beatSec = 60.0 / juce::jmax (1.0, effectiveBpm);
        ppqVirtual = elapsed / beatSec;

        effectivePpq     = ppqVirtual;
        effectivePlaying = true;
    }

    // Este BPM lo ven LFOs (SynthVoice) y Delay sync (Effects)
    currentBpm.store (effectiveBpm);

    // ---- Guardar eventos MIDI de entrada ----
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
            keyboardState.noteOn (msg.getChannel(), msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff())
            keyboardState.noteOff (msg.getChannel(), msg.getNoteNumber(), msg.getFloatVelocity());
    }

    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);

    // ---- Fase 10: secuenciador ----
    {
        std::vector<StepSequencer::Event> seqEvents;
        sequencer.process (effectiveBpm, effectivePpq, effectivePlaying,
                           buffer.getNumSamples(), seqEvents);

        for (const auto& e : seqEvents)
        {
            if (e.type == StepSequencer::Event::NoteOn)
                midi.addEvent (juce::MidiMessage::noteOn (1, e.midiNote, e.velocity),
                               e.sampleOffset);
            else
                midi.addEvent (juce::MidiMessage::noteOff (1, e.midiNote),
                               e.sampleOffset);
        }
    }

    // ---- Fase 13: arpegiador ----
    {
        std::vector<Arpeggiator::Event> arpEvents;
        arpeggiator.process (effectiveBpm, effectivePpq, effectivePlaying,
                             buffer.getNumSamples(), keyboardState, arpEvents);

        for (const auto& e : arpEvents)
        {
            if (e.type == Arpeggiator::Event::NoteOn)
                midi.addEvent (juce::MidiMessage::noteOn (1, e.midiNote, e.velocity),
                               e.sampleOffset);
            else
                midi.addEvent (juce::MidiMessage::noteOff (1, e.midiNote),
                               e.sampleOffset);
        }
    }

    // ---- Pitch bend y mod wheel ----
    {
        const int pbValue = juce::jlimit (0, 16383,
            (int) std::lround (8192.0f + pitchBendAtomic.load() * 8192.0f));
        midi.addEvent (juce::MidiMessage::pitchWheel (1, pbValue), 0);

        const int mwValue = juce::jlimit (0, 127,
            (int) std::lround (modWheelAtomic.load() * 127.0f));
        midi.addEvent (juce::MidiMessage::controllerEvent (1, 1, mwValue), 0);
    }

    synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());

    // ---- Parámetros de los efectos ----
    auto getF = [&] (const char* id, float def) -> float
    {
        if (auto* p = apvts.getRawParameterValue (id)) return p->load();
        return def;
    };
    auto getB = [&] (const char* id, bool def) -> bool
    {
        if (auto* p = apvts.getRawParameterValue (id)) return p->load() > 0.5f;
        return def;
    };
    auto getI = [&] (const char* id, int def) -> int
    {
        if (auto* p = apvts.getRawParameterValue (id)) return (int) p->load();
        return def;
    };

    effects.setEQ (getB (ParamIDs::eqOn,   true),
                   getB (ParamIDs::eqHpOn, false),
                   getB (ParamIDs::eqLpOn, false),
                   getF (ParamIDs::eqLowFreq,  100.0f),
                   getF (ParamIDs::eqLowQ,     0.707f),
                   getF (ParamIDs::eqLowGain,  0.0f),
                   getF (ParamIDs::eqLmidFreq, 500.0f),
                   getF (ParamIDs::eqLmidQ,    0.707f),
                   getF (ParamIDs::eqLmidGain, 0.0f),
                   getF (ParamIDs::eqHmidFreq, 2000.0f),
                   getF (ParamIDs::eqHmidQ,    0.707f),
                   getF (ParamIDs::eqHmidGain, 0.0f),
                   getF (ParamIDs::eqHighFreq, 8000.0f),
                   getF (ParamIDs::eqHighQ,    0.707f),
                   getF (ParamIDs::eqHighGain, 0.0f));

    effects.setChorus (getB (ParamIDs::chorusOn, false),
                       getF (ParamIDs::chorusRate, 0.5f),
                       getF (ParamIDs::chorusDepth, 0.25f),
                       getF (ParamIDs::chorusMix, 0.5f));

    effects.setPhaser (getB (ParamIDs::phaserOn, false),
                       getF (ParamIDs::phaserRate, 0.5f),
                       getF (ParamIDs::phaserDepth, 0.5f),
                       getF (ParamIDs::phaserFeedback, 0.5f),
                       getF (ParamIDs::phaserMix, 0.5f));

    {
        const int syncIdx = getI (ParamIDs::delaySync, 0);
        float delayTimeSec = getF (ParamIDs::delayTime, 0.3f);

        if (syncIdx > 0)
        {
            const double bpm = currentBpm.load();
            const double beatSec = 60.0 / juce::jmax (1.0, bpm);

            const double divisions[] = {
                4.0, 2.0, 1.0, 0.5, 0.25,
                1.0 * 2.0/3.0, 0.5 * 2.0/3.0, 0.25 * 2.0/3.0,
                1.5, 0.75
            };
            const int divIdx = juce::jlimit (0, 9, syncIdx - 1);
            delayTimeSec = (float) (beatSec * divisions[divIdx]);
        }

        effects.setDelay (getB (ParamIDs::delayOn, false),
                          delayTimeSec,
                          getF (ParamIDs::delayFeedback, 0.4f),
                          getF (ParamIDs::delayMix, 0.3f));
    }

    effects.setReverb (getB (ParamIDs::reverbOn, false),
                       getF (ParamIDs::reverbSize, 0.6f),
                       getF (ParamIDs::reverbDamp, 0.5f),
                       getF (ParamIDs::reverbMix, 0.3f));

    effects.setDrive (getB (ParamIDs::driveOn, false),
                      getF (ParamIDs::driveAmount, 3.0f),
                      getF (ParamIDs::driveTone, 0.5f),
                      getF (ParamIDs::driveMix, 0.5f));

    effects.setVintage (getB (ParamIDs::vintageOn, false),
                        getF (ParamIDs::vintageAmount, 0.5f),
                        getF (ParamIDs::vintageBits,   12.0f),
                        getF (ParamIDs::vintageSr,     1.0f),
                        getF (ParamIDs::vintageNoise,  0.15f));

    effects.setCompressor (getB (ParamIDs::compOn, false),
                           getF (ParamIDs::compThreshold, -12.0f),
                           getF (ParamIDs::compRatio,     4.0f),
                           getF (ParamIDs::compAttack,    10.0f),
                           getF (ParamIDs::compRelease,   100.0f),
                           getF (ParamIDs::compKnee,      6.0f),
                           getF (ParamIDs::compMakeup,    0.0f),
                           getB (ParamIDs::compSidechain, false),
                           getF (ParamIDs::compScAmount,  1.0f),
                           &compressorGR);

    effects.process (buffer);

    // ---- Analizador de espectro ----
    {
        const int numSamples = buffer.getNumSamples();
        const int numCh      = buffer.getNumChannels();
        int fifoIdx = fftFifoIndex.load();

        for (int i = 0; i < numSamples; ++i)
        {
            float sample = 0.0f;
            for (int ch = 0; ch < numCh; ++ch)
                sample += buffer.getReadPointer (ch)[i];
            if (numCh > 1) sample *= 0.5f;

            fftFifo[(size_t) fifoIdx] = sample;
            ++fifoIdx;

            if (fifoIdx >= fftSize)
            {
                std::copy (fftFifo.begin(), fftFifo.end(), fftBuffer.begin());
                std::fill (fftBuffer.begin() + fftSize, fftBuffer.end(), 0.0f);

                window.multiplyWithWindowingTable (fftBuffer.data(), (size_t) fftSize);
                fft.performFrequencyOnlyForwardTransform (fftBuffer.data());

                const float srF = (float) currentSampleRate;
                const float freqMin = 20.0f;
                const float freqMax = 20000.0f;

                for (int b = 0; b < numSpectrumBins; ++b)
                {
                    const float t0 = (float) b       / (float) numSpectrumBins;
                    const float t1 = (float) (b + 1) / (float) numSpectrumBins;
                    const float f0 = freqMin * std::pow (freqMax / freqMin, t0);
                    const float f1 = freqMin * std::pow (freqMax / freqMin, t1);

                    int bin0 = juce::jlimit (0, fftBins - 1,
                        (int) std::round (f0 / srF * (float) fftSize));
                    int bin1 = juce::jlimit (0, fftBins - 1,
                        (int) std::round (f1 / srF * (float) fftSize));
                    if (bin1 < bin0) bin1 = bin0;

                    float maxMag = 0.0f;
                    for (int k = bin0; k <= bin1; ++k)
                        maxMag = juce::jmax (maxMag, fftBuffer[(size_t) k]);

                    const float normalized = maxMag / ((float) fftSize * 0.5f);
                    const float db = juce::Decibels::gainToDecibels (normalized, -90.0f);

                    spectrumMagnitudes[(size_t) b].store (db);
                }

                fifoIdx = 0;
            }
        }
        fftFifoIndex.store (fifoIdx);
    }

    {
        const float peakL = buffer.getNumChannels() > 0
            ? buffer.getMagnitude (0, 0, buffer.getNumSamples()) : 0.0f;
        const float peakR = buffer.getNumChannels() > 1
            ? buffer.getMagnitude (1, 0, buffer.getNumSamples()) : peakL;

        peakLevelL.store (peakL);
        peakLevelR.store (peakR);
    }
}

juce::AudioProcessorEditor* PPGWave3Processor::createEditor()
{
    return new PPGWave3Editor (*this);
}

void PPGWave3Processor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();

    if (auto old = state.getChildWithName ("SEQUENCER"); old.isValid())
        state.removeChild (old, nullptr);
    state.addChild (sequencer.toValueTree(), -1, nullptr);

    if (auto old = state.getChildWithName ("ARPEGGIATOR"); old.isValid())
        state.removeChild (old, nullptr);
    state.addChild (arpeggiator.toValueTree(), -1, nullptr);

    // FASE 13d: guardar el clock
    juce::ValueTree clock ("CLOCK");
    clock.setProperty ("mode",    clockMode.load(), nullptr);
    clock.setProperty ("freeBpm", freeBpm.load(),   nullptr);
    if (auto old = state.getChildWithName ("CLOCK"); old.isValid())
        state.removeChild (old, nullptr);
    state.addChild (clock, -1, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void PPGWave3Processor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            apvts.replaceState (tree);
            sequencer.fromValueTree (tree.getChildWithName ("SEQUENCER"));
            arpeggiator.fromValueTree (tree.getChildWithName ("ARPEGGIATOR"));

            auto clockTree = tree.getChildWithName ("CLOCK");
            if (clockTree.isValid())
            {
                clockMode.store ((int)   clockTree.getProperty ("mode",    0));
                freeBpm.store   ((float) clockTree.getProperty ("freeBpm", 120.0f));
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PPGWave3Processor();
}
