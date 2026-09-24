#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "../DSP/Oscillator.h"
#include "../DSP/Wavetable.h"
#include "../DSP/Filter.h"
#include "../DSP/LFO.h"

namespace synth
{
    class SynthVoice : public juce::SynthesiserVoice
    {
    public:
        explicit SynthVoice (juce::AudioProcessorValueTreeState& apvts);

        void setBpmSource (std::atomic<double>* bpm) { bpmSource = bpm; }

        void setPhaseTargets (std::atomic<float>* osc1Ph,
                              std::atomic<float>* osc2Ph,
                              std::atomic<float>* lfo1Ph,
                              std::atomic<float>* lfo2Ph)
        {
            osc1PhaseTarget = osc1Ph;
            osc2PhaseTarget = osc2Ph;
            lfo1PhaseTarget = lfo1Ph;
            lfo2PhaseTarget = lfo2Ph;
        }

        bool canPlaySound (juce::SynthesiserSound*) override;
        void startNote (int midiNote, float velocity,
                        juce::SynthesiserSound*, int currentPitchWheelPos) override;
        void stopNote (float velocity, bool allowTailOff) override;
        void pitchWheelMoved (int newValue) override;
        void controllerMoved (int controllerNumber, int newValue) override;
        void renderNextBlock (juce::AudioBuffer<float>&, int, int) override;
        void setCurrentPlaybackSampleRate (double newRate) override;

    private:
        struct ModSlot
        {
            int   source = 0;
            int   dest   = 0;
            float amount = 0.0f;
        };

        static float syncIndexToBeats (int idx) noexcept;

        juce::AudioProcessorValueTreeState& apvts;
        std::atomic<double>* bpmSource = nullptr;

        dsp::Oscillator osc1, osc2;
        dsp::Wavetable  wave1, wave2;
        int currentWave1Idx = -1;
        int currentWave2Idx = -1;

        dsp::Filter filter;
        dsp::LFO    lfo1, lfo2;

        float currentFreq     = 440.0f;
        float currentMidiNote = 60.0f;
        float velocityGain    = 1.0f;
        float pitchBendValue  = 0.0f;
        float modWheelValue   = 0.0f;
        float aftertouchValue = 0.0f;
        float randomValue     = 0.0f;
        bool  isActive        = false;

        juce::ADSR adsr;
        juce::ADSR filtAdsr;
        juce::ADSR env3;

        juce::ADSR::Parameters adsrParams     { 0.005f, 0.100f, 0.700f, 0.300f };
        juce::ADSR::Parameters filtAdsrParams { 0.005f, 0.200f, 0.500f, 0.300f };
        juce::ADSR::Parameters env3Params     { 0.005f, 0.200f, 0.500f, 0.300f };

        // ===== FASE 8: Vintage Character (per-voice) =====
        juce::Random vintageRandom;
        float driftPhase       = 0.0f;
        float driftSeed        = 0.0f;
        float voiceDetune      = 0.0f;
        float voiceFilterOffset= 0.0f;
        float voiceLevelOffset = 1.0f;

        // ===== FASE 12: trackers de fase para visualizadores =====
        std::atomic<float>* osc1PhaseTarget = nullptr;
        std::atomic<float>* osc2PhaseTarget = nullptr;
        std::atomic<float>* lfo1PhaseTarget = nullptr;
        std::atomic<float>* lfo2PhaseTarget = nullptr;

        float osc1PhaseTrack = 0.0f;
        float osc2PhaseTrack = 0.0f;
        float lfo1PhaseTrack = 0.0f;
        float lfo2PhaseTrack = 0.0f;
    };
}
