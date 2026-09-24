#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "../DSP/Wavetable.h"

namespace ui
{
    class WavetablePreview : public juce::Component,
                             private juce::Timer
    {
    public:
        WavetablePreview (juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& waveParamID,
                          const juce::String& posParamID);
        ~WavetablePreview() override;
        void paint (juce::Graphics&) override;
    private:
        void timerCallback() override;
        void refreshIfNeeded();

        juce::AudioProcessorValueTreeState& apvtsRef;
        juce::String waveId, posId;
        int   cachedWave = -1;
        float cachedPos  = -1.0f;
        dsp::Wavetable cachedTable;
        float animPhase  = 0.0f;   // NUEVO: fase decorativa 0..1
    };

    class EnvelopeDisplay : public juce::Component,
                            private juce::Timer
    {
    public:
        EnvelopeDisplay (juce::AudioProcessorValueTreeState& apvts,
                         const juce::String& aId, const juce::String& dId,
                         const juce::String& sId, const juce::String& rId);
        ~EnvelopeDisplay() override;
        void paint (juce::Graphics&) override;
    private:
        void timerCallback() override;

        juce::AudioProcessorValueTreeState& apvtsRef;
        juce::String attackId, decayId, sustainId, releaseId;

        float cachedA = -1, cachedD = -1, cachedS = -1, cachedR = -1;
    };

    class LFODisplay : public juce::Component,
                       private juce::Timer
    {
    public:
        LFODisplay (juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& waveParamID);
        ~LFODisplay() override;
        void paint (juce::Graphics&) override;
    private:
        void timerCallback() override;

        juce::AudioProcessorValueTreeState& apvtsRef;
        juce::String waveId;
        int cachedWave = -1;
        float animPhase = 0.0f;   // NUEVO: fase decorativa 0..1
    };

    class LevelMeter : public juce::Component, private juce::Timer
    {
    public:
        explicit LevelMeter (std::atomic<float>& levelSource);
        void paint (juce::Graphics&) override;
    private:
        void timerCallback() override;
        std::atomic<float>& level;
        float smoothed = 0.0f;
    };

    class HorizontalMeter : public juce::Component, private juce::Timer
    {
    public:
        HorizontalMeter (std::atomic<float>& levelSource,
                         const juce::String& labelText = {},
                         bool gainReductionMode = false);
        void paint (juce::Graphics&) override;
    private:
        void timerCallback() override;

        std::atomic<float>& level;
        juce::String label;
        bool grMode = false;
        float smoothed = 0.0f;
    };

    class StereoHorizontalMeter : public juce::Component, private juce::Timer
    {
    public:
        StereoHorizontalMeter (std::atomic<float>& levelL,
                               std::atomic<float>& levelR,
                               const juce::String& labelText = {});
        void paint (juce::Graphics&) override;
    private:
        void timerCallback() override;

        std::atomic<float>& levelL;
        std::atomic<float>& levelR;
        juce::String label;
        float smoothedL = 0.0f;
        float smoothedR = 0.0f;
    };
}
