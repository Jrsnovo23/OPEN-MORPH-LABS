#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <array>
#include <vector>
#include "DSP/Effects.h"
#include "DSP/StepSequencer.h"
#include "DSP/Arpeggiator.h"

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

    std::atomic<double> currentBpm { 120.0 };

    std::atomic<float> peakLevelL { 0.0f };
    std::atomic<float> peakLevelR { 0.0f };
    std::atomic<float> compressorGR { 0.0f };

    juce::MidiKeyboardState keyboardState;
    std::atomic<float> pitchBendAtomic { 0.0f };
    std::atomic<float> modWheelAtomic  { 0.0f };

    // Fase 12: fases actuales para visualizadores
    std::atomic<float> osc1Phase { 0.0f };
    std::atomic<float> osc2Phase { 0.0f };
    std::atomic<float> lfo1Phase { 0.0f };
    std::atomic<float> lfo2Phase { 0.0f };

    // Fase 10: secuenciador
    StepSequencer sequencer;

    // Fase 13: arpegiador
    Arpeggiator arpeggiator;

    // Fase 12: analizador de espectro
    static constexpr int fftOrder        = 11;
    static constexpr int fftSize         = 1 << fftOrder;
    static constexpr int fftBins         = fftSize / 2;
    static constexpr int numSpectrumBins = 128;

    float getSpectrumMagnitude (int bin) const noexcept
    {
        if (bin < 0 || bin >= numSpectrumBins) return -90.0f;
        return spectrumMagnitudes[(size_t) bin].load();
    }

private:
    juce::Synthesiser synth;
    dsp::Effects effects;

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window {
        (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann
    };
    std::array<float, fftSize>     fftFifo   {};
    std::array<float, fftSize * 2> fftBuffer {};
    std::atomic<int> fftFifoIndex { 0 };
    std::array<std::atomic<float>, numSpectrumBins> spectrumMagnitudes {};

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PPGWave3Processor)
};
