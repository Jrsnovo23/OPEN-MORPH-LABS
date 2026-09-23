#pragma once
#include <juce_core/juce_core.h>
#include <vector>
#include <functional>

namespace dsp
{
    // Cada frame es una onda de 2048 samples. La "posición" (0..1) barre
    // entre frames con interpolación lineal, permitiendo timbres en movimiento.
    class Wavetable
    {
    public:
        static constexpr int frameSize = 2048;

        Wavetable() = default;

        void addFrame (const std::vector<float>& frame);
        int  getNumFrames() const noexcept { return (int) frames.size(); }

        // phase y position en [0,1). Devuelve el sample interpolado.
        float getSample (float phase, float position) const noexcept;

    private:
        std::vector<std::vector<float>> frames;
    };

    namespace wavetables
    {
        Wavetable makeByIndex (int index); // 0..3
        juce::StringArray getNames();
    }
}