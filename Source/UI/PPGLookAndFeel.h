#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Label con caja redondeada (fondo negro + borde amarillo).
// Lo usamos para mostrar el valor numérico de cada knob.
class ValueBoxLabel : public juce::Label
{
public:
    ValueBoxLabel();
    void paint (juce::Graphics&) override;
};

// Look & Feel inspirado en el PPG Wave 3.3 original.
class PPGLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // ---- Paleta PPG (más oscura) ----
    static juce::Colour bgApp()        { return juce::Colour (0xff0a0a0a); }
    static juce::Colour bgPanel()      { return juce::Colour (0xff141414); }
    static juce::Colour bgPanelLight() { return juce::Colour (0xff1c1c1c); }
    static juce::Colour borderSoft()   { return juce::Colour (0xff2a2a2a); }
    static juce::Colour accent()       { return juce::Colour (0xffffaa00); }
    static juce::Colour accentBright() { return juce::Colour (0xffffcc55); }
    static juce::Colour accentDim()    { return juce::Colour (0xff7a5200); }
    static juce::Colour textPrimary()  { return juce::Colour (0xffe0e0e0); }
    static juce::Colour textDim()      { return juce::Colour (0xff888888); }

    PPGLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;

    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getPopupMenuFont() override;
    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator,
                                    int standardMenuItemHeight,
                                    int& idealWidth, int& idealHeight) override;

    // Formatea el valor de un knob a 5 caracteres máximo.
    //   0.5562064  →  "0.556"
    //   18.0       →  "18.00"
    //   -18.0      →  "-18.0"
    //   8000       →  "8000"
    static juce::String formatValueShort (float v);
};
