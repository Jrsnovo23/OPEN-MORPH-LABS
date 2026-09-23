#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <atomic>
#include "DSP/Effects.h"

class PPGWave3Processor : public juce::AudioProcessor
{
public:
    PPGWave3Processor();
    ~PPGWave3Processor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi()  const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    // BPM compartido con las voces.
    std::atomic<double> currentBpm { 120.0 };

    // Nivel de pico de la última pasada (para el VU master).
    std::atomic<float> peakLevel { 0.0f };

    // FASE 7.1a: gain reduction del compresor (normalizado 0..1).
    std::atomic<float> compressorGR { 0.0f };

    // Teclado virtual + Pitch/Mod wheels.
    juce::MidiKeyboardState keyboardState;
    std::atomic<float> pitchBendAtomic { 0.0f };
    std::atomic<float> modWheelAtomic  { 0.0f };

private:
    juce::Synthesiser synth;
    dsp::Effects effects;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PPGWave3Processor)
};
