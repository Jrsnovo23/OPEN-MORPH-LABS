#include "Visualizers.h"

namespace ui
{
    // ============================================================
    // WavetablePreview
    // ============================================================
    WavetablePreview::WavetablePreview (juce::AudioProcessorValueTreeState& apvts,
                                        const juce::String& waveParamID,
                                        const juce::String& posParamID)
        : apvtsRef (apvts), waveId (waveParamID), posId (posParamID)
    {
        startTimerHz (30);
    }

    WavetablePreview::~WavetablePreview()
    {
        stopTimer();
    }

    void WavetablePreview::timerCallback()
    {
        refreshIfNeeded();
    }

    void WavetablePreview::refreshIfNeeded()
    {
        int   w = 0;
        float p = 0.0f;

        if (auto* param = apvtsRef.getRawParameterValue (waveId))
            w = (int) param->load();
        if (auto* param = apvtsRef.getRawParameterValue (posId))
            p = param->load();

        bool changed = false;

        if (w != cachedWave)
        {
            cachedTable = dsp::wavetables::makeByIndex (w);
            cachedWave  = w;
            changed = true;
        }

        // Comparación con tolerancia para evitar repaint innecesario
        if (std::abs (p - cachedPos) > 0.001f)
        {
            cachedPos = p;
            changed = true;
        }

        if (changed)
            repaint();
    }

    void WavetablePreview::paint (juce::Graphics& g)
    {
        // Asegurar que el primer render tiene valores
        if (cachedWave < 0)
            refreshIfNeeded();

        auto r = getLocalBounds().toFloat();
        const float w = r.getWidth();
        const float h = r.getHeight();

        g.setColour (juce::Colour (0xff0a0a0a));
        g.fillRoundedRectangle (r, 2.0f);
        g.setColour (juce::Colour (0xff2f2f2f));
        g.drawRoundedRectangle (r.reduced (0.5f), 2.0f, 1.0f);

        g.setColour (juce::Colour (0xff1a1a1a));
        g.drawHorizontalLine ((int) (h * 0.5f), r.getX(), r.getRight());

        if (cachedTable.getNumFrames() == 0) return;

        constexpr int numPoints = 128;
        juce::Path path;
        for (int i = 0; i < numPoints; ++i)
        {
            const float phase  = (float) i / (float) (numPoints - 1);
            const float sample = cachedTable.getSample (phase, cachedPos);
            const float x = r.getX() + phase * w;
            const float y = r.getCentreY() - sample * (h * 0.42f);
            if (i == 0) path.startNewSubPath (x, y);
            else        path.lineTo (x, y);
        }

        g.setColour (juce::Colour (0xffffaa00).withAlpha (0.25f));
        g.strokePath (path, juce::PathStrokeType (3.0f));

        g.setColour (juce::Colour (0xffffcc55));
        g.strokePath (path, juce::PathStrokeType (1.4f));
    }

    // ============================================================
    // EnvelopeDisplay
    // ============================================================
    EnvelopeDisplay::EnvelopeDisplay (juce::AudioProcessorValueTreeState& apvts,
                                      const juce::String& aId, const juce::String& dId,
                                      const juce::String& sId, const juce::String& rId)
        : apvtsRef (apvts),
          attackId (aId), decayId (dId), sustainId (sId), releaseId (rId)
    {
        startTimerHz (30);
    }

    EnvelopeDisplay::~EnvelopeDisplay()
    {
        stopTimer();
    }

    void EnvelopeDisplay::timerCallback()
    {
        float a = 0, d = 0, s = 0, r = 0;
        if (auto* p = apvtsRef.getRawParameterValue (attackId))  a = p->load();
        if (auto* p = apvtsRef.getRawParameterValue (decayId))   d = p->load();
        if (auto* p = apvtsRef.getRawParameterValue (sustainId)) s = p->load();
        if (auto* p = apvtsRef.getRawParameterValue (releaseId)) r = p->load();

        if (std::abs (a - cachedA) > 0.0001f ||
            std::abs (d - cachedD) > 0.0001f ||
            std::abs (s - cachedS) > 0.0001f ||
            std::abs (r - cachedR) > 0.0001f)
        {
            cachedA = a; cachedD = d; cachedS = s; cachedR = r;
            repaint();
        }
    }

    void EnvelopeDisplay::paint (juce::Graphics& g)
    {
        auto r = getLocalBounds().toFloat();
        const float w = r.getWidth();
        const float h = r.getHeight();

        g.setColour (juce::Colour (0xff0a0a0a));
        g.fillRoundedRectangle (r, 2.0f);
        g.setColour (juce::Colour (0xff2f2f2f));
        g.drawRoundedRectangle (r.reduced (0.5f), 2.0f, 1.0f);

        float A = 0.1f, D = 0.1f, S = 0.7f, R = 0.3f;
        if (auto* p = apvtsRef.getRawParameterValue (attackId))  A = p->load();
        if (auto* p = apvtsRef.getRawParameterValue (decayId))   D = p->load();
        if (auto* p = apvtsRef.getRawParameterValue (sustainId)) S = p->load();
        if (auto* p = apvtsRef.getRawParameterValue (releaseId)) R = p->load();

        const float sustainDisplay = 0.5f;
        const float total = juce::jmax (0.01f, A + D + sustainDisplay + R);

        const float pad = 4.0f;
        const float innerW = w - pad * 2.0f;
        const float innerH = h - pad * 2.0f;
        const float x0 = r.getX() + pad;
        const float y0 = r.getBottom() - pad;
        const float y1 = r.getY() + pad;

        const float xA = x0 + (A / total) * innerW;
        const float xD = xA + (D / total) * innerW;
        const float xS = xD + (sustainDisplay / total) * innerW;
        const float xR = xS + (R / total) * innerW;
        const float yS = y0 - S * innerH;

        juce::Path p;
        p.startNewSubPath (x0, y0);
        p.lineTo (xA, y1);
        p.lineTo (xD, yS);
        p.lineTo (xS, yS);
        p.lineTo (xR, y0);

        juce::Path filled = p;
        filled.lineTo (x0, y0);
        filled.closeSubPath();
        g.setColour (juce::Colour (0xffffaa00).withAlpha (0.12f));
        g.fillPath (filled);

        g.setColour (juce::Colour (0xffffcc55));
        g.strokePath (p, juce::PathStrokeType (1.6f,
                     juce::PathStrokeType::curved,
                     juce::PathStrokeType::rounded));

        g.setColour (juce::Colour (0xffffaa00));
        const juce::Point<float> pts[] = {
            { xA, y1 }, { xD, yS }, { xS, yS }, { xR, y0 }
        };
        for (const auto& pt : pts)
            g.fillEllipse (pt.x - 2.0f, pt.y - 2.0f, 4.0f, 4.0f);
    }

    // ============================================================
    // LFODisplay
    // ============================================================
    LFODisplay::LFODisplay (juce::AudioProcessorValueTreeState& apvts,
                            const juce::String& waveParamID)
        : apvtsRef (apvts), waveId (waveParamID)
    {
        startTimerHz (30);
    }

    LFODisplay::~LFODisplay()
    {
        stopTimer();
    }

    void LFODisplay::timerCallback()
    {
        int w = 0;
        if (auto* p = apvtsRef.getRawParameterValue (waveId))
            w = (int) p->load();

        if (w != cachedWave)
        {
            cachedWave = w;
            repaint();
        }
    }

    void LFODisplay::paint (juce::Graphics& g)
    {
        auto r = getLocalBounds().toFloat();
        const float w = r.getWidth();
        const float h = r.getHeight();

        g.setColour (juce::Colour (0xff0a0a0a));
        g.fillRoundedRectangle (r, 2.0f);
        g.setColour (juce::Colour (0xff2f2f2f));
        g.drawRoundedRectangle (r.reduced (0.5f), 2.0f, 1.0f);

        g.setColour (juce::Colour (0xff1a1a1a));
        g.drawHorizontalLine ((int) r.getCentreY(), r.getX(), r.getRight());

        int waveIdx = cachedWave;
        if (waveIdx < 0)
        {
            if (auto* p = apvtsRef.getRawParameterValue (waveId))
                waveIdx = (int) p->load();
            else
                waveIdx = 0;
        }

        constexpr int N = 96;
        juce::Path path;
        for (int i = 0; i < N; ++i)
        {
            const float phase = (float) i / (float) (N - 1);
            float sample = 0.0f;

            switch (waveIdx)
            {
                case 0:  sample = std::sin (phase * juce::MathConstants<float>::twoPi); break;
                case 1:  sample = 4.0f * std::abs (phase - 0.5f) - 1.0f;                break;
                case 2:  sample = (phase < 0.5f) ? 1.0f : -1.0f;                        break;
                case 3:  sample = 2.0f * phase - 1.0f;                                  break;
                case 4:  sample = 1.0f - 2.0f * phase;                                  break;
                case 5:  sample = std::sin (phase * juce::MathConstants<float>::twoPi * 3.0f)
                                * std::sin (phase * juce::MathConstants<float>::pi);    break;
                case 6:  sample = (phase < 0.33f) ? 0.7f : (phase < 0.66f ? -0.4f : 0.2f); break;
                default: sample = 0.0f; break;
            }

            const float x = r.getX() + phase * w;
            const float y = r.getCentreY() - sample * (h * 0.38f);
            if (i == 0) path.startNewSubPath (x, y);
            else        path.lineTo (x, y);
        }

        g.setColour (juce::Colour (0xffffaa00).withAlpha (0.25f));
        g.strokePath (path, juce::PathStrokeType (3.0f));
        g.setColour (juce::Colour (0xffffcc55));
        g.strokePath (path, juce::PathStrokeType (1.4f));
    }

    // ============================================================
    // LevelMeter
    // ============================================================
    LevelMeter::LevelMeter (std::atomic<float>& levelSource) : level (levelSource)
    {
        startTimerHz (30);
    }

    void LevelMeter::timerCallback()
    {
        const float current = level.load();
        if (current > smoothed) smoothed = current;
        else                    smoothed = smoothed * 0.88f + current * 0.12f;
        repaint();
    }

    void LevelMeter::paint (juce::Graphics& g)
    {
        auto r = getLocalBounds().toFloat();

        g.setColour (juce::Colour (0xff0a0a0a));
        g.fillRoundedRectangle (r, 2.0f);
        g.setColour (juce::Colour (0xff2f2f2f));
        g.drawRoundedRectangle (r.reduced (0.5f), 2.0f, 1.0f);

        const float v = juce::jlimit (0.0f, 1.0f, smoothed);
        if (v > 0.001f)
        {
            auto bar = r.reduced (2.0f);
            const float filled = bar.getWidth() * v;

            juce::Colour c;
            if (v < 0.7f)      c = juce::Colour (0xff2ecc40);
            else if (v < 0.9f) c = juce::Colour (0xffffaa00);
            else               c = juce::Colour (0xffff3b30);

            g.setColour (c);
            g.fillRoundedRectangle (bar.getX(), bar.getY(),
                                    filled, bar.getHeight(), 2.0f);
        }

        g.setColour (juce::Colour (0xff2f2f2f));
        const float marks[] = { 0.25f, 0.5f, 0.75f };
        for (float p : marks)
            g.drawVerticalLine ((int) (r.getX() + r.getWidth() * p),
                                r.getY() + 2.0f, r.getBottom() - 2.0f);
    }
}
