#include "Filter.h"

namespace dsp
{
    void Filter::prepare (double sampleRate)
    {
        sr = sampleRate;
        juce::dsp::ProcessSpec spec { sampleRate, 512, 1 };
        svf.prepare (spec);
        svf.reset();
    }

    void Filter::reset()
    {
        svf.reset();
    }

    void Filter::setType (int type)
    {
        switch (type)
        {
            case 1:  svf.setType (juce::dsp::StateVariableTPTFilterType::highpass); break;
            case 2:  svf.setType (juce::dsp::StateVariableTPTFilterType::bandpass); break;
            case 0:
            default: svf.setType (juce::dsp::StateVariableTPTFilterType::lowpass);  break;
        }
    }

    void Filter::setCutoff (float hz)
    {
        const float maxHz = (float) (sr * 0.45);
        svf.setCutoffFrequency (juce::jlimit (20.0f, maxHz, hz));
    }

    void Filter::setResonance (float r)
    {
        // r en [0,1] -> resonancia del SVF en [0.7, 8.0]
        svf.setResonance (juce::jlimit (0.7f, 8.0f, 0.7f + r * 7.3f));
    }

    float Filter::processSample (float input) noexcept
    {
        return svf.processSample (0, input);
    }
}
