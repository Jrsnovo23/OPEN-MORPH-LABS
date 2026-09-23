#pragma once
#include <juce_core/juce_core.h>
#include <cmath>

namespace dsp
{
    // LFO con 7 formas: Sine, Triangle, Square, Saw Up, Saw Down, Random, Sample & Hold.
    // Random = interpolación suave entre valores aleatorios (smooth random).
    // Sample & Hold = valor aleatorio que se mantiene hasta el siguiente wrap.
    class LFO
    {
    public:
        enum class Wave
        {
            Sine = 0,
            Triangle,
            Square,
            SawUp,
            SawDown,
            Random,
            SampleHold
        };

        void prepare (double sampleRate) noexcept { sr = sampleRate; }
        void reset   (float initialPhase = 0.0f) noexcept
        {
            phase    = initialPhase;
            rngState = 0x12345678u;
            randomA  = 0.0f;
            randomB  = 0.0f;
            lastSH   = 0.0f;
        }

        // rateHz: frecuencia en Hz. phaseOffset en [0,1).
        float getNextSample (Wave wave, float rateHz, float phaseOffset = 0.0f) noexcept;

    private:
        double sr    = 44100.0;
        float  phase = 0.0f;

        juce::uint32 rngState = 0x12345678u;
        float randomA = 0.0f;  // valor actual (para interpolación de Random)
        float randomB = 0.0f;  // valor siguiente
        float lastSH  = 0.0f;  // valor actual de Sample & Hold

        float nextRandom() noexcept;
    };
}
