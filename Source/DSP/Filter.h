#pragma once
#include <juce_dsp/juce_dsp.h>

namespace dsp
{
    // Filtro multimodo basado en State Variable Filter de JUCE (TPT).
    // Un filtro independiente por voz, con control de cutoff y resonancia.
    class Filter
    {
    public:
        void prepare (double sampleRate);
        void reset();
        void setType (int type);   // 0=LP, 1=HP, 2=BP
        void setCutoff (float hz);
        void setResonance (float r);
        float processSample (float input) noexcept;

    private:
        juce::dsp::StateVariableTPTFilter<float> svf;
        double sr = 44100.0;
    };
}
