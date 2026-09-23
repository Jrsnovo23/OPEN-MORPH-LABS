#pragma once
#include "Wavetable.h"

namespace dsp
{
    class Oscillator
    {
    public:
        void prepare (double sampleRate) noexcept { sr = sampleRate; }
        void reset   (float initialPhase = 0.0f) noexcept { phase = initialPhase; }

        float getNextSample (const Wavetable& table,
                             float frequencyHz,
                             float position) noexcept;

    private:
        double sr    = 44100.0;
        float  phase = 0.0f;
    };
}