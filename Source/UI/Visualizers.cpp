#include "Visualizers.h"

namespace ui
{
    // ============ WavetablePreview ============
    WavetablePreview::WavetablePreview (juce::AudioProcessorValueTreeState& apvts,
                                        const juce::String& waveParamID,
                                        const juce::String& posParamID)
        : apvtsRef (apvts), waveId (waveParamID), posId (posParamID)
    {
        startTimerHz (30);
    }

    WavetablePreview::~WavetablePreview() { stopTimer(); }

    void WavetablePreview::timerCallback()
    {
        // FASE 12: si hay fase real disponible, la usamos.
        // Si no, avanzamos decorativamente como en 7.3a.
        if (phasePtr != nullptr)
        {
            float p = phasePtr->load();
            p -= std::floor (p);
            animPhase = p;
        }
        else
        {
            animPhase += 0.019f;
            if (animPhase >= 1.0f) animPhase -= 1.0f;
        }

        refreshIfNeeded();
        repaint();
    }

    void WavetablePreview::refreshIfNeeded()
    {
        int   w = 0;
        float p = 0.0f;

        if (auto* param = apvtsRef.getRawParameterValue (waveId))
            w = (int) param->load();
        if (auto* param = apvtsRef.getRawParameterValue (posId))
            p = param->load();

        if (w != cachedWave)
        {
            cachedTable = dsp::wavetables::makeByIndex (w);
            cachedWave  = w;
        }
        if (std::abs (p - cachedPos) > 0.001f) { cachedPos = p; }
    }

    void WavetablePreview::paint (juce::Graphics& g)
    {
        if (cachedWave < 0) refreshIfNeeded();

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

        // Punto que sigue la fase (real o decorativa)
        {
            const float sample = cachedTable.getSample (animPhase, cachedPos);
            const float px = r.getX() + animPhase * w;
            const float py = r.getCentreY() - sample * (h * 0.42f);

            g.setColour (juce::Colour (0xffffaa00).withAlpha (0.30f));
            g.fillEllipse (px - 5.0f, py - 5.0f, 10.0f, 10.0f);

            g.setColour (juce::Colour (0xffffe08a));
            g.fillEllipse (px - 2.2f, py - 2.2f, 4.4f, 4.4f);

            g.setColour (juce::Colour (0xffb07000));
            g.drawEllipse (px - 2.2f, py - 2.2f, 4.4f, 4.4f, 0.7f);
        }
    }

    // ============ EnvelopeDisplay ============
    EnvelopeDisplay::EnvelopeDisplay (juce::AudioProcessorValueTreeState& apvts,
                                      const juce::String& aId, const juce::String& dId,
                                      const juce::String& sId, const juce::String& rId)
        : apvtsRef (apvts),
          attackId (aId), decayId (dId), sustainId (sId), releaseId (rId)
    { startTimerHz (30); }

    EnvelopeDisplay::~EnvelopeDisplay() { stopTimer(); }

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

    // ============ LFODisplay ============
    LFODisplay::LFODisplay (juce::AudioProcessorValueTreeState& apvts,
                            const juce::String& waveParamID)
        : apvtsRef (apvts), waveId (waveParamID)
    { startTimerHz (30); }

    LFODisplay::~LFODisplay() { stopTimer(); }

    void LFODisplay::timerCallback()
    {
        // FASE 12: fase real si está disponible; si no, decorativa.
        if (phasePtr != nullptr)
        {
            float p = phasePtr->load();
            p -= std::floor (p);
            animPhase = p;
        }
        else
        {
            animPhase += 0.019f;
            if (animPhase >= 1.0f) animPhase -= 1.0f;
        }

        int w = 0;
        if (auto* p = apvtsRef.getRawParameterValue (waveId))
            w = (int) p->load();

        cachedWave = w;
        repaint();
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

        auto sampleAt = [waveIdx] (float phase) -> float
        {
            switch (waveIdx)
            {
                case 0:  return std::sin (phase * juce::MathConstants<float>::twoPi);
                case 1:  return 4.0f * std::abs (phase - 0.5f) - 1.0f;
                case 2:  return (phase < 0.5f) ? 1.0f : -1.0f;
                case 3:  return 2.0f * phase - 1.0f;
                case 4:  return 1.0f - 2.0f * phase;
                case 5:  return std::sin (phase * juce::MathConstants<float>::twoPi * 3.0f)
                              * std::sin (phase * juce::MathConstants<float>::pi);
                case 6:  return (phase < 0.33f) ? 0.7f : (phase < 0.66f ? -0.4f : 0.2f);
                default: return 0.0f;
            }
        };

        constexpr int N = 96;
        juce::Path path;
        for (int i = 0; i < N; ++i)
        {
            const float phase = (float) i / (float) (N - 1);
            const float sample = sampleAt (phase);
            const float x = r.getX() + phase * w;
            const float y = r.getCentreY() - sample * (h * 0.38f);
            if (i == 0) path.startNewSubPath (x, y);
            else        path.lineTo (x, y);
        }

        g.setColour (juce::Colour (0xffffaa00).withAlpha (0.25f));
        g.strokePath (path, juce::PathStrokeType (3.0f));
        g.setColour (juce::Colour (0xffffcc55));
        g.strokePath (path, juce::PathStrokeType (1.4f));

        // Punto que sigue la fase (real o decorativa)
        {
            const float sample = sampleAt (animPhase);
            const float px = r.getX() + animPhase * w;
            const float py = r.getCentreY() - sample * (h * 0.38f);

            g.setColour (juce::Colour (0xffffaa00).withAlpha (0.30f));
            g.fillEllipse (px - 4.0f, py - 4.0f, 8.0f, 8.0f);

            g.setColour (juce::Colour (0xffffe08a));
            g.fillEllipse (px - 1.8f, py - 1.8f, 3.6f, 3.6f);

            g.setColour (juce::Colour (0xffb07000));
            g.drawEllipse (px - 1.8f, py - 1.8f, 3.6f, 3.6f, 0.6f);
        }
    }

    // ============ LevelMeter (vertical) ============
    LevelMeter::LevelMeter (std::atomic<float>& levelSource) : level (levelSource)
    { startTimerHz (30); }

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

    // ============ HorizontalMeter (mono) ============
    HorizontalMeter::HorizontalMeter (std::atomic<float>& levelSource,
                                      const juce::String& labelText,
                                      bool gainReductionMode)
        : level (levelSource), label (labelText), grMode (gainReductionMode)
    { startTimerHz (30); }

    void HorizontalMeter::timerCallback()
    {
        const float current = level.load();
        if (current > smoothed) smoothed = current;
        else                    smoothed = smoothed * 0.85f + current * 0.15f;
        repaint();
    }

    void HorizontalMeter::paint (juce::Graphics& g)
    {
        auto r = getLocalBounds().toFloat();

        g.setColour (juce::Colour (0xff0a0a0a));
        g.fillRoundedRectangle (r, 2.0f);
        g.setColour (juce::Colour (0xff2f2f2f));
        g.drawRoundedRectangle (r.reduced (0.5f), 2.0f, 1.0f);

        auto bar = r.reduced (2.0f);

        if (label.isNotEmpty())
        {
            const int labelW = 24;
            auto labelArea = bar.removeFromLeft ((float) labelW);

            g.setColour (juce::Colour (0xff888888));
            g.setFont (juce::FontOptions (juce::jmax (7.0f, bar.getHeight() * 0.7f),
                                          juce::Font::bold));
            g.drawText (label, labelArea, juce::Justification::centred);

            bar.removeFromLeft (2.0f);
        }

        const float v = juce::jlimit (0.0f, 1.0f, smoothed);

        if (v > 0.001f)
        {
            const float filled = bar.getWidth() * v;

            juce::Colour c;
            if (grMode)
            {
                if (v < 0.5f)      c = juce::Colour (0xff2ecc40);
                else if (v < 0.8f) c = juce::Colour (0xffffaa00);
                else               c = juce::Colour (0xffff3b30);
            }
            else
            {
                if (v < 0.7f)      c = juce::Colour (0xff2ecc40);
                else if (v < 0.9f) c = juce::Colour (0xffffaa00);
                else               c = juce::Colour (0xffff3b30);
            }

            g.setColour (c);
            g.fillRoundedRectangle (bar.getX(), bar.getY(),
                                    filled, bar.getHeight(), 2.0f);
        }

        g.setColour (juce::Colour (0xff2f2f2f));
        const float marks[] = { 0.25f, 0.5f, 0.75f };
        for (float p : marks)
            g.drawVerticalLine ((int) (bar.getX() + bar.getWidth() * p),
                                bar.getY() + 1.0f, bar.getBottom() - 1.0f);
    }

    // ============ StereoHorizontalMeter ============
    StereoHorizontalMeter::StereoHorizontalMeter (std::atomic<float>& levelL_,
                                                  std::atomic<float>& levelR_,
                                                  const juce::String& labelText)
        : levelL (levelL_), levelR (levelR_), label (labelText)
    { startTimerHz (30); }

    void StereoHorizontalMeter::timerCallback()
    {
        const float curL = levelL.load();
        const float curR = levelR.load();

        if (curL > smoothedL) smoothedL = curL;
        else                  smoothedL = smoothedL * 0.85f + curL * 0.15f;

        if (curR > smoothedR) smoothedR = curR;
        else                  smoothedR = smoothedR * 0.85f + curR * 0.15f;

        repaint();
    }

    void StereoHorizontalMeter::paint (juce::Graphics& g)
    {
        auto r = getLocalBounds().toFloat();

        g.setColour (juce::Colour (0xff0a0a0a));
        g.fillRoundedRectangle (r, 2.0f);
        g.setColour (juce::Colour (0xff2f2f2f));
        g.drawRoundedRectangle (r.reduced (0.5f), 2.0f, 1.0f);

        auto bar = r.reduced (2.0f);

        if (label.isNotEmpty())
        {
            const int labelW = 22;
            auto labelArea = bar.removeFromLeft ((float) labelW);

            g.setColour (juce::Colour (0xff888888));
            g.setFont (juce::FontOptions (juce::jmax (7.0f, bar.getHeight() * 0.40f),
                                          juce::Font::bold));
            g.drawText (label, labelArea, juce::Justification::centred);

            bar.removeFromLeft (3.0f);
        }

        const auto marksArea = bar;

        const float gap  = 1.5f;
        const float barH = (bar.getHeight() - gap) * 0.5f;

        auto barL = bar.removeFromTop (barH);
        bar.removeFromTop (gap);
        auto barR = bar;

        auto drawBar = [&] (juce::Rectangle<float> area, float value)
        {
            const float v = juce::jlimit (0.0f, 1.0f, value);
            if (v > 0.001f)
            {
                juce::Colour c;
                if (v < 0.7f)      c = juce::Colour (0xff2ecc40);
                else if (v < 0.9f) c = juce::Colour (0xffffaa00);
                else               c = juce::Colour (0xffff3b30);

                g.setColour (c);
                g.fillRoundedRectangle (area.getX(), area.getY(),
                                        area.getWidth() * v, area.getHeight(), 1.0f);
            }
        };

        drawBar (barL, smoothedL);
        drawBar (barR, smoothedR);

        g.setColour (juce::Colour (0x88ffffff));
        const float marks[] = { 0.25f, 0.5f, 0.75f };
        for (float p : marks)
            g.drawVerticalLine ((int) (marksArea.getX() + marksArea.getWidth() * p),
                                marksArea.getY(), marksArea.getBottom());
    }
}
