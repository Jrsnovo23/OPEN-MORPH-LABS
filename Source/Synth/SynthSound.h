#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace synth
{
    class SynthSound : public juce::SynthesiserSound
    {
    public:
        bool appliesToNote    (int) override { return true; }
        bool appliesToChannel (int) override { return true; }
    };
}
