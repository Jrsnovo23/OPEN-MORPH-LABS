#include "PPGLookAndFeel.h"
#include <cmath>

// ==================== ValueBoxLabel ====================

ValueBoxLabel::ValueBoxLabel()
{
    setJustificationType (juce::Justification::centred);
    setColour (juce::Label::textColourId, juce::Colour (0xffffcc55));
    setColour (juce::Label::backgroundColourId, juce::Colour (0x00000000));
    setColour (juce::Label::outlineColourId,    juce::Colour (0x00000000));
    setInterceptsMouseClicks (false, false);
}

void ValueBoxLabel::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (juce::Colour (0xee000000));
    g.fillRoundedRectangle (r, 4.0f);

    g.setColour (juce::Colour (0xffffaa00));
    g.drawRoundedRectangle (r, 4.0f, 1.0f);

    g.setColour (findColour (juce::Label::textColourId));
    g.setFont (getFont());
    g.drawText (getText(), getLocalBounds(), juce::Justification::centred, false);
}

// ==================== PPGLookAndFeel ====================

PPGLookAndFeel::PPGLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, bgApp());
    setColour (juce::PopupMenu::backgroundColourId, bgPanel());
    setColour (juce::PopupMenu::textColourId, textPrimary());
    setColour (juce::PopupMenu::highlightedBackgroundColourId, accent());
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::black);
    setColour (juce::TextEditor::backgroundColourId, bgPanelLight());
    setColour (juce::TextEditor::textColourId, textPrimary());
    setColour (juce::TextEditor::highlightColourId, accent().withAlpha (0.4f));
}

juce::String PPGLookAndFeel::formatValueShort (float v)
{
    const float a = std::abs (v);
    const int signChars = (v < 0.0f) ? 1 : 0;

    int intDigits;
    if (a < 1.0f) intDigits = 1;
    else          intDigits = (int) std::floor (std::log10 (a)) + 1;

    int maxDecimals = 5 - signChars - intDigits - 1;
    if (maxDecimals < 0) maxDecimals = 0;
    if (maxDecimals > 3) maxDecimals = 3;

    return juce::String (v, maxDecimals);
}

// ============ ROTARY KNOB ============
void PPGLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                       int x, int y, int width, int height,
                                       float sliderPos,
                                       float rotaryStartAngle,
                                       float rotaryEndAngle,
                                       juce::Slider& slider)
{
    const auto bounds  = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (2.0f);
    const auto radius  = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre  = bounds.getCentre();
    const auto angle   = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    const bool enabled = slider.isEnabled();

    const float trackThickness   = juce::jmax (2.0f, radius * 0.16f);
    const float pointerThickness = juce::jmax (1.5f, radius * 0.09f);

    const juce::Colour fillColour   = enabled ? accent()       : juce::Colour (0xff333333);
    const juce::Colour pointerColour = enabled ? accentBright() : juce::Colour (0xff555555);

    {
        const auto outerR = radius + 1.0f;
        g.setColour (juce::Colour (0x77000000));
        g.fillEllipse (centre.x - outerR, centre.y - outerR + 0.7f,
                       outerR * 2.0f, outerR * 2.0f);
    }

    {
        juce::Path track;
        track.addCentredArc (centre.x, centre.y,
                             radius - trackThickness * 0.5f,
                             radius - trackThickness * 0.5f,
                             0.0f, rotaryStartAngle, rotaryEndAngle, true);

        g.setColour (juce::Colour (0xff181818));
        g.strokePath (track, juce::PathStrokeType (trackThickness + 1.5f,
                                                   juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

        g.setColour (juce::Colour (0xff2a2a2a));
        g.strokePath (track, juce::PathStrokeType (trackThickness,
                                                   juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
    }

    {
        juce::Path fill;
        fill.addCentredArc (centre.x, centre.y,
                            radius - trackThickness * 0.5f,
                            radius - trackThickness * 0.5f,
                            0.0f, rotaryStartAngle, angle, true);

        if (enabled)
        {
            g.setColour (fillColour.withAlpha (0.25f));
            g.strokePath (fill, juce::PathStrokeType (trackThickness + 2.5f,
                                                      juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
        }

        g.setColour (fillColour);
        g.strokePath (fill, juce::PathStrokeType (trackThickness,
                                                  juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    const auto innerRadius = radius - trackThickness - 2.0f;
    if (innerRadius > 1.0f)
    {
        const auto bodyRect = juce::Rectangle<float> (
            centre.x - innerRadius, centre.y - innerRadius,
            innerRadius * 2.0f, innerRadius * 2.0f);

        {
            juce::ColourGradient bodyGrad (
                juce::Colour (0xff252525),
                centre.x, centre.y - innerRadius * 0.40f,
                juce::Colour (0xff050505),
                centre.x, centre.y + innerRadius,
                true);
            g.setGradientFill (bodyGrad);
            g.fillEllipse (bodyRect);
        }

        g.setColour (juce::Colour (0xff3a3a3a));
        g.drawEllipse (bodyRect.reduced (0.5f), 1.0f);

        {
            juce::Path highlight;
            const float hR = innerRadius * 0.78f;
            highlight.addCentredArc (centre.x, centre.y, hR, hR,
                                     0.0f,
                                     juce::MathConstants<float>::pi * 1.18f,
                                     juce::MathConstants<float>::pi * 1.82f,
                                     true);
            g.setColour (juce::Colour (0x33ffffff));
            g.strokePath (highlight,
                          juce::PathStrokeType (juce::jmax (1.0f, innerRadius * 0.07f),
                                                juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
        }

        {
            juce::Path innerShadow;
            const float sR = innerRadius * 0.78f;
            innerShadow.addCentredArc (centre.x, centre.y, sR, sR,
                                       0.0f,
                                       juce::MathConstants<float>::pi * 0.18f,
                                       juce::MathConstants<float>::pi * 0.82f,
                                       true);
            g.setColour (juce::Colour (0x55000000));
            g.strokePath (innerShadow,
                          juce::PathStrokeType (juce::jmax (1.0f, innerRadius * 0.10f),
                                                juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
        }
    }

    {
        const float pointerLength = innerRadius * 0.85f;
        juce::Path pointer;
        pointer.startNewSubPath (0.0f, -innerRadius * 0.30f);
        pointer.lineTo           (0.0f, -innerRadius * 0.30f - pointerLength);

        if (enabled)
        {
            g.setColour (pointerColour.withAlpha (0.35f));
            g.strokePath (pointer,
                          juce::PathStrokeType (pointerThickness + 2.0f,
                                                juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded),
                          juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
        }

        g.setColour (pointerColour);
        g.strokePath (pointer,
                      juce::PathStrokeType (pointerThickness,
                                            juce::PathStrokeType::curved,
                                            juce::PathStrokeType::rounded),
                      juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
    }
}

// ============ LINEAR SLIDER ============
void PPGLookAndFeel::drawLinearSlider (juce::Graphics& g,
                                       int x, int y, int width, int height,
                                       float sliderPos, float minSliderPos, float maxSliderPos,
                                       juce::Slider::SliderStyle /*style*/,
                                       juce::Slider& slider)
{
    const bool isVertical = slider.isVertical();

    // ============ VERTICAL: pitch / mod wheel física ============
    if (isVertical)
    {
        const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
        const float wheelW = juce::jmin (bounds.getWidth() - 2.0f, 26.0f);
        const float wheelX = bounds.getCentreX() - wheelW * 0.5f;
        const float wheelY = bounds.getY() + 2.0f;
        const float wheelH = bounds.getHeight() - 4.0f;

        const auto wheelRect = juce::Rectangle<float> (wheelX, wheelY, wheelW, wheelH);

        // ---- Sombra exterior ----
        g.setColour (juce::Colour (0x99000000));
        g.fillRoundedRectangle (wheelRect.translated (-1.5f, 1.5f), 4.0f);

        // ---- Cuerpo cilíndrico (gradiente horizontal claro→oscuro→claro) ----
        {
            juce::ColourGradient cyl (
                juce::Colour (0xff0a0a0a), wheelRect.getX(), 0.0f,
                juce::Colour (0xff0a0a0a), wheelRect.getRight(), 0.0f,
                false);
            cyl.addColour (0.12, juce::Colour (0xff202020));
            cyl.addColour (0.38, juce::Colour (0xff6a6a6a));   // highlight principal
            cyl.addColour (0.55, juce::Colour (0xff484848));
            cyl.addColour (0.82, juce::Colour (0xff202020));
            g.setGradientFill (cyl);
            g.fillRoundedRectangle (wheelRect, 3.0f);
        }

        // ---- Borde ----
        g.setColour (juce::Colour (0xff2a2a2a));
        g.drawRoundedRectangle (wheelRect.reduced (0.5f), 3.0f, 1.0f);

        // ---- Surcos horizontales (textura de grip) ----
        {
            const int   numRidges = 28;
            const float ridgeH    = wheelH / (float) numRidges;
            for (int i = 0; i < numRidges; ++i)
            {
                const float ry = wheelY + i * ridgeH;

                // Surco oscuro (corte)
                g.setColour (juce::Colour (0x55000000));
                g.fillRect (wheelX + 2.0f, ry, wheelW - 4.0f, 1.0f);

                // Reflejo bajo el surco
                g.setColour (juce::Colour (0x1cffffff));
                g.fillRect (wheelX + 2.0f, ry + 1.0f, wheelW - 4.0f, 0.7f);
            }
        }

        // ---- Highlight borde derecho (curvatura) ----
        g.setColour (juce::Colour (0x33ffffff));
        g.fillRect (wheelX + wheelW - 2.5f, wheelY + 4.0f, 1.0f, wheelH - 8.0f);

        // ---- Sombra borde izquierdo ----
        g.setColour (juce::Colour (0x77000000));
        g.fillRect (wheelX + 1.0f, wheelY + 3.0f, 1.0f, wheelH - 6.0f);

        // ---- Thumb (línea dorada delgada, 7 px) ----
        const float thumbY = juce::jlimit (wheelY, wheelY + wheelH, sliderPos);
        {
            const float thumbH = 7.0f;
            const float thumbTop = juce::jlimit (wheelY,
                                                 wheelY + wheelH - thumbH,
                                                 thumbY - thumbH * 0.5f);

            const auto thumbRect = juce::Rectangle<float> (
                wheelX + 1.5f, thumbTop, wheelW - 3.0f, thumbH);

            // Sombra debajo
            g.setColour (juce::Colour (0xaa000000));
            g.fillRoundedRectangle (thumbRect.translated (0.0f, 1.2f), 1.5f);

            // Cuerpo dorado
            {
                juce::ColourGradient tGrad (juce::Colour (0xffffe08a),
                                            thumbRect.getX(), thumbRect.getY(),
                                            juce::Colour (0xffb07000),
                                            thumbRect.getX(), thumbRect.getBottom(),
                                            false);
                g.setGradientFill (tGrad);
                g.fillRoundedRectangle (thumbRect, 1.5f);
            }

            // Highlight superior
            g.setColour (juce::Colour (0xccffffff));
            g.fillRect (thumbRect.getX() + 1.0f, thumbRect.getY() + 0.5f,
                        thumbRect.getWidth() - 2.0f, 0.7f);

            // Borde fino
            g.setColour (juce::Colour (0xff5a3a00));
            g.drawRoundedRectangle (thumbRect.reduced (0.3f), 1.5f, 0.6f);
        }

        return;
    }

    // ============ HORIZONTAL: ribbon con surcos (para mod matrix) ============
    {
        const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (0.0f, 2.0f);
        const float trackH = juce::jmin (bounds.getHeight() - 4.0f, 12.0f);
        const float trackY = bounds.getCentreY() - trackH * 0.5f;

        const auto trackRect = juce::Rectangle<float> (
            bounds.getX(), trackY, bounds.getWidth(), trackH);

        {
            juce::ColourGradient grad (juce::Colour (0xff1a1a1a),
                                       trackRect.getX(), trackRect.getY(),
                                       juce::Colour (0xff050505),
                                       trackRect.getX(), trackRect.getBottom(),
                                       false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (trackRect, 2.0f);
        }

        g.setColour (juce::Colour (0xff2a2a2a));
        g.drawRoundedRectangle (trackRect.reduced (0.5f), 2.0f, 1.0f);

        {
            const int   numRidges = 30;
            const float ridgeW    = trackRect.getWidth() / (float) numRidges;
            for (int i = 0; i < numRidges; ++i)
            {
                const float rx = trackRect.getX() + i * ridgeW + 0.5f;
                const float rw = ridgeW - 1.0f;
                if (rw <= 0.0f) continue;

                g.setColour (juce::Colour (0xff101010));
                g.fillRect (rx, trackRect.getY() + 2.0f, rw, trackRect.getHeight() - 4.0f);

                g.setColour (juce::Colour (0x18ffffff));
                g.fillRect (rx, trackRect.getY() + 2.0f, rw, 0.8f);
            }
        }

        g.setColour (juce::Colour (0x22ffffff));
        g.fillRect (trackRect.getX() + 2.0f, trackRect.getY() + 1.0f,
                    trackRect.getWidth() - 4.0f, 0.8f);

        {
            const float thumbW = 14.0f;
            const float thumbH = trackH + 4.0f;
            const float thumbX = juce::jlimit (trackRect.getX(),
                                               trackRect.getRight() - thumbW,
                                               sliderPos - thumbW * 0.5f);
            const float thumbY = trackRect.getCentreY() - thumbH * 0.5f;

            const auto thumbRect = juce::Rectangle<float> (thumbX, thumbY, thumbW, thumbH);

            g.setColour (juce::Colour (0x88000000));
            g.fillRoundedRectangle (thumbRect.translated (0.0f, 1.0f), 2.0f);

            {
                juce::ColourGradient tGrad (juce::Colour (0xffffcc55),
                                            thumbRect.getX(), thumbRect.getY(),
                                            juce::Colour (0xffa06800),
                                            thumbRect.getX(), thumbRect.getBottom(),
                                            false);
                g.setGradientFill (tGrad);
                g.fillRoundedRectangle (thumbRect, 2.0f);
            }

            g.setColour (juce::Colour (0x88ffffff));
            g.fillRect (thumbRect.getX() + 1.0f, thumbRect.getY() + 1.0f,
                        thumbRect.getWidth() - 2.0f, 1.0f);

            g.setColour (juce::Colour (0xff5a3a00));
            g.drawRoundedRectangle (thumbRect.reduced (0.5f), 2.0f, 1.0f);
        }
    }
}

// ============ BUTTON ============
void PPGLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                           juce::Button& button,
                                           const juce::Colour& /*backgroundColour*/,
                                           bool shouldDrawButtonAsHighlighted,
                                           bool shouldDrawButtonAsDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const bool isOn   = button.getToggleState();

    juce::Colour fill;
    juce::Colour border;

    if (isOn)
    {
        fill   = accent();
        border = accentBright();
    }
    else
    {
        fill   = juce::Colour (0xff1c1c1c);
        border = borderSoft();
    }

    if (shouldDrawButtonAsDown)
        fill = fill.darker (0.15f);
    else if (shouldDrawButtonAsHighlighted)
        fill = fill.brighter (0.10f);

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, 3.0f);

    g.setColour (border);
    g.drawRoundedRectangle (bounds, 3.0f, 1.0f);
}

void PPGLookAndFeel::drawButtonText (juce::Graphics& g,
                                     juce::TextButton& button,
                                     bool /*shouldDrawButtonAsHighlighted*/,
                                     bool /*shouldDrawButtonAsDown*/)
{
    const auto bounds = button.getLocalBounds();
    const bool isOn   = button.getToggleState();

    const float fontSize = juce::jlimit (8.0f, 11.5f,
                                         (float) bounds.getHeight() * 0.55f);

    g.setFont (juce::Font (juce::FontOptions (fontSize, juce::Font::bold)));
    g.setColour (isOn ? juce::Colours::black : textPrimary());

    g.drawFittedText (button.getButtonText(), bounds, juce::Justification::centred, 1);
}

// ============ COMBOBOX ============
void PPGLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height,
                                   bool /*isButtonDown*/,
                                   int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                   juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);

    g.setColour (juce::Colour (0xff1c1c1c));
    g.fillRoundedRectangle (bounds, 3.0f);

    g.setColour (box.hasKeyboardFocus (true) ? accent() : borderSoft());
    g.drawRoundedRectangle (bounds, 3.0f, 1.0f);

    const auto arrowArea = bounds.removeFromRight (16.0f).reduced (4.0f, 0.0f);
    juce::Path arrow;
    arrow.startNewSubPath (arrowArea.getX(), arrowArea.getCentreY() - 2.0f);
    arrow.lineTo (arrowArea.getCentreX(), arrowArea.getCentreY() + 3.0f);
    arrow.lineTo (arrowArea.getRight(), arrowArea.getCentreY() - 2.0f);
    g.setColour (accent());
    g.strokePath (arrow, juce::PathStrokeType (1.6f));
}

juce::Font PPGLookAndFeel::getComboBoxFont (juce::ComboBox& box)
{
    return juce::Font (juce::FontOptions (juce::jlimit (10.0f, 13.0f,
                                       (float) box.getHeight() * 0.55f)));
}

juce::Font PPGLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::Font (juce::FontOptions (juce::jlimit (9.0f, 11.5f,
                                       (float) buttonHeight * 0.5f),
                                          juce::Font::bold));
}

juce::Font PPGLookAndFeel::getPopupMenuFont()
{
    return juce::Font (juce::FontOptions (14.0f));
}

void PPGLookAndFeel::getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator,
                                                int /*standardMenuItemHeight*/,
                                                int& idealWidth, int& idealHeight)
{
    if (isSeparator)
    {
        idealWidth  = 80;
        idealHeight = 8;
        return;
    }

    const auto font = getPopupMenuFont();

    juce::GlyphArrangement ga;
    ga.addLineOfText (font, text, 0.0f, 0.0f);
    const auto bbox = ga.getBoundingBox (0, ga.getNumGlyphs(), true);
    const int textW = (int) std::ceil (bbox.getWidth());

    idealHeight = juce::jmax (26, (int) std::ceil (font.getHeight() * 1.9f));
    idealWidth  = juce::jmax (140, textW + 60);
}
