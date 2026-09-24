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
}

// ============ ROTARY KNOB (mejorado con relieve) ============
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
    // Un anillo oscuro exterior muy sutil para dar profundidad.
    {
        const auto outerR = radius + 1.0f;
        g.setColour (juce::Colour (0x66000000));
        g.fillEllipse (centre.x - outerR, centre.y - outerR + 0.7f,
                       outerR * 2.0f, outerR * 2.0f);
    }

    // === 1. TRACK (fondo oscuro del arco) ===
    {
        juce::Path track;
        track.addCentredArc (centre.x, centre.y,
                             radius - trackThickness * 0.5f,
                             radius - trackThickness * 0.5f,
                             0.0f, rotaryStartAngle, rotaryEndAngle, true);

        // Track en dos capas: la de abajo más oscura (sombra interna del track),
        // la de arriba más clara (relieve)
        g.setColour (juce::Colour (0xff1a1a1a));
        g.strokePath (track, juce::PathStrokeType (trackThickness + 1.5f,
                                                   juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

        g.setColour (juce::Colour (0xff2a2a2a));
        g.strokePath (track, juce::PathStrokeType (trackThickness,
                                                   juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
    }

    // === 2. FILL (arco dorado hasta la posición actual, con leve glow) ===
    {
        juce::Path fill;
        fill.addCentredArc (centre.x, centre.y,
                            radius - trackThickness * 0.5f,
                            radius - trackThickness * 0.5f,
                            0.0f, rotaryStartAngle, angle, true);

        // Glow exterior
        if (enabled)
        {
            g.setColour (fillColour.withAlpha (0.25f));
            g.strokePath (fill, juce::PathStrokeType (trackThickness + 2.5f,
                                                      juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
        }

        // Fill principal
        g.setColour (fillColour);
        g.strokePath (fill, juce::PathStrokeType (trackThickness,
                                                  juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    // === 3. CUERPO DEL KNOB (círculo interior con gradiente radial) ===
    const auto innerRadius = radius - trackThickness - 2.0f;
    if (innerRadius > 1.0f)
    {
        const auto bodyRect = juce::Rectangle<float> (
            centre.x - innerRadius, centre.y - innerRadius,
            innerRadius * 2.0f, innerRadius * 2.0f);

        // 3a. Gradiente radial: más claro en el centro-arriba, más oscuro en el borde
        {
            juce::ColourGradient bodyGrad (
                juce::Colour (0xff1e1e1e),
                centre.x, centre.y - innerRadius * 0.35f,
                juce::Colour (0xff050505),
                centre.x, centre.y + innerRadius,
                true);   // radial

            g.setGradientFill (bodyGrad);
            g.fillEllipse (bodyRect);
        }

        // 3b. Borde exterior del cuerpo (leve relieve)
        g.setColour (juce::Colour (0xff3a3a3a));
        g.drawEllipse (bodyRect.reduced (0.5f), 1.0f);

        // 3c. Highlight superior (simula luz desde arriba)
        {
            juce::Path highlight;
            const float hR = innerRadius * 0.85f;
            const float hCy = centre.y - innerRadius * 0.30f;
            highlight.addCentredArc (centre.x, hCy, hR, hR * 0.55f,
                                     0.0f,
                                     juce::MathConstants<float>::pi * 1.15f,
                                     juce::MathConstants<float>::pi * 1.85f,
                                     true);
            g.setColour (juce::Colour (0x33ffffff));
            g.strokePath (highlight, juce::PathStrokeType (juce::jmax (1.0f, innerRadius * 0.08f),
                                                           juce::PathStrokeType::curved,
                                                           juce::PathStrokeType::rounded));
        }

        // 3d. Sombra interior inferior (leve)
        {
            juce::Path innerShadow;
            const float sR = innerRadius * 0.92f;
            const float sCy = centre.y + innerRadius * 0.35f;
            innerShadow.addCentredArc (centre.x, sCy, sR, sR * 0.5f,
                                       0.0f,
                                       juce::MathConstants<float>::pi * 0.15f,
                                       juce::MathConstants<float>::pi * 0.85f,
                                       true);
            g.setColour (juce::Colour (0x55000000));
            g.strokePath (innerShadow, juce::PathStrokeType (juce::jmax (1.0f, innerRadius * 0.12f),
                                                             juce::PathStrokeType::curved,
                                                             juce::PathStrokeType::rounded));
        }
    }

    // === 4. PUNTERO (línea dorada desde el centro hacia el borde) ===
    {
        const float pointerLength = innerRadius * 0.85f;
        juce::Path pointer;
        pointer.startNewSubPath (0.0f, -innerRadius * 0.30f);
        pointer.lineTo           (0.0f, -innerRadius * 0.30f - pointerLength);

        // Glow del puntero
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

    if (isVertical)
    {
        const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
        const float trackX = bounds.getCentreX();
        const float trackW = 4.0f;
        const float thumbR = 6.0f;

        g.setColour (juce::Colour (0xff2a2a2a));
        g.fillRoundedRectangle (trackX - trackW * 0.5f, bounds.getY(),
                                trackW, bounds.getHeight(), 2.0f);

        const float bottomY = juce::jmax (minSliderPos, maxSliderPos);
        const float topY    = juce::jmin (minSliderPos, maxSliderPos);

        if (sliderPos >= topY && sliderPos <= bottomY)
        {
            const float fillTop = sliderPos;
            const float fillH   = bottomY - sliderPos;
            if (fillH > 0.0f)
            {
                g.setColour (accent());
                g.fillRoundedRectangle (trackX - trackW * 0.5f, fillTop,
                                        trackW, fillH, 2.0f);
            }
        }

        g.setColour (accentBright());
        g.fillEllipse (trackX - thumbR, sliderPos - thumbR, thumbR * 2.0f, thumbR * 2.0f);
        g.setColour (juce::Colour (0xff151515));
        g.drawEllipse (trackX - thumbR, sliderPos - thumbR, thumbR * 2.0f, thumbR * 2.0f, 1.0f);

        return;
    }

    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    const float trackY    = bounds.getCentreY();
    const float trackH    = 4.0f;
    const float thumbR    = 6.0f;

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
        fill   = juce::Colour (0xff252525);
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

    g.setColour (juce::Colour (0xff252525));
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
