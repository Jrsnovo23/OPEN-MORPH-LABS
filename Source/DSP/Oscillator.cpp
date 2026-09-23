#include "Oscillator.h"
#include <cmath>

namespace dsp
{
    float Oscillator::getNextSample (const Wavetable& table,
                                     float frequencyHz,
                                     float position) noexcept
    {
        const float s   = table.getSample (phase, position);
        const float inc = (float) ((double) frequencyHz / sr);
        phase += inc;
        phase -= std::floor (phase);   // wrap a [0,1)
        return s;
    }
}
