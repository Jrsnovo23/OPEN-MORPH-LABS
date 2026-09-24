#include "PPGLookAndFeel.h"

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

    // Labels: fondo negro + borde amarillo por defecto
    setColour (juce::Label::backgroundColourId, juce::Colour (0xff000000));
    setColour (juce::Label::textColourId,       juce::Colour (0xffffcc55));
    setColour (juce::Label::outlineColourId,    juce::Colour (0xffffaa00));
}

// ============ ROTARY KNOB (con relieve correcto) ============
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

    // === 0. SOMBRA EXTERIOR ===
    {
        const auto outerR = radius + 1.0f;
        g.setColour (juce::Colour (0x77000000));
        g.fillEllipse (centre.x - outerR, centre.y - outerR + 0.7f,
                       outerR * 2.0f, outerR * 2.0f);
    }

    // === 1. TRACK ===
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

    // === 2. FILL dorado con glow ===
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

    // === 3. CUERPO DEL KNOB ===
    const auto innerRadius = radius - trackThickness - 2.0f;
    if (innerRadius > 1.0f)
    {
        const auto bodyRect = juce::Rectangle<float> (
            centre.x - innerRadius, centre.y - innerRadius,
            innerRadius * 2.0f, innerRadius * 2.0f);

        // 3a. Gradiente radial (más claro arriba, más oscuro abajo)
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

        // 3b. Borde exterior del cuerpo
        g.setColour (juce::Colour (0xff3a3a3a));
        g.drawEllipse (bodyRect.reduced (0.5f), 1.0f);

        // 3c. HIGHLIGHT SUPERIOR — arco limpio sobre el propio círculo
        //     (NO aplastado, sino siguiendo el contorno)
        {
            juce::Path highlight;
            const float hR = innerRadius * 0.78f;   // radio del highlight
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

        // 3d. Sombra interior inferior — arco limpio pegado al borde inferior
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

    // === 4. PUNTERO ===
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

// ============ LINEAR SLIDER (pitch/mod wheels físicas) ============
void PPGLookAndFeel::drawLinearSlider (juce::Graphics& g,
                                       int x, int y, int width, int height,
                                       float sliderPos, float minSliderPos, float maxSliderPos,
                                       juce::Slider::SliderStyle /*style*/,
                                       juce::Slider& slider)
{
    const bool isVertical = slider.isVertical();

    if (isVertical)
    {
        // === VERTICAL: pitch / mod wheels FÍSICAS ===
        const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
        const float wheelW = juce::jmin (bounds.getWidth() - 2.0f, 24.0f);
        const float wheelX = bounds.getCentreX() - wheelW * 0.5f;
        const float wheelY = bounds.getY() + 2.0f;
        const float wheelH = bounds.getHeight() - 4.0f;

        const auto wheelRect = juce::Rectangle<float> (wheelX, wheelY, wheelW, wheelH);

        // 1. Sombra exterior (borde izquierdo)
        g.setColour (juce::Colour (0x88000000));
        g.fillRoundedRectangle (wheelRect.translated (-1.0f, 1.0f), 3.0f);

        // 2. Fondo del wheel (cuerpo con gradiente vertical)
        {
            juce::ColourGradient wGrad (juce::Colour (0xff1a1a1a),
                                        wheelRect.getX(), wheelRect.getY(),
                                        juce::Colour (0xff050505),
                                        wheelRect.getX(), wheelRect.getBottom(),
                                        false);
            g.setGradientFill (wGrad);
            g.fillRoundedRectangle (wheelRect, 3.0f);
        }

        // 3. Bordes (left darker, right lighter — relieve)
        g.setColour (juce::Colour (0xff2a2a2a));
        g.drawRoundedRectangle (wheelRect.reduced (0.5f), 3.0f, 1.0f);

        // 4. Rayas horizontales internas (aspecto de rueda física)
        {
            const int   numRidges = 24;
            const float ridgeH    = wheelH / (float) numRidges;
            for (int i = 0; i < numRidges; ++i)
            {
                const float ry = wheelY + i * ridgeH + 1.0f;
                const float rh = ridgeH - 1.5f;

                // Base oscura
                g.setColour (juce::Colour (0xff101010));
                g.fillRect (wheelX + 2.0f, ry, wheelW - 4.0f, rh);

                // Highlight sutil arriba de cada cresta
                g.setColour (juce::Colour (0x1affffff));
                g.fillRect (wheelX + 2.0f, ry, wheelW - 4.0f, 0.8f);
            }
        }

        // 5. Highlight lateral derecho (luz desde la derecha)
        {
            g.setColour (juce::Colour (0x22ffffff));
            g.fillRect (wheelX + wheelW - 2.0f, wheelY + 2.0f, 1.0f, wheelH - 4.0f);
        }

        // 6. Zona "activa" (dorada) desde el fondo hasta el thumb
        const float thumbY = juce::jlimit (wheelY, wheelY + wheelH, sliderPos);

        // Guardamos la geometría para dibujar el thumb encima

        // 7. Thumb (bloque ancho que ocupa el ancho del wheel)
        {
            const float thumbH = 14.0f;
            const float thumbTop = juce::jlimit (wheelY,
                                                 wheelY + wheelH - thumbH,
                                                 thumbY - thumbH * 0.5f);

            const auto thumbRect = juce::Rectangle<float> (
                wheelX + 1.0f, thumbTop, wheelW - 2.0f, thumbH);

            // Sombra del thumb
            g.setColour (juce::Colour (0x88000000));
            g.fillRoundedRectangle (thumbRect.translated (0.0f, 1.0f), 2.0f);

            // Cuerpo del thumb (gradiente dorado)
            {
                juce::ColourGradient tGrad (juce::Colour (0xffffcc55),
                                            thumbRect.getX(), thumbRect.getY(),
                                            juce::Colour (0xffa06800),
                                            thumbRect.getX(), thumbRect.getBottom(),
                                            false);
                g.setGradientFill (tGrad);
                g.fillRoundedRectangle (thumbRect, 2.0f);
            }

            // Highlight del thumb
            g.setColour (juce::Colour (0x88ffffff));
            g.fillRect (thumbRect.getX() + 1.0f, thumbRect.getY() + 1.0f,
                        thumbRect.getWidth() - 2.0f, 1.0f);

            // Borde
            g.setColour (juce::Colour (0xff5a3a00));
            g.drawRoundedRectangle (thumbRect.reduced (0.5f), 2.0f, 1.0f);
        }

        // 8. Flechas arriba y abajo del wheel (decorativas)
        {
            juce::Path downArrow;
            const float ax = wheelX + wheelW * 0.5f;
            const float ay = wheelY + wheelH + 3.0f;
            downArrow.startNewSubPath (ax - 4.0f, ay);
            downArrow.lineTo (ax, ay + 4.0f);
            downArrow.lineTo (ax + 4.0f, ay);
            g.setColour (juce::Colour (0xff666666));
            g.strokePath (downArrow, juce::PathStrokeType (1.2f));
        }

        return;
    }

    // === HORIZONTAL ===
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    const float trackY = bounds.getCentreY();
    const float trackH = 4.0f;
    const float thumbR = 6.0f;

    g.setColour (juce::Colour (0xff2a2a2a));
    g.fillRoundedRectangle (bounds.getX(), trackY - trackH * 0.5f,
                            bounds.getWidth(), trackH, 2.0f);

    const float minX = juce::jmin (minSliderPos, maxSliderPos);
    const float maxX = juce::jmax (minSliderPos, maxSliderPos);

    if (sliderPos >= minX && sliderPos <= maxX)
    {
        g.setColour (accent());
        g.fillRoundedRectangle (minX, trackY - trackH * 0.5f,
                                sliderPos - minX, trackH, 2.0f);
    }

    g.setColour (accentBright());
    g.fillEllipse (sliderPos - thumbR, trackY - thumbR, thumbR * 2.0f, thumbR * 2.0f);
    g.setColour (juce::Colour (0xff151515));
    g.drawEllipse (sliderPos - thumbR, trackY - thumbR, thumbR * 2.0f, thumbR * 2.0f, 1.0f);
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
