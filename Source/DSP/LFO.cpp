#include "LFO.h"

namespace dsp
{
    float LFO::nextRandom() noexcept
    {
        // xorshift32 — rápido, sin allocaciones
        rngState ^= rngState << 13;
        rngState ^= rngState >> 17;
        rngState ^= rngState << 5;
        return ((float) (rngState & 0xFFFFu) / 32768.0f) - 1.0f;  // [-1, 1]
    }

    float LFO::getNextSample (Wave wave, float rateHz, float phaseOffset) noexcept
    {
        const float p = std::fmod (phase + phaseOffset, 1.0f);
        float out = 0.0f;

        switch (wave)
        {
            case Wave::Sine:
                out = std::sin (p * juce::MathConstants<float>::twoPi);
                break;

            case Wave::Triangle:
                out = 4.0f * std::abs (p - 0.5f) - 1.0f;
                break;

            case Wave::Square:
                out = (p < 0.5f) ? 1.0f : -1.0f;
                break;

            case Wave::SawUp:
                out = 2.0f * p - 1.0f;      // sube de -1 a +1
                break;

            case Wave::SawDown:
                out = 1.0f - 2.0f * p;      // baja de +1 a -1
                break;

            case Wave::Random:
                // Interpolación lineal entre randomA y randomB
                out = randomA + (randomB - randomA) * p;
                break;

            case Wave::SampleHold:
                out = lastSH;
                break;
        }

        // Avance de fase + detección de wrap
        const float oldPhase = phase;
        phase += (float) ((double) rateHz / sr);
        phase -= std::floor (phase);

        if (phase < oldPhase)  // fase dio la vuelta
        {
            randomA = randomB;         // el valor "siguiente" pasa a ser el actual
            randomB = nextRandom();    // generamos uno nuevo para el siguiente ciclo
            lastSH  = nextRandom();    // y actualizamos S&H
        }

        return out;
    }
}
