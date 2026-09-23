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
        synth.addVoice (voice);
    }

    synth.addSound (new synth::SynthSound());
}

void PPGWave3Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    effects.prepare (sampleRate, samplesPerBlock, 2);
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

    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto bpm = pos->getBpm())
                currentBpm.store (*bpm);
        }
    }

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
            keyboardState.noteOn (msg.getChannel(), msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff())
            keyboardState.noteOff (msg.getChannel(), msg.getNoteNumber(), msg.getFloatVelocity());
    }

    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);

    {
        const int pbValue = juce::jlimit (0, 16383,
            (int) std::lround (8192.0f + pitchBendAtomic.load() * 8192.0f));
        midi.addEvent (juce::MidiMessage::pitchWheel (1, pbValue), 0);

        const int mwValue = juce::jlimit (0, 127,
            (int) std::lround (modWheelAtomic.load() * 127.0f));
        midi.addEvent (juce::MidiMessage::controllerEvent (1, 1, mwValue), 0);
    }

    synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());

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

    {
        float peak = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, buffer.getNumSamples()));
        peakLevel.store (peak);
    }
}

juce::AudioProcessorEditor* PPGWave3Processor::createEditor()
{
    return new PPGWave3Editor (*this);
}

void PPGWave3Processor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void PPGWave3Processor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PPGWave3Processor();
}
