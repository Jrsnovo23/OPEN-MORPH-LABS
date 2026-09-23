#include "Wavetable.h"
#include <cmath>

namespace dsp
{
    void Wavetable::addFrame (const std::vector<float>& frame)
    {
        jassert ((int) frame.size() == frameSize);
        frames.push_back (frame);
    }

    float Wavetable::getSample (float phase, float position) const noexcept
    {
        if (frames.empty()) return 0.0f;

        const int numFrames = (int) frames.size();

        auto readFrame = [&] (int idx) noexcept
        {
            const auto& f = frames[(size_t) idx];
            const float x  = phase * (float) frameSize;
            const int   i0 = (int) x;
            const int   i1 = (i0 + 1) & (frameSize - 1);
            const float t  = x - (float) i0;
            return f[(size_t) i0] + (f[(size_t) i1] - f[(size_t) i0]) * t;
        };

        if (numFrames == 1)
            return readFrame (0);

        const float fpos = juce::jlimit (0.0f, 1.0f, position) * (float) (numFrames - 1);
        const int   f0   = (int) fpos;
        const int   f1   = juce::jmin (f0 + 1, numFrames - 1);
        const float ft   = fpos - (float) f0;

        const float a = readFrame (f0);
        const float b = readFrame (f1);
        return a + (b - a) * ft;
    }

    namespace wavetables
    {
        static std::vector<float> frameFromFunc (const std::function<float(float)>& fn)
        {
            std::vector<float> f ((size_t) Wavetable::frameSize);
            for (int i = 0; i < Wavetable::frameSize; ++i)
            {
                const float p = (float) i / (float) Wavetable::frameSize;
                f[(size_t) i] = fn (p);
            }
            return f;
        }

        static Wavetable makeSine()
        {
            Wavetable t;
            t.addFrame (frameFromFunc ([] (float p) {
                return std::sin (p * juce::MathConstants<float>::twoPi);
            }));
            return t;
        }

        static Wavetable makeTriangle()
        {
            Wavetable t;
            t.addFrame (frameFromFunc ([] (float p) {
                const float x = p - std::floor (p + 0.5f);
                return 4.0f * std::abs (x) - 1.0f;
            }));
            return t;
        }

        static Wavetable makeSaw()
        {
            Wavetable t;
            // 6 frames: barrido de 64 a 2 armónicos (reducción progresiva).
            for (int harmonics : { 64, 32, 16, 8, 4, 2 })
            {
                t.addFrame (frameFromFunc ([harmonics] (float p) {
                    float s = 0.0f;
                    for (int h = 1; h <= harmonics; ++h)
                        s += std::sin (p * juce::MathConstants<float>::twoPi * (float) h) / (float) h;
                    return s * (2.0f / juce::MathConstants<float>::pi);
                }));
            }
            return t;
        }

        static Wavetable makeSquare()
        {
            Wavetable t;
            for (int harmonics : { 63, 31, 15, 7, 3, 1 })
            {
                t.addFrame (frameFromFunc ([harmonics] (float p) {
                    float s = 0.0f;
                    for (int h = 1; h <= harmonics; h += 2)
                        s += std::sin (p * juce::MathConstants<float>::twoPi * (float) h) / (float) h;
                    return s * (4.0f / juce::MathConstants<float>::pi);
                }));
            }
            return t;
        }

        Wavetable makeByIndex (int index)
        {
            switch (index)
            {
                case 0:  return makeSine();
                case 1:  return makeTriangle();
                case 2:  return makeSaw();
                case 3:  return makeSquare();
                default: return makeSine();
            }
        }

        juce::StringArray getNames()
        {
            return { "Sine", "Triangle", "Saw", "Square" };
        }
    }
}