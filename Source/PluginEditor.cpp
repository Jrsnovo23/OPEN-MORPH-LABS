#include "PluginEditor.h"
#include "ParameterIDs.h"
#include "UI/PPGLookAndFeel.h"
#include "UI/Visualizers.h"
#include "UI/SeqStepControl.h"

// ==================== InfoDisplay ====================

void PPGWave3Editor::InfoDisplay::setInfo (const juce::String& name, const juce::String& value)
{
    paramName  = name;
    paramValue = value;
    repaint();
}

void PPGWave3Editor::InfoDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (juce::Colour (0xffffaa00));
    g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);

    auto textArea = getLocalBounds().reduced (juce::roundToInt (5.0f * scale), 1);
    g.setColour (juce::Colour (0xffffcc55));
    g.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(),
                                  juce::jmax (7.0f, 9.0f * scale), juce::Font::plain));

    if (paramValue.isEmpty())
    {
        g.drawText (paramName, textArea, juce::Justification::centredLeft);
        return;
    }

    const auto font = g.getCurrentFont();
    const int nameW  = (int) std::ceil (font.getStringWidthFloat (paramName))  + 2;
    const int valueW = (int) std::ceil (font.getStringWidthFloat (paramValue)) + 2;
    const int totalW = textArea.getWidth();
    const int gapPx  = 4;

    if (nameW + valueW + gapPx <= totalW)
    {
        g.drawText (paramName, textArea.removeFromLeft (nameW),
                    juce::Justification::centredLeft);
        g.drawText (paramValue, textArea,
                    juce::Justification::centredRight);
    }
    else
    {
        const int reserved = juce::jmin (valueW, (int) (totalW * 0.65f));
        auto valueArea = textArea.removeFromRight (reserved);

        g.drawFittedText (paramName, textArea,
                          juce::Justification::centredLeft, 1, 0.9f);
        g.drawText (paramValue, valueArea,
                    juce::Justification::centredRight);
    }
}

// ==================== PresetDisplay ====================

void PPGWave3Editor::PresetDisplay::setInfo (const juce::String& name,
                                             const juce::String& category,
                                             bool factory)
{
    presetName     = name;
    presetCategory = category;
    isFactory      = factory;
    repaint();
}

void PPGWave3Editor::PresetDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();

    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRoundedRectangle (r, 3.0f);

    g.setColour (juce::Colour (0xffffaa00));
    g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);

    auto inner = getLocalBounds().reduced (6, 2);

    const int arrowW = 14;
    auto arrowArea = inner.removeFromRight (arrowW);
    {
        juce::Path arrow;
        const float cx = (float) arrowArea.getCentreX();
        const float cy = (float) arrowArea.getCentreY();
        arrow.startNewSubPath (cx - 4.0f, cy - 2.0f);
        arrow.lineTo (cx, cy + 2.0f);
        arrow.lineTo (cx + 4.0f, cy - 2.0f);
        g.setColour (juce::Colour (0xffffaa00));
        g.strokePath (arrow, juce::PathStrokeType (1.6f));
    }

    g.setColour (juce::Colour (0xffffcc55));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText (presetName, inner, juce::Justification::centred, false);

    auto catRow = inner.removeFromBottom (10);
    g.setColour (isFactory ? juce::Colour (0xffffaa00)
                            : juce::Colour (0xff2ecc40));
    g.setFont (juce::FontOptions (8.0f, juce::Font::plain));
    const auto tag = isFactory ? "FACTORY" : "USER";
    g.drawText (presetCategory.toUpperCase() + "  -  " + tag,
                catRow, juce::Justification::centred, false);
}

void PPGWave3Editor::PresetDisplay::mouseDown (const juce::MouseEvent&)
{
    if (onOpenMenu)
        onOpenMenu();
}

// ==================== RotaryKnob ====================

PPGWave3Editor::RotaryKnob::RotaryKnob (juce::AudioProcessorValueTreeState& state,
                                        const juce::String& paramID,
                                        const juce::String& labelText,
                                        InfoDisplay* display)
    : infoDisplay (display), paramName (labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (0xffffaa00));
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff333333));
    slider.setColour (juce::Slider::thumbColourId,               juce::Colour (0xffffcc55));
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffaaaaaa));
    label.setColour (juce::Label::backgroundColourId, juce::Colour (0x00000000));
    label.setColour (juce::Label::outlineColourId,    juce::Colour (0x00000000));
    addAndMakeVisible (label);

    valueLabel.setText ("--", juce::dontSendNotification);
    addAndMakeVisible (valueLabel);

    slider.onValueChange = [this]()
    {
        const float v = (float) slider.getValue();
        valueLabel.setText (PPGLookAndFeel::formatValueShort (v),
                            juce::dontSendNotification);

        if (infoDisplay != nullptr)
            infoDisplay->setInfo (paramName, "");
    };

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        state, paramID, slider);

    valueLabel.setText (PPGLookAndFeel::formatValueShort (
                            (float) slider.getValue()),
                        juce::dontSendNotification);
}

void PPGWave3Editor::RotaryKnob::setScale (float s)
{
    scale = s;

    const float baseFont = juce::jmax (6.5f, 8.5f * scale);
    label.setFont      (juce::FontOptions (baseFont));
    valueLabel.setFont (juce::FontOptions (baseFont));

    resized();
}

void PPGWave3Editor::RotaryKnob::resized()
{
    auto r = getLocalBounds();

    const int labelH = juce::jmax (8, juce::roundToInt (10.0f * scale));
    const int valueH = juce::jmax (10, juce::roundToInt (12.0f * scale));

    label.setBounds (r.removeFromTop (labelH));

    auto valueRow = r.removeFromBottom (valueH);
    const int valueW = juce::jmin (valueRow.getWidth() - 6,
                                   juce::roundToInt (46.0f * scale));
    valueLabel.setBounds (valueRow.withSizeKeepingCentre (valueW, valueH));

    slider.setBounds (r.reduced (2, 0));
}

void PPGWave3Editor::RotaryKnob::paint (juce::Graphics&) {}

// ==================== HSlider ====================

PPGWave3Editor::HSlider::HSlider (juce::AudioProcessorValueTreeState& state,
                                  const juce::String& paramID,
                                  InfoDisplay* display)
    : infoDisplay (display)
{
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::trackColourId,       juce::Colour (0xffffaa00));
    slider.setColour (juce::Slider::backgroundColourId,  juce::Colour (0xff2a2a2a));
    slider.setColour (juce::Slider::thumbColourId,       juce::Colour (0xffffcc55));
    addAndMakeVisible (slider);

    slider.onValueChange = [this]()
    {
        if (infoDisplay != nullptr)
            infoDisplay->setInfo ("Mod Amount", slider.getTextFromValue (slider.getValue()));
    };

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        state, paramID, slider);
}

void PPGWave3Editor::HSlider::setScale (float s)
{
    scale = s;
    resized();
}

void PPGWave3Editor::HSlider::resized()
{
    slider.setBounds (getLocalBounds().reduced (2, 0));
}

void PPGWave3Editor::HSlider::paint (juce::Graphics&) {}

// ==================== ButtonSelector ====================

PPGWave3Editor::ButtonSelector::ButtonSelector (juce::AudioProcessorValueTreeState& state,
                                                const juce::String& paramID,
                                                const juce::StringArray& names)
    : apvtsRef (state), id (paramID)
{
    for (int i = 0; i < names.size(); ++i)
    {
        auto* b = buttons.add (new juce::TextButton (names[i]));
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff2a2a2a));
        b->setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
        b->setColour (juce::TextButton::textColourOffId,  juce::Colour (0xffcccccc));
        b->setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
        b->onClick = [this, i]()
        {
            if (attachment)
                attachment->setValueAsCompleteGesture ((float) i);
        };
        addAndMakeVisible (b);
    }

    attachment = std::make_unique<juce::ParameterAttachment> (
        *apvtsRef.getParameter (id),
        [this] (float newValue)
        {
            currentIndex = (int) newValue;
            refreshFromParameter();
        });
    attachment->sendInitialUpdate();
}

void PPGWave3Editor::ButtonSelector::refreshFromParameter()
{
    for (int i = 0; i < buttons.size(); ++i)
        buttons[i]->setToggleState (i == currentIndex, juce::dontSendNotification);
}

void PPGWave3Editor::ButtonSelector::resized()
{
    auto r = getLocalBounds();
    const int w = r.getWidth() / juce::jmax (1, buttons.size());
    for (auto* b : buttons)
        b->setBounds (r.removeFromLeft (w).reduced (1));
}

void PPGWave3Editor::ButtonSelector::paint (juce::Graphics&) {}

// ==================== ComboBoxSelector ====================

PPGWave3Editor::ComboBoxSelector::ComboBoxSelector (juce::AudioProcessorValueTreeState& state,
                                                    const juce::String& paramID,
                                                    const juce::String& labelText,
                                                    InfoDisplay* display)
    : infoDisplay (display), paramName (labelText)
{
    combo.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff2a2a2a));
    combo.setColour (juce::ComboBox::textColourId,       juce::Colour (0xffffcc55));
    combo.setColour (juce::ComboBox::outlineColourId,    juce::Colour (0xff555555));
    combo.setColour (juce::ComboBox::arrowColourId,      juce::Colour (0xffffaa00));
    addAndMakeVisible (combo);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centredLeft);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffcccccc));
    addAndMakeVisible (label);

    if (auto* param = state.getParameter (paramID))
    {
        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (param))
            combo.addItemList (choice->choices, 1);
    }

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        state, paramID, combo);

    combo.onChange = [this]()
    {
        if (infoDisplay != nullptr)
            infoDisplay->setInfo (paramName, combo.getText());
    };
}

void PPGWave3Editor::ComboBoxSelector::setScale (float s)
{
    scale = s;
    label.setFont (juce::FontOptions (juce::jmax (7.0f, 9.0f * scale)));
    resized();
}

void PPGWave3Editor::ComboBoxSelector::resized()
{
    auto r = getLocalBounds();
    const int labelH = juce::jmax (9, juce::roundToInt (11.0f * scale));
    label.setBounds (r.removeFromTop (labelH));
    combo.setBounds (r);
}

void PPGWave3Editor::ComboBoxSelector::paint (juce::Graphics&) {}

// ==================== ToggleButton ====================

PPGWave3Editor::ToggleButton::ToggleButton (juce::AudioProcessorValueTreeState& state,
                                            const juce::String& paramID,
                                            const juce::String& labelText,
                                            InfoDisplay* display)
    : infoDisplay (display), paramName (labelText)
{
    button.setButtonText (labelText);
    button.setClickingTogglesState (true);
    button.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff2a2a2a));
    button.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
    button.setColour (juce::TextButton::textColourOffId,  juce::Colour (0xffcccccc));
    button.setColour (juce::TextButton::textColourOnId,   juce::Colours::black);

    button.onClick = [this]()
    {
        if (infoDisplay != nullptr)
            infoDisplay->setInfo (paramName, button.getToggleState() ? "ON" : "OFF");
    };

    addAndMakeVisible (button);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        state, paramID, button);
}

void PPGWave3Editor::ToggleButton::setScale (float s)
{
    scale = s;
}

void PPGWave3Editor::ToggleButton::resized()
{
    button.setBounds (getLocalBounds());
}

void PPGWave3Editor::ToggleButton::paint (juce::Graphics&) {}

// ==================== EQBandKnob ====================

PPGWave3Editor::EQBandKnob::EQBandKnob (juce::AudioProcessorValueTreeState& state,
                                        const juce::StringArray& paramIdsForBands,
                                        const juce::String& labelText,
                                        InfoDisplay* display)
    : apvtsRef (state), ids (paramIdsForBands),
      infoDisplay (display), paramName (labelText)
{
    jassert (ids.size() == 4);

    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (0xffffaa00));
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff333333));
    slider.setColour (juce::Slider::thumbColourId,               juce::Colour (0xffffcc55));
    slider.setRange (0.0, 1.0, 0.0001);

    slider.onValueChange = [this]() { sliderChanged(); };
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffaaaaaa));
    label.setColour (juce::Label::backgroundColourId, juce::Colour (0x00000000));
    label.setColour (juce::Label::outlineColourId,    juce::Colour (0x00000000));
    addAndMakeVisible (label);

    valueLabel.setText ("--", juce::dontSendNotification);
    addAndMakeVisible (valueLabel);

    refreshSliderFromParam();
    startTimerHz (30);
}

PPGWave3Editor::EQBandKnob::~EQBandKnob()
{
    stopTimer();
}

void PPGWave3Editor::EQBandKnob::setActiveBand (int band)
{
    activeBand = juce::jlimit (0, 3, band);
    refreshSliderFromParam();
    repaint();
}

void PPGWave3Editor::EQBandKnob::setScale (float s)
{
    scale = s;

    const float baseFont = juce::jmax (6.5f, 8.5f * scale);
    label.setFont      (juce::FontOptions (baseFont));
    valueLabel.setFont (juce::FontOptions (baseFont));

    resized();
}

void PPGWave3Editor::EQBandKnob::resized()
{
    auto r = getLocalBounds();

    const int labelH = juce::jmax (8, juce::roundToInt (10.0f * scale));
    const int valueH = juce::jmax (10, juce::roundToInt (12.0f * scale));

    label.setBounds (r.removeFromTop (labelH));

    auto valueRow = r.removeFromBottom (valueH);
    const int valueW = juce::jmin (valueRow.getWidth() - 6,
                                   juce::roundToInt (46.0f * scale));
    valueLabel.setBounds (valueRow.withSizeKeepingCentre (valueW, valueH));

    slider.setBounds (r.reduced (2, 0));
}

void PPGWave3Editor::EQBandKnob::paint (juce::Graphics&) {}

void PPGWave3Editor::EQBandKnob::refreshSliderFromParam()
{
    const auto id = ids[activeBand];
    auto* param = apvtsRef.getParameter (id);
    if (param == nullptr) return;

    updatingFromParam = true;
    slider.setValue (param->getValue(), juce::dontSendNotification);
    updatingFromParam = false;

    updateInfoText();
}

void PPGWave3Editor::EQBandKnob::sliderChanged()
{
    if (updatingFromParam) return;

    const auto id = ids[activeBand];
    auto* param = apvtsRef.getParameter (id);
    if (param == nullptr) return;

    param->setValueNotifyingHost ((float) slider.getValue());
    updateInfoText();
}

void PPGWave3Editor::EQBandKnob::updateInfoText()
{
    const auto id = ids[activeBand];
    auto* param = apvtsRef.getParameter (id);
    if (param == nullptr) return;

    float realValue = 0.0f;
    if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
        realValue = ranged->getNormalisableRange().convertFrom0to1 (param->getValue());

    valueLabel.setText (PPGLookAndFeel::formatValueShort (realValue),
                        juce::dontSendNotification);

    if (infoDisplay != nullptr)
        infoDisplay->setInfo (paramName, "");
}

void PPGWave3Editor::EQBandKnob::timerCallback()
{
    const auto id = ids[activeBand];
    if (auto* param = apvtsRef.getParameter (id))
    {
        const float norm = param->getValue();
        if (std::abs (norm - (float) slider.getValue()) > 0.0005f)
        {
            updatingFromParam = true;
            slider.setValue (norm, juce::dontSendNotification);
            updatingFromParam = false;
            updateInfoText();
        }
    }
}
// ==================== EQCurveDisplay ====================

PPGWave3Editor::EQCurveDisplay::EQCurveDisplay (PPGWave3Processor& processor)
    : processorRef (processor), apvtsRef (processor.apvts)
{
    startTimerHz (30);
}

PPGWave3Editor::EQCurveDisplay::~EQCurveDisplay()
{
    stopTimer();
}

PPGWave3Editor::EQCurveDisplay::Cache
PPGWave3Editor::EQCurveDisplay::readParams() const
{
    Cache c;
    auto getF = [&] (const char* id, float def) -> float
    {
        if (auto* p = apvtsRef.getRawParameterValue (id)) return p->load();
        return def;
    };
    auto getB = [&] (const char* id, bool def) -> bool
    {
        if (auto* p = apvtsRef.getRawParameterValue (id)) return p->load() > 0.5f;
        return def;
    };

    c.on   = getB (ParamIDs::eqOn,   true);
    c.hpOn = getB (ParamIDs::eqHpOn, false);
    c.lpOn = getB (ParamIDs::eqLpOn, false);

    c.lowF  = getF (ParamIDs::eqLowFreq,  100.0f);
    c.lowQ  = getF (ParamIDs::eqLowQ,     0.707f);
    c.lowG  = getF (ParamIDs::eqLowGain,  0.0f);

    c.lmidF = getF (ParamIDs::eqLmidFreq, 500.0f);
    c.lmidQ = getF (ParamIDs::eqLmidQ,    0.707f);
    c.lmidG = getF (ParamIDs::eqLmidGain, 0.0f);

    c.hmidF = getF (ParamIDs::eqHmidFreq, 2000.0f);
    c.hmidQ = getF (ParamIDs::eqHmidQ,    0.707f);
    c.hmidG = getF (ParamIDs::eqHmidGain, 0.0f);

    c.highF = getF (ParamIDs::eqHighFreq, 8000.0f);
    c.highQ = getF (ParamIDs::eqHighQ,    0.707f);
    c.highG = getF (ParamIDs::eqHighGain, 0.0f);

    return c;
}

bool PPGWave3Editor::EQCurveDisplay::cacheChanged (const Cache& a, const Cache& b)
{
    const float eps = 0.001f;
    return a.on != b.on || a.hpOn != b.hpOn || a.lpOn != b.lpOn
        || std::abs (a.lowF  - b.lowF)  > eps || std::abs (a.lowQ  - b.lowQ)  > eps
        || std::abs (a.lowG  - b.lowG)  > eps
        || std::abs (a.lmidF - b.lmidF) > eps || std::abs (a.lmidQ - b.lmidQ) > eps
        || std::abs (a.lmidG - b.lmidG) > eps
        || std::abs (a.hmidF - b.hmidF) > eps || std::abs (a.hmidQ - b.hmidQ) > eps
        || std::abs (a.hmidG - b.hmidG) > eps
        || std::abs (a.highF - b.highF) > eps || std::abs (a.highQ - b.highQ) > eps
        || std::abs (a.highG - b.highG) > eps;
}

void PPGWave3Editor::EQCurveDisplay::timerCallback()
{
    // FASE 12: repintamos siempre para que el analizador se mueva en tiempo real.
    sonarPhase += 0.0133f;
    if (sonarPhase >= 1.0f) sonarPhase -= 1.0f;

    const auto now = readParams();
    cached = now;
    hasCached = true;
    repaint();
}

void PPGWave3Editor::EQCurveDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    if (r.getWidth() < 10.0f || r.getHeight() < 10.0f) return;

    const auto c = hasCached ? cached : readParams();
    hasCached = true;
    cached = c;

    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (juce::Colour (0xff2f2f2f));
    g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);

    // ===== FASE 12: analizador de espectro (detrás de la curva de EQ) =====
    {
        const int numBins = PPGWave3Processor::numSpectrumBins;
        const float dbFloor = -60.0f;
        const float dbCeil  = 0.0f;

        juce::Path spectrum;
        for (int i = 0; i < numBins; ++i)
        {
            const float t = (float) i / (float) (numBins - 1);
            const float x = r.getX() + t * r.getWidth();

            const float db = processorRef.getSpectrumMagnitude (i);
            const float normalized = juce::jlimit (0.0f, 1.0f,
                                                   (db - dbFloor) / (dbCeil - dbFloor));
            const float y = r.getBottom() - normalized * (r.getHeight() - 4.0f) - 2.0f;

            if (i == 0) spectrum.startNewSubPath (x, y);
            else        spectrum.lineTo (x, y);
        }

        juce::Path filledSpectrum = spectrum;
        filledSpectrum.lineTo (r.getRight(), r.getBottom());
        filledSpectrum.lineTo (r.getX(),     r.getBottom());
        filledSpectrum.closeSubPath();

        g.setColour (juce::Colour (0xffaaaaaa).withAlpha (0.10f));
        g.fillPath (filledSpectrum);

        g.setColour (juce::Colour (0xffdddddd).withAlpha (0.35f));
        g.strokePath (spectrum, juce::PathStrokeType (1.0f));
    }

    const float sr = 44100.0f;
    const float fMin = 20.0f;
    const float fMax = 20000.0f;
    const float dbRange = 24.0f;

    const float guides[] = { 100.0f, 1000.0f, 10000.0f };
    for (float f : guides)
    {
        const float t = std::log (f / fMin) / std::log (fMax / fMin);
        const float x = r.getX() + t * r.getWidth();

        g.setColour (juce::Colour (0xff222222));
        g.drawVerticalLine ((int) x, r.getY() + 2.0f, r.getBottom() - 2.0f);

        g.setColour (juce::Colour (0xff555555));
        g.setFont (juce::FontOptions (7.0f));
        const juce::String label = (f >= 1000.0f)
            ? juce::String ((int) (f / 1000.0f)) + "k"
            : juce::String ((int) f);
        g.drawText (label, (int) x - 12, (int) r.getBottom() - 10, 24, 10,
                    juce::Justification::centred);
    }

    const float midY = r.getCentreY();
    g.setColour (juce::Colour (0xff333333));
    g.drawHorizontalLine ((int) midY, r.getX() + 2.0f, r.getRight() - 2.0f);

    for (float db : { -12.0f, 12.0f })
    {
        const float y = midY - (db / dbRange) * (r.getHeight() * 0.45f);
        g.setColour (juce::Colour (0xff1e1e1e));
        g.drawHorizontalLine ((int) y, r.getX() + 2.0f, r.getRight() - 2.0f);
    }

    if (! c.on) return;

    const float lmidQ = juce::jlimit (0.1f, 10.0f, c.lmidQ);
    const float hmidQ = juce::jlimit (0.1f, 10.0f, c.hmidQ);

    const float lowGainLin  = juce::Decibels::decibelsToGain (c.lowG);
    const float lmidGainLin = juce::Decibels::decibelsToGain (c.lmidG);
    const float hmidGainLin = juce::Decibels::decibelsToGain (c.hmidG);
    const float highGainLin = juce::Decibels::decibelsToGain (c.highG);

    auto lowCoeffs  = juce::dsp::IIR::Coefficients<float>::makeLowShelf (
        sr, c.lowF, 0.707f, lowGainLin);
    auto lmidCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
        sr, c.lmidF, lmidQ, lmidGainLin);
    auto hmidCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
        sr, c.hmidF, hmidQ, hmidGainLin);
    auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf (
        sr, c.highF, 0.707f, highGainLin);
    auto hpCoeffs   = juce::dsp::IIR::Coefficients<float>::makeHighPass (
        sr, c.lowF, 0.707f);
    auto lpCoeffs   = juce::dsp::IIR::Coefficients<float>::makeLowPass (
        sr, c.highF, 0.707f);

    constexpr int numPoints = 128;
    juce::Path path;
    for (int i = 0; i < numPoints; ++i)
    {
        const float t = (float) i / (float) (numPoints - 1);
        const float freq = fMin * std::pow (fMax / fMin, t);

        double mag = 1.0;
        mag *= lowCoeffs ->getMagnitudeForFrequency (freq, sr);
        mag *= lmidCoeffs->getMagnitudeForFrequency (freq, sr);
        mag *= hmidCoeffs->getMagnitudeForFrequency (freq, sr);
        mag *= highCoeffs->getMagnitudeForFrequency (freq, sr);
        if (c.hpOn) mag *= hpCoeffs->getMagnitudeForFrequency (freq, sr);
        if (c.lpOn) mag *= lpCoeffs->getMagnitudeForFrequency (freq, sr);

        const float db = juce::jlimit (-dbRange, dbRange,
                                       juce::Decibels::gainToDecibels ((float) mag));
        const float x = r.getX() + t * r.getWidth();
        const float y = midY - (db / dbRange) * (r.getHeight() * 0.45f);

        if (i == 0) path.startNewSubPath (x, y);
        else        path.lineTo (x, y);
    }

    juce::Path filled = path;
    filled.lineTo (r.getRight(), midY);
    filled.lineTo (r.getX(),     midY);
    filled.closeSubPath();
    g.setColour (juce::Colour (0xffffaa00).withAlpha (0.12f));
    g.fillPath (filled);

    g.setColour (juce::Colour (0xffffaa00).withAlpha (0.35f));
    g.strokePath (path, juce::PathStrokeType (3.0f));
    g.setColour (juce::Colour (0xffffcc55));
    g.strokePath (path, juce::PathStrokeType (1.5f));

    struct Marker { float freq; float gain; };
    const Marker markers[] = {
        { c.lowF,  c.lowG  },
        { c.lmidF, c.lmidG },
        { c.hmidF, c.hmidG },
        { c.highF, c.highG },
    };
    for (const auto& m : markers)
    {
        const float t = std::log (m.freq / fMin) / std::log (fMax / fMin);
        const float x = r.getX() + juce::jlimit (0.0f, 1.0f, t) * r.getWidth();
        const float y = midY - (juce::jlimit (-dbRange, dbRange, m.gain) / dbRange)
                              * (r.getHeight() * 0.45f);

        g.setColour (juce::Colour (0xffffcc55));
        g.fillEllipse (x - 2.5f, y - 2.5f, 5.0f, 5.0f);
        g.setColour (juce::Colour (0xff151515));
        g.drawEllipse (x - 2.5f, y - 2.5f, 5.0f, 5.0f, 1.0f);
    }

    // FASE 12: overlay sonar
    ui::drawSonarOverlay (g, r, sonarPhase);
}

// ==================== FxTab ====================

PPGWave3Editor::FxTab::FxTab (juce::AudioProcessorValueTreeState& apvts,
                              const juce::String& toggleParamId,
                              const juce::String& label)
    : text (label)
{
    toggleBtn.setButtonText ("");
    toggleBtn.setClickingTogglesState (true);
    toggleBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff181818));
    toggleBtn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
    toggleBtn.setColour (juce::TextButton::textColourOffId,  juce::Colours::transparentBlack);
    toggleBtn.setColour (juce::TextButton::textColourOnId,   juce::Colours::transparentBlack);
    addAndMakeVisible (toggleBtn);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, toggleParamId, toggleBtn);
}

void PPGWave3Editor::FxTab::resized()
{
    auto r = getLocalBounds();
    auto bottomRow = r.removeFromBottom (20);
    toggleBtn.setBounds (bottomRow.reduced (2, 2));
}

void PPGWave3Editor::FxTab::paint (juce::Graphics& g)
{
    auto r = getLocalBounds();
    r.removeFromBottom (20);

    g.setColour (juce::Colour (0xffffaa00));
    g.setFont (juce::Font (juce::FontOptions (9.5f, juce::Font::bold)));

    g.drawText (text.toUpperCase(),
                r.getX() + 8, r.getY() + 3,
                r.getWidth() - 8, 12,
                juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xffffaa00).withAlpha (0.35f));
    g.fillRect (r.getX() + 8, r.getY() + 16,
                juce::jmin (r.getWidth() - 8, 100), 1);
}

// ==================== Constructor del editor ====================

PPGWave3Editor::PPGWave3Editor (PPGWave3Processor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      apvts (p.apvts),
      presetManager (p.apvts),
      prevBtn ("<"),
      nextBtn (">"),
      favoriteBtn ("*"),
      favoritesOnlyBtn ("FAV"),
      loadBtn ("LOAD"),
      saveBtn ("SAVE"),
      browseBtn ("BROWSE"),
      osc1Wave (std::make_unique<ButtonSelector> (p.apvts, ParamIDs::osc1Wave,
                  juce::StringArray { "SIN", "TRI", "SAW", "SQR" })),
      osc1Preview (p.apvts, ParamIDs::osc1Wave, ParamIDs::osc1Pos),
      osc1Pos   (p.apvts, ParamIDs::osc1Pos,    "POS",    &osc1Info),
      osc1Oct   (p.apvts, ParamIDs::osc1Octave, "OCT",    &osc1Info),
      osc1Semi  (p.apvts, ParamIDs::osc1Semi,   "SEMI",   &osc1Info),
      osc1Fine  (p.apvts, ParamIDs::osc1Fine,   "FINE",   &osc1Info),
      osc1Level (p.apvts, ParamIDs::osc1Level,  "LEVEL",  &osc1Info),
      osc2Wave (std::make_unique<ButtonSelector> (p.apvts, ParamIDs::osc2Wave,
                  juce::StringArray { "SIN", "TRI", "SAW", "SQR" })),
      osc2Preview (p.apvts, ParamIDs::osc2Wave, ParamIDs::osc2Pos),
      osc2Pos   (p.apvts, ParamIDs::osc2Pos,    "POS",    &osc2Info),
      osc2Oct   (p.apvts, ParamIDs::osc2Octave, "OCT",    &osc2Info),
      osc2Semi  (p.apvts, ParamIDs::osc2Semi,   "SEMI",   &osc2Info),
      osc2Fine  (p.apvts, ParamIDs::osc2Fine,   "FINE",   &osc2Info),
      osc2Level (p.apvts, ParamIDs::osc2Level,  "LEVEL",  &osc2Info),
      filterType (std::make_unique<ButtonSelector> (p.apvts, ParamIDs::filterType,
                    juce::StringArray { "LP", "HP", "BP" })),
      filterCutoff  (p.apvts, ParamIDs::filterCutoff,   "CUTOFF",  &filterInfo),
      filterReso    (p.apvts, ParamIDs::filterReso,     "RESO",    &filterInfo),
      filterEnvAmt  (p.apvts, ParamIDs::filterEnvAmt,   "ENV AMT", &filterInfo),
      filterKeyTrack(p.apvts, ParamIDs::filterKeyTrack, "KEY TRK", &filterInfo),
      env1Display (p.apvts, ParamIDs::ampAttack, ParamIDs::ampDecay,
                   ParamIDs::ampSustain, ParamIDs::ampRelease),
      env2Display (p.apvts, ParamIDs::filtAttack, ParamIDs::filtDecay,
                   ParamIDs::filtSustain, ParamIDs::filtRelease),
      env3Display (p.apvts, ParamIDs::env3Attack, ParamIDs::env3Decay,
                   ParamIDs::env3Sustain, ParamIDs::env3Release),
      env1A (p.apvts, ParamIDs::ampAttack,   "A", &envInfo),
      env1D (p.apvts, ParamIDs::ampDecay,    "D", &envInfo),
      env1S (p.apvts, ParamIDs::ampSustain,  "S", &envInfo),
      env1R (p.apvts, ParamIDs::ampRelease,  "R", &envInfo),
      env2A (p.apvts, ParamIDs::filtAttack,  "A", &envInfo),
      env2D (p.apvts, ParamIDs::filtDecay,   "D", &envInfo),
      env2S (p.apvts, ParamIDs::filtSustain, "S", &envInfo),
      env2R (p.apvts, ParamIDs::filtRelease, "R", &envInfo),
      env3A (p.apvts, ParamIDs::env3Attack,  "A", &envInfo),
      env3D (p.apvts, ParamIDs::env3Decay,   "D", &envInfo),
      env3S (p.apvts, ParamIDs::env3Sustain, "S", &envInfo),
      env3R (p.apvts, ParamIDs::env3Release, "R", &envInfo),
      master (p.apvts, ParamIDs::masterGain, "MASTER", &masterInfo),
      masterHztMeter (p.peakLevelL, p.peakLevelR, "MST"),
      compGrMeter    (p.compressorGR, "GR", true),
      lfo1Wave (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::lfo1Wave, "WAVE", &lfoInfo)),
      lfo1Display (p.apvts, ParamIDs::lfo1Wave),
      lfo1Rate  (p.apvts, ParamIDs::lfo1Rate,  "RATE",  &lfoInfo),
      lfo1Depth (p.apvts, ParamIDs::lfo1Depth, "DEPTH", &lfoInfo),
      lfo1Phase (p.apvts, ParamIDs::lfo1Phase, "PHASE", &lfoInfo),
      lfo1Sync  (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::lfo1Sync, "SYNC", &lfoInfo)),
      lfo2Wave (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::lfo2Wave, "WAVE", &lfoInfo)),
      lfo2Display (p.apvts, ParamIDs::lfo2Wave),
      lfo2Rate  (p.apvts, ParamIDs::lfo2Rate,  "RATE",  &lfoInfo),
      lfo2Depth (p.apvts, ParamIDs::lfo2Depth, "DEPTH", &lfoInfo),
      lfo2Phase (p.apvts, ParamIDs::lfo2Phase, "PHASE", &lfoInfo),
      lfo2Sync  (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::lfo2Sync, "SYNC", &lfoInfo)),
      mod1Src (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod1Source, "SRC", &modInfo)),
      mod1Dst (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod1Dest,   "DST", &modInfo)),
      mod1Amt (p.apvts, ParamIDs::mod1Amount, &modInfo),
      mod2Src (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod2Source, "SRC", &modInfo)),
      mod2Dst (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod2Dest,   "DST", &modInfo)),
      mod2Amt (p.apvts, ParamIDs::mod2Amount, &modInfo),
      mod3Src (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod3Source, "SRC", &modInfo)),
      mod3Dst (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod3Dest,   "DST", &modInfo)),
      mod3Amt (p.apvts, ParamIDs::mod3Amount, &modInfo),
      mod4Src (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod4Source, "SRC", &modInfo)),
      mod4Dst (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod4Dest,   "DST", &modInfo)),
      mod4Amt (p.apvts, ParamIDs::mod4Amount, &modInfo),
      driveTab   (std::make_unique<FxTab> (p.apvts, ParamIDs::driveOn,   "DIST")),
      chorusTab  (std::make_unique<FxTab> (p.apvts, ParamIDs::chorusOn,  "CHORUS")),
      phaserTab  (std::make_unique<FxTab> (p.apvts, ParamIDs::phaserOn,  "PHASER")),
      delayTab   (std::make_unique<FxTab> (p.apvts, ParamIDs::delayOn,   "DELAY")),
      reverbTab  (std::make_unique<FxTab> (p.apvts, ParamIDs::reverbOn,  "REVERB")),
      vintageTab (std::make_unique<FxTab> (p.apvts, ParamIDs::vintageOn, "VINTAGE")),
      eqTab      (std::make_unique<FxTab> (p.apvts, ParamIDs::eqOn,      "EQ")),
      compTab    (std::make_unique<FxTab> (p.apvts, ParamIDs::compOn,    "COMP")),
      driveAmount (p.apvts, ParamIDs::driveAmount, "AMT",  &fxInfoDrive),
      driveTone   (p.apvts, ParamIDs::driveTone,   "TONE", &fxInfoDrive),
      driveMix    (p.apvts, ParamIDs::driveMix,    "MIX",  &fxInfoDrive),
      chorusRate  (p.apvts, ParamIDs::chorusRate,  "RATE",  &fxInfoChorus),
      chorusDepth (p.apvts, ParamIDs::chorusDepth, "DEPTH", &fxInfoChorus),
      chorusMix   (p.apvts, ParamIDs::chorusMix,   "MIX",   &fxInfoChorus),
      delaySync (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::delaySync, "SYNC", &fxInfoDelay)),
      delayTime     (p.apvts, ParamIDs::delayTime,     "TIME",  &fxInfoDelay),
      delayFeedback (p.apvts, ParamIDs::delayFeedback, "FEEDBK",&fxInfoDelay),
      delayMix      (p.apvts, ParamIDs::delayMix,      "MIX",   &fxInfoDelay),
      reverbSize (p.apvts, ParamIDs::reverbSize, "SIZE", &fxInfoReverb),
      reverbDamp (p.apvts, ParamIDs::reverbDamp, "DAMP", &fxInfoReverb),
      reverbMix  (p.apvts, ParamIDs::reverbMix,  "MIX",  &fxInfoReverb),
      vintageAmount (p.apvts, ParamIDs::vintageAmount, "AMOUNT", &fxInfoVintage),
      vintageBits   (p.apvts, ParamIDs::vintageBits,   "BITS",   &fxInfoVintage),
      vintageSr     (p.apvts, ParamIDs::vintageSr,     "SR",     &fxInfoVintage),
      vintageNoise  (p.apvts, ParamIDs::vintageNoise,  "NOISE",  &fxInfoVintage),
      vintageDrift  (p.apvts, ParamIDs::vintageDrift,  "DRIFT",  &fxInfoVintage),
      vintageVar    (p.apvts, ParamIDs::vintageVar,    "VAR",    &fxInfoVintage),
      eqHpOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::eqHpOn, "HP", &fxInfoEq)),
      eqLpOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::eqLpOn, "LP", &fxInfoEq)),
      eqFreqKnob (p.apvts,
                  { ParamIDs::eqLowFreq, ParamIDs::eqLmidFreq,
                    ParamIDs::eqHmidFreq, ParamIDs::eqHighFreq },
                  "FREQ", &fxInfoEq),
      eqQKnob    (p.apvts,
                  { ParamIDs::eqLowQ, ParamIDs::eqLmidQ,
                    ParamIDs::eqHmidQ, ParamIDs::eqHighQ },
                  "Q", &fxInfoEq),
      eqGainKnob (p.apvts,
                  { ParamIDs::eqLowGain, ParamIDs::eqLmidGain,
                    ParamIDs::eqHmidGain, ParamIDs::eqHighGain },
                  "GAIN", &fxInfoEq),
      eqCurveDisplay (p),
      phaserRate     (p.apvts, ParamIDs::phaserRate,     "RATE",  &fxInfoPhaser),
      phaserDepth    (p.apvts, ParamIDs::phaserDepth,    "DEPTH", &fxInfoPhaser),
      phaserFeedback (p.apvts, ParamIDs::phaserFeedback, "FEEDBK",&fxInfoPhaser),
      phaserMix      (p.apvts, ParamIDs::phaserMix,      "MIX",   &fxInfoPhaser),
      compSidechain (std::make_unique<ToggleButton> (p.apvts, ParamIDs::compSidechain, "SC", &fxInfoComp)),
      compThreshold (p.apvts, ParamIDs::compThreshold, "THRSH",  &fxInfoComp),
      compRatio     (p.apvts, ParamIDs::compRatio,     "RATIO",  &fxInfoComp),
      compAttack    (p.apvts, ParamIDs::compAttack,    "ATTACK", &fxInfoComp),
      compRelease   (p.apvts, ParamIDs::compRelease,   "RELSE",  &fxInfoComp),
      compKnee      (p.apvts, ParamIDs::compKnee,      "KNEE",   &fxInfoComp),
      compMakeup    (p.apvts, ParamIDs::compMakeup,    "MAKEUP", &fxInfoComp),
      compScAmount  (p.apvts, ParamIDs::compScAmount,  "SC AMT", &fxInfoComp),
      keyboardComponent (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    juce::ignoreUnused (processorRef, apvts);

    prevBtn.setConnectedEdges (juce::Button::ConnectedOnRight);
    nextBtn.setConnectedEdges (juce::Button::ConnectedOnLeft);

    prevBtn.onClick = [this]() { onPrevPreset(); };
    nextBtn.onClick = [this]() { onNextPreset(); };
    favoriteBtn.onClick      = [this]() { onToggleFavorite(); };
    favoritesOnlyBtn.onClick = [this]() { onToggleFavoritesOnly(); };
    loadBtn.onClick = [this]() { onLoadPreset(); };
    saveBtn.onClick = [this]() { onSavePreset(); };
    browseBtn.onClick = [this]() { onBrowsePreset(); };

    for (auto* b : { &prevBtn, &nextBtn, &favoriteBtn, &favoritesOnlyBtn,
                     &loadBtn, &saveBtn, &browseBtn })
        addAndMakeVisible (b);

    favoriteBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff252525));
    favoriteBtn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
    favoriteBtn.setColour (juce::TextButton::textColourOffId,  juce::Colour (0xff999999));
    favoriteBtn.setColour (juce::TextButton::textColourOnId,   juce::Colours::black);

    favoritesOnlyBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff252525));
    favoritesOnlyBtn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
    favoritesOnlyBtn.setColour (juce::TextButton::textColourOffId,  juce::Colour (0xff999999));
    favoritesOnlyBtn.setColour (juce::TextButton::textColourOnId,   juce::Colours::black);

    presetDisplay.onOpenMenu = [this]() { showPresetMenu(); };

    addAndMakeVisible (presetDisplay);
    updatePresetDisplay();

    for (auto* t : { driveTab.get(), chorusTab.get(), phaserTab.get(),
                     delayTab.get(), reverbTab.get(), vintageTab.get(),
                     eqTab.get(), compTab.get() })
        addAndMakeVisible (*t);

    for (auto* b : { &env1TabBtn, &env2TabBtn, &env3TabBtn })
    {
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff1c1c1c));
        b->setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
        b->setColour (juce::TextButton::textColourOffId,  juce::Colour (0xffaaaaaa));
        b->setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
        addAndMakeVisible (b);
    }

    env1TabBtn.setButtonText ("ENV1");
    env2TabBtn.setButtonText ("ENV2");
    env3TabBtn.setButtonText ("ENV3");

    env1TabBtn.onClick = [this]() { activeEnvTab = 0; updateEnvVisibility(); repaint(); };
    env2TabBtn.onClick = [this]() { activeEnvTab = 1; updateEnvVisibility(); repaint(); };
    env3TabBtn.onClick = [this]() { activeEnvTab = 2; updateEnvVisibility(); repaint(); };

    for (auto* b : { &eqLowBtn, &eqLmidBtn, &eqHmidBtn, &eqHighBtn })
    {
        b->setClickingTogglesState (true);
        b->setRadioGroupId (0xE0B);
        b->setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff2a2a2a));
        b->setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
        b->setColour (juce::TextButton::textColourOffId,  juce::Colour (0xffcccccc));
        b->setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
        addAndMakeVisible (b);
    }
    eqLowBtn .setButtonText ("LOW");
    eqLmidBtn.setButtonText ("LMID");
    eqHmidBtn.setButtonText ("HMID");
    eqHighBtn.setButtonText ("HIGH");

    eqLowBtn .onClick = [this]() { setActiveEqBand (0); };
    eqLmidBtn.onClick = [this]() { setActiveEqBand (1); };
    eqHmidBtn.onClick = [this]() { setActiveEqBand (2); };
    eqHighBtn.onClick = [this]() { setActiveEqBand (3); };

    setActiveEqBand (0);
    addAndMakeVisible (eqCurveDisplay);

    keyboardComponent.setAvailableRange (24, 108);
    keyboardComponent.setLowestVisibleKey (24);
    keyboardComponent.setScrollButtonsVisible (false);
    keyboardComponent.setColour (juce::MidiKeyboardComponent::whiteNoteColourId,
                                 juce::Colour (0xffe8e8e8));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::blackNoteColourId,
                                 juce::Colour (0xff1a1a1a));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId,
                                 juce::Colour (0xff2f2f2f));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,
                                 juce::Colour (0xffffaa00).withAlpha (0.35f));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId,
                                 juce::Colour (0xffffaa00).withAlpha (0.75f));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::shadowColourId,
                                 juce::Colour (0x66000000));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::textLabelColourId,
                                 juce::Colour (0xff444444));
    addAndMakeVisible (keyboardComponent);

    pitchWheelSlider.setSliderStyle (juce::Slider::LinearVertical);
    pitchWheelSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    pitchWheelSlider.setRange (-1.0, 1.0, 0.001);
    pitchWheelSlider.setValue (0.0, juce::dontSendNotification);
    pitchWheelSlider.setColour (juce::Slider::trackColourId,      juce::Colour (0xffffaa00));
    pitchWheelSlider.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff2a2a2a));
    pitchWheelSlider.setColour (juce::Slider::thumbColourId,      juce::Colour (0xffffcc55));
    pitchWheelSlider.onValueChange = [this]()
    {
        processorRef.pitchBendAtomic.store ((float) pitchWheelSlider.getValue());
    };
    pitchWheelSlider.onDragEnd = [this]()
    {
        pitchWheelSlider.setValue (0.0, juce::sendNotificationSync);
    };
    addAndMakeVisible (pitchWheelSlider);

    pitchWheelLabel.setText ("PITCH", juce::dontSendNotification);
    pitchWheelLabel.setJustificationType (juce::Justification::centred);
    pitchWheelLabel.setColour (juce::Label::textColourId, juce::Colour (0xffaaaaaa));
    pitchWheelLabel.setFont (juce::FontOptions (9.0f, juce::Font::bold));
    addAndMakeVisible (pitchWheelLabel);

    modWheelSlider.setSliderStyle (juce::Slider::LinearVertical);
    modWheelSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    modWheelSlider.setRange (0.0, 1.0, 0.001);
    modWheelSlider.setValue (0.0, juce::dontSendNotification);
    modWheelSlider.setColour (juce::Slider::trackColourId,      juce::Colour (0xffffaa00));
    modWheelSlider.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff2a2a2a));
    modWheelSlider.setColour (juce::Slider::thumbColourId,      juce::Colour (0xffffcc55));
    modWheelSlider.onValueChange = [this]()
    {
        processorRef.modWheelAtomic.store ((float) modWheelSlider.getValue());
    };
    addAndMakeVisible (modWheelSlider);

    modWheelLabel.setText ("MOD", juce::dontSendNotification);
    modWheelLabel.setJustificationType (juce::Justification::centred);
    modWheelLabel.setColour (juce::Label::textColourId, juce::Colour (0xffaaaaaa));
    modWheelLabel.setFont (juce::FontOptions (9.0f, juce::Font::bold));
    addAndMakeVisible (modWheelLabel);

    addAndMakeVisible (osc1Preview);
    addAndMakeVisible (osc2Preview);
    addAndMakeVisible (lfo1Display);
    addAndMakeVisible (lfo2Display);
    addAndMakeVisible (env1Display);
    addAndMakeVisible (env2Display);
    addAndMakeVisible (env3Display);
    addAndMakeVisible (masterHztMeter);
    addAndMakeVisible (compGrMeter);

    addAndMakeVisible (*osc1Wave);
    addAndMakeVisible (*osc2Wave);
    addAndMakeVisible (*filterType);
    addAndMakeVisible (*lfo1Wave);   addAndMakeVisible (*lfo1Sync);
    addAndMakeVisible (*lfo2Wave);   addAndMakeVisible (*lfo2Sync);
    addAndMakeVisible (*mod1Src);    addAndMakeVisible (*mod1Dst);
    addAndMakeVisible (*mod2Src);    addAndMakeVisible (*mod2Dst);
    addAndMakeVisible (*mod3Src);    addAndMakeVisible (*mod3Dst);
    addAndMakeVisible (*mod4Src);    addAndMakeVisible (*mod4Dst);
    addAndMakeVisible (*delaySync);
    addAndMakeVisible (*eqHpOn);
    addAndMakeVisible (*eqLpOn);
    addAndMakeVisible (*compSidechain);

    addAndMakeVisible (eqFreqKnob);
    addAndMakeVisible (eqQKnob);
    addAndMakeVisible (eqGainKnob);

    for (auto* d : { &osc1Info, &osc2Info, &filterInfo,
                     &envInfo, &masterInfo,
                     &lfoInfo, &modInfo,
                     &fxInfoDrive, &fxInfoChorus, &fxInfoPhaser, &fxInfoDelay,
                     &fxInfoReverb, &fxInfoVintage, &fxInfoEq, &fxInfoComp })
        addAndMakeVisible (d);

    std::initializer_list<juce::Component*> allKnobs {
        &osc1Pos, &osc1Oct, &osc1Semi, &osc1Fine, &osc1Level,
        &osc2Pos, &osc2Oct, &osc2Semi, &osc2Fine, &osc2Level,
        &filterCutoff, &filterReso, &filterEnvAmt, &filterKeyTrack,
        &env1A, &env1D, &env1S, &env1R,
        &env2A, &env2D, &env2S, &env2R,
        &env3A, &env3D, &env3S, &env3R,
        &master,
        &lfo1Rate, &lfo1Depth, &lfo1Phase,
        &lfo2Rate, &lfo2Depth, &lfo2Phase,
        &driveAmount, &driveTone, &driveMix,
        &chorusRate, &chorusDepth, &chorusMix,
        &delayTime, &delayFeedback, &delayMix,
        &reverbSize, &reverbDamp, &reverbMix,
        &phaserRate, &phaserDepth, &phaserFeedback, &phaserMix,
        &vintageAmount, &vintageBits, &vintageSr,
        &vintageNoise, &vintageDrift, &vintageVar,
        &compThreshold, &compRatio, &compAttack, &compRelease,
        &compKnee, &compMakeup, &compScAmount,
        &mod1Amt, &mod2Amt, &mod3Amt, &mod4Amt
    };
    for (auto* c : allKnobs)
        addAndMakeVisible (c);

    setLookAndFeel (&ppgLnf);
    setResizable (false, false);

    // FASE 12: conectar los visualizadores con las fases reales del motor
    osc1Preview.setPhaseSource (&p.osc1Phase);
    osc2Preview.setPhaseSource (&p.osc2Phase);
    lfo1Display.setPhaseSource (&p.lfo1Phase);
    lfo2Display.setPhaseSource (&p.lfo2Phase);

    // ===== FASE 10: secuenciador de pasos =====
    {
        // Los 16 controles de paso
        for (int i = 0; i < StepSequencer::numSteps; ++i)
        {
            auto* sc = new SeqStepControl (p.sequencer, i);
            seqSteps.add (sc);
            addAndMakeVisible (*sc);
        }

        // ON/OFF global
        seqOnOffBtn.setButtonText ("SEQ");
        seqOnOffBtn.setClickingTogglesState (true);
        seqOnOffBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff202020));
        seqOnOffBtn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
        seqOnOffBtn.setColour (juce::TextButton::textColourOffId,  juce::Colour (0xff808080));
        seqOnOffBtn.setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
        seqOnOffBtn.onClick = [this]()
        {
            processorRef.sequencer.enabled.store (seqOnOffBtn.getToggleState());
        };
        addAndMakeVisible (seqOnOffBtn);

        // CLR (reset)
        seqResetBtn.setButtonText ("CLR");
        seqResetBtn.setColour (juce::TextButton::buttonColourId,  juce::Colour (0xff202020));
        seqResetBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffcccccc));
        seqResetBtn.onClick = [this]() { resetSequencer(); };
        addAndMakeVisible (seqResetBtn);

        // RATE
        seqRateCombo.addItemList (
            { "1/1", "1/2", "1/4", "1/8", "1/16", "1/4T", "1/8T", "1/16T", "1/4." }, 1);
        seqRateCombo.setSelectedId (4, juce::dontSendNotification);
        seqRateCombo.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1c1c1c));
        seqRateCombo.setColour (juce::ComboBox::textColourId,       juce::Colour (0xffffcc55));
        seqRateCombo.setColour (juce::ComboBox::outlineColourId,    juce::Colour (0xff555555));
        seqRateCombo.setColour (juce::ComboBox::arrowColourId,      juce::Colour (0xffffaa00));
        seqRateCombo.onChange = [this]()
        {
            processorRef.sequencer.rateIndex.store (seqRateCombo.getSelectedId() - 1);
        };
        addAndMakeVisible (seqRateCombo);

        // DIRECTION
        seqDirCombo.addItemList ({ "FWD", "REV", "PING", "RND" }, 1);
        seqDirCombo.setSelectedId (1, juce::dontSendNotification);
        seqDirCombo.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1c1c1c));
        seqDirCombo.setColour (juce::ComboBox::textColourId,       juce::Colour (0xffffcc55));
        seqDirCombo.setColour (juce::ComboBox::outlineColourId,    juce::Colour (0xff555555));
        seqDirCombo.setColour (juce::ComboBox::arrowColourId,      juce::Colour (0xffffaa00));
        seqDirCombo.onChange = [this]()
        {
            processorRef.sequencer.direction.store (seqDirCombo.getSelectedId() - 1);
        };
        addAndMakeVisible (seqDirCombo);

        // SWING
        seqSwingSlider.setSliderStyle (juce::Slider::LinearHorizontal);
        seqSwingSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        seqSwingSlider.setRange (0.0, 1.0, 0.01);
        seqSwingSlider.setValue (0.0, juce::dontSendNotification);
        seqSwingSlider.setColour (juce::Slider::trackColourId,      juce::Colour (0xffffaa00));
        seqSwingSlider.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff2a2a2a));
        seqSwingSlider.setColour (juce::Slider::thumbColourId,      juce::Colour (0xffffcc55));
        seqSwingSlider.onValueChange = [this]()
        {
            processorRef.sequencer.swing.store ((float) seqSwingSlider.getValue());
        };
        addAndMakeVisible (seqSwingSlider);

        // LENGTH
        seqLengthSlider.setSliderStyle (juce::Slider::LinearHorizontal);
        seqLengthSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        seqLengthSlider.setRange (1.0, 16.0, 1.0);
        seqLengthSlider.setValue (16.0, juce::dontSendNotification);
        seqLengthSlider.setColour (juce::Slider::trackColourId,      juce::Colour (0xffffaa00));
        seqLengthSlider.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff2a2a2a));
        seqLengthSlider.setColour (juce::Slider::thumbColourId,      juce::Colour (0xffffcc55));
        seqLengthSlider.onValueChange = [this]()
        {
            processorRef.sequencer.length.store ((int) seqLengthSlider.getValue());
        };
        addAndMakeVisible (seqLengthSlider);

        // BASE NOTE
        seqBaseNoteSlider.setSliderStyle (juce::Slider::LinearHorizontal);
        seqBaseNoteSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        seqBaseNoteSlider.setRange (0.0, 127.0, 1.0);
        seqBaseNoteSlider.setValue (60.0, juce::dontSendNotification);
        seqBaseNoteSlider.setColour (juce::Slider::trackColourId,      juce::Colour (0xffffaa00));
        seqBaseNoteSlider.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff2a2a2a));
        seqBaseNoteSlider.setColour (juce::Slider::thumbColourId,      juce::Colour (0xffffcc55));
        seqBaseNoteSlider.onValueChange = [this]()
        {
            processorRef.sequencer.baseNote.store ((int) seqBaseNoteSlider.getValue());
        };
        addAndMakeVisible (seqBaseNoteSlider);

        startTimerHz (20);
    }

    // El setSize va al final, DESPUÉS de crear todos los componentes,
    // para que resized() no acceda a seqSteps vacío.
    setSize (1280, 920);

    updateFxVisibility();
    updateEnvVisibility();
}

PPGWave3Editor::~PPGWave3Editor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

// ==================== EQ band switching ====================

void PPGWave3Editor::setActiveEqBand (int band)
{
    activeEqBand = juce::jlimit (0, 3, band);
    eqFreqKnob.setActiveBand (activeEqBand);
    eqQKnob   .setActiveBand (activeEqBand);
    eqGainKnob.setActiveBand (activeEqBand);

    const bool qApplicable = (activeEqBand == 1 || activeEqBand == 2);
    eqQKnob.setEnabled (qApplicable);

    eqLowBtn .setToggleState (activeEqBand == 0, juce::dontSendNotification);
    eqLmidBtn.setToggleState (activeEqBand == 1, juce::dontSendNotification);
    eqHmidBtn.setToggleState (activeEqBand == 2, juce::dontSendNotification);
    eqHighBtn.setToggleState (activeEqBand == 3, juce::dontSendNotification);
}

// ==================== Preset menu ====================

void PPGWave3Editor::showPresetMenu()
{
    juce::PopupMenu menu;

    const auto& allPresets = presetManager.getAllPresets();
    const int currentIdx = presetManager.getCurrentIndex();

    menu.addItem (10000, "Mostrar solo favoritos",
                  true, presetManager.isFavoritesOnly());

    menu.addSeparator();

    {
        juce::PopupMenu favMenu;
        int favCount = 0;
        for (int i = 0; i < allPresets.size(); ++i)
        {
            const auto& p = allPresets.getReference (i);
            if (! presetManager.isFavorite (p.name)) continue;

            const bool isCur = (i == currentIdx);
            favMenu.addItem (i + 1, "  " + p.name, true, isCur);
            ++favCount;
        }

        if (favCount == 0)
            favMenu.addItem (-1, "(sin favoritos)", false, false);

        menu.addSubMenu (juce::String::fromUTF8 ("\xe2\x98\x85 Favoritos (")
                            + juce::String (favCount) + ")",
                         favMenu, favCount > 0);
    }

    menu.addSeparator();

    juce::StringArray categories;
    for (const auto& p : allPresets)
        if (! categories.contains (p.category))
            categories.add (p.category);

    for (const auto& cat : categories)
    {
        juce::PopupMenu sub;

        for (int i = 0; i < allPresets.size(); ++i)
        {
            const auto& p = allPresets.getReference (i);
            if (p.category != cat) continue;

            const bool fav   = presetManager.isFavorite (p.name);
            const bool isCur = (i == currentIdx);

            juce::String prefix = "   ";
            if (fav) prefix = juce::String::fromUTF8 ("\xe2\x98\x85  ");

            sub.addItem (i + 1, prefix + p.name, true, isCur);
        }

        menu.addSubMenu (cat, sub);
    }

    menu.addSeparator();
    menu.addItem (-2, "Cmd+click = marcar/desmarcar \xe2\x98\x85", false, false);

    menu.showMenuAsync (juce::PopupMenu::Options().withMinimumWidth (260),
        [this] (int result)
        {
            if (result == 0) return;

            if (result == 10000)
            {
                onToggleFavoritesOnly();
                return;
            }

            if (result < 0) return;

            const int presetIdx = result - 1;
            if (presetIdx < 0 || presetIdx >= presetManager.getAllPresets().size())
                return;

            const auto mods = juce::ModifierKeys::getCurrentModifiers();
            const bool toggleFav = mods.isCommandDown() || mods.isCtrlDown();

            if (toggleFav)
            {
                const auto name = presetManager.getAllPresets()
                                      .getReference (presetIdx).name;
                presetManager.toggleFavorite (name);
                updatePresetDisplay();

                juce::MessageManager::callAsync ([this]() { showPresetMenu(); });
                return;
            }

            presetManager.loadByIndex (presetIdx);
            updatePresetDisplay();
        });
}

void PPGWave3Editor::onToggleFavorite()
{
    const auto name = presetManager.getCurrentName();
    if (name.isEmpty() || name == "-") return;

    presetManager.toggleFavorite (name);

    const bool isFav = presetManager.isFavorite (name);
    favoriteBtn.setToggleState (isFav, juce::dontSendNotification);
    favoriteBtn.setButtonText (isFav ? juce::String::fromUTF8 ("\xe2\x98\x85")
                                     : juce::String ("*"));
}

void PPGWave3Editor::onToggleFavoritesOnly()
{
    const bool newState = ! presetManager.isFavoritesOnly();
    presetManager.setFavoritesOnly (newState);
    favoritesOnlyBtn.setToggleState (newState, juce::dontSendNotification);
}

// ==================== FX/Env Tab visibility ====================

void PPGWave3Editor::updateFxVisibility()
{
    // Todo visible siempre.
}

void PPGWave3Editor::updateEnvVisibility()
{
    const bool e1 = (activeEnvTab == 0);
    const bool e2 = (activeEnvTab == 1);
    const bool e3 = (activeEnvTab == 2);

    env1Display.setVisible (e1);
    env1A.setVisible (e1); env1D.setVisible (e1);
    env1S.setVisible (e1); env1R.setVisible (e1);

    env2Display.setVisible (e2);
    env2A.setVisible (e2); env2D.setVisible (e2);
    env2S.setVisible (e2); env2R.setVisible (e2);

    env3Display.setVisible (e3);
    env3A.setVisible (e3); env3D.setVisible (e3);
    env3S.setVisible (e3); env3R.setVisible (e3);

    env1TabBtn.setToggleState (e1, juce::dontSendNotification);
    env2TabBtn.setToggleState (e2, juce::dontSendNotification);
    env3TabBtn.setToggleState (e3, juce::dontSendNotification);
}

// ==================== Preset actions ====================

void PPGWave3Editor::updatePresetDisplay()
{
    const int idx = presetManager.getCurrentIndex();
    const auto& list = presetManager.getAllPresets();

    if (idx >= 0 && idx < list.size())
    {
        const auto& info = list.getReference (idx);
        presetDisplay.setInfo (info.name, info.category, info.isFactory);
    }
    else
    {
        presetDisplay.setInfo (presetManager.getCurrentName(),
                               presetManager.getCurrentCategory(), true);
    }

    const auto name = presetManager.getCurrentName();
    const bool isFav = presetManager.isFavorite (name);
    favoriteBtn.setToggleState (isFav, juce::dontSendNotification);
    favoriteBtn.setButtonText (isFav ? juce::String::fromUTF8 ("\xe2\x98\x85")
                                     : juce::String ("*"));

    favoritesOnlyBtn.setToggleState (presetManager.isFavoritesOnly(),
                                     juce::dontSendNotification);
}

void PPGWave3Editor::onPrevPreset()
{
    presetManager.prev();
    updatePresetDisplay();
}

void PPGWave3Editor::onNextPreset()
{
    presetManager.next();
    updatePresetDisplay();
}

void PPGWave3Editor::onLoadPreset()
{
    const auto currentName = presetManager.getCurrentName();

    presetManager.refresh();

    const auto& all = presetManager.getAllPresets();
    for (int i = 0; i < all.size(); ++i)
    {
        if (all.getReference (i).name == currentName)
        {
            presetManager.loadByIndex (i);
            break;
        }
    }

    updatePresetDisplay();
}

void PPGWave3Editor::onSavePreset()
{
    auto* window = new juce::AlertWindow ("Save Preset",
                                          "Enter preset name and category:",
                                          juce::MessageBoxIconType::NoIcon);
    window->addTextEditor ("name", presetManager.getCurrentName(), "Name:");
    window->addComboBox ("cat",
                         { "Bass","Lead","Pad","Keys","Bell","Pluck",
                           "Sequence","FX","Atmospheric","Digital",
                           "Experimental","Percussive" },
                         "Category:");
    window->addButton ("Save",   1, juce::KeyPress (juce::KeyPress::returnKey));
    window->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    window->enterModalState (true,
        juce::ModalCallbackFunction::create ([this, window] (int result)
        {
            if (result == 1)
            {
                auto name = window->getTextEditorContents ("name");
                auto* cb  = window->getComboBoxComponent ("cat");
                auto cat  = cb != nullptr ? cb->getText() : juce::String ("User");

                if (name.isNotEmpty())
                {
                    presetManager.saveUserPreset (name, cat);
                    updatePresetDisplay();
                }
            }
        }),
        true);
}

void PPGWave3Editor::onBrowsePreset()
{
    fileChooser = std::make_unique<juce::FileChooser> (
        "Load Preset",
        presetManager.getUserPresetDirectory(),
        "*.json");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                presetManager.loadFromFile (file);
                updatePresetDisplay();
            }
        });
}

// ==================== Scale ====================

float PPGWave3Editor::computeScale() const
{
    const float refH = 920.0f;
    return juce::jlimit (0.72f, 1.5f, (float) getHeight() / refH);
}

void PPGWave3Editor::applyScaleToAll (float scaleValue)
{
    for (auto* k : { &osc1Pos, &osc1Oct, &osc1Semi, &osc1Fine, &osc1Level,
                     &osc2Pos, &osc2Oct, &osc2Semi, &osc2Fine, &osc2Level,
                     &filterCutoff, &filterReso, &filterEnvAmt, &filterKeyTrack,
                     &env1A, &env1D, &env1S, &env1R,
                     &env2A, &env2D, &env2S, &env2R,
                     &env3A, &env3D, &env3S, &env3R,
                     &master,
                     &lfo1Rate, &lfo1Depth, &lfo1Phase,
                     &lfo2Rate, &lfo2Depth, &lfo2Phase,
                     &driveAmount, &driveTone, &driveMix,
                     &chorusRate, &chorusDepth, &chorusMix,
                     &delayTime, &delayFeedback, &delayMix,
                     &reverbSize, &reverbDamp, &reverbMix,
                     &phaserRate, &phaserDepth, &phaserFeedback, &phaserMix,
                     &vintageAmount, &vintageBits, &vintageSr,
                     &vintageNoise, &vintageDrift, &vintageVar,
                     &compThreshold, &compRatio, &compAttack, &compRelease,
                     &compKnee, &compMakeup, &compScAmount })
        k->setScale (scaleValue);

    eqFreqKnob.setScale (scaleValue);
    eqQKnob   .setScale (scaleValue);
    eqGainKnob.setScale (scaleValue);

    for (auto* d : { &osc1Info, &osc2Info, &filterInfo,
                     &envInfo, &masterInfo,
                     &lfoInfo, &modInfo,
                     &fxInfoDrive, &fxInfoChorus, &fxInfoPhaser, &fxInfoDelay,
                     &fxInfoReverb, &fxInfoVintage, &fxInfoEq, &fxInfoComp })
        d->setScale (scaleValue);

    for (auto* h : { &mod1Amt, &mod2Amt, &mod3Amt, &mod4Amt })
        h->setScale (scaleValue);

    osc1Wave  ->setScale (scaleValue);
    osc2Wave  ->setScale (scaleValue);
    filterType->setScale (scaleValue);

    lfo1Wave->setScale (scaleValue);  lfo2Wave->setScale (scaleValue);
    lfo1Sync->setScale (scaleValue);  lfo2Sync->setScale (scaleValue);
    delaySync->setScale (scaleValue);

    mod1Src->setScale (scaleValue);  mod1Dst->setScale (scaleValue);
    mod2Src->setScale (scaleValue);  mod2Dst->setScale (scaleValue);
    mod3Src->setScale (scaleValue);  mod3Dst->setScale (scaleValue);
    mod4Src->setScale (scaleValue);  mod4Dst->setScale (scaleValue);

    eqHpOn->setScale (scaleValue);
    eqLpOn->setScale (scaleValue);
    compSidechain->setScale (scaleValue);

    pitchWheelLabel.setFont (juce::FontOptions (juce::jmax (7.0f, 9.0f * scaleValue),
                                                juce::Font::bold));
    modWheelLabel  .setFont (juce::FontOptions (juce::jmax (7.0f, 9.0f * scaleValue),
                                                juce::Font::bold));
}

// ==================== reset secuenciador ====================

void PPGWave3Editor::resetSequencer()
{
    for (int i = 0; i < StepSequencer::numSteps; ++i)
    {
        processorRef.sequencer.steps[i].active.store   (false);
        processorRef.sequencer.steps[i].pitch.store    (0);
        processorRef.sequencer.steps[i].velocity.store (0.8f);
    }

    for (auto* sc : seqSteps)
        sc->refreshFromModel();
}
// ==================== paint ====================

void PPGWave3Editor::paint (juce::Graphics& g)
{
    g.fillAll (PPGLookAndFeel::bgApp());

    auto top = getLocalBounds().removeFromTop (42);
    {
        juce::ColourGradient grad (juce::Colour (0xff0a0a0a),
                                   0.0f, (float) top.getY(),
                                   juce::Colour (0xff1a1a1a),
                                   0.0f, (float) top.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRect (top);

        g.setColour (PPGLookAndFeel::accent());
        g.fillRect (top.getX(), top.getBottom() - 1, top.getWidth(), 1);
    }

    drawLogo (g, headerLogoArea);

    drawSection (g, osc1Area,   "OSC 1");
    drawSection (g, osc2Area,   "OSC 2");
    drawSection (g, filterArea, "FILTER");
    drawSection (g, envArea,    "ENVELOPES");
    drawSection (g, lfoArea,    "LFO");
    drawSection (g, modArea,    "MOD MATRIX");

    // ============ STEP SEQUENCER ============
    if (! seqReservedArea.isEmpty())
    {
        const auto r = seqReservedArea.toFloat();

        g.setColour (juce::Colour (0xff141414));
        g.fillRoundedRectangle (r, 4.0f);

        g.setColour (juce::Colour (0xff2f2f2f));
        g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.0f);

        // Título de la sección
        g.setColour (PPGLookAndFeel::accent());
        g.setFont (juce::Font (juce::FontOptions (9.5f * currentScale, juce::Font::bold)));
        g.drawText ("STEP SEQUENCER",
                    seqReservedArea.getX() + 10, seqReservedArea.getY() + 3,
                    220, 14, juce::Justification::centredLeft);

        g.setColour (PPGLookAndFeel::accent().withAlpha (0.35f));
        g.fillRect (seqReservedArea.getX() + 10, seqReservedArea.getY() + 16, 200, 1);
    }

    // Columnas de efectos
    for (int i = 0; i < 8; ++i)
        drawBox (g, fxColumnAreas[i]);

    // Teclado
    if (! keyboardArea.isEmpty())
    {
        const auto r = keyboardArea.toFloat();
        juce::ColourGradient grad (juce::Colour (0xff1c1c1c), r.getX(), r.getY(),
                                   juce::Colour (0xff151515), r.getX(), r.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (r, 4.0f);

        g.setColour (juce::Colour (0xff2f2f2f));
        g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.0f);
    }
}

void PPGWave3Editor::drawSection (juce::Graphics& g, juce::Rectangle<int> area,
                                  const juce::String& title) const
{
    if (area.isEmpty()) return;

    const auto r = area.toFloat();
    juce::ColourGradient grad (juce::Colour (0xff1c1c1c), r.getX(), r.getY(),
                               juce::Colour (0xff151515), r.getX(), r.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (r, 4.0f);

    g.setColour (juce::Colour (0xff2f2f2f));
    g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.0f);

    g.setColour (PPGLookAndFeel::accent());
    g.setFont (juce::Font (juce::FontOptions (9.5f * currentScale, juce::Font::bold)));

    const int titleW = juce::jmin (area.getWidth() - 16, 240);
    const int titleY = area.getY() + 3;
    g.drawText (title.toUpperCase(), area.getX() + 8, titleY, titleW, 12,
                juce::Justification::centredLeft);

    g.setColour (PPGLookAndFeel::accent().withAlpha (0.35f));
    g.fillRect (area.getX() + 8, titleY + 13,
                juce::jmin (titleW, 200), 1);
}

void PPGWave3Editor::drawBox (juce::Graphics& g, juce::Rectangle<int> area) const
{
    if (area.isEmpty()) return;

    const auto r = area.toFloat();
    juce::ColourGradient grad (juce::Colour (0xff1c1c1c), r.getX(), r.getY(),
                               juce::Colour (0xff151515), r.getX(), r.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (r, 4.0f);

    g.setColour (juce::Colour (0xff2f2f2f));
    g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.0f);
}

void PPGWave3Editor::drawLogo (juce::Graphics& g, juce::Rectangle<int> area) const
{
    if (area.isEmpty()) return;

    const float omlSize = juce::jmax (18.0f, (float) area.getHeight() * 0.95f);
    g.setFont (juce::Font (juce::FontOptions (omlSize, juce::Font::bold)));
    g.setColour (PPGLookAndFeel::accent());
    g.drawText ("OML", area, juce::Justification::centredLeft, false);

    const int barX = area.getRight() - 2;
    const int barY = area.getY() + 4;
    const int barH = area.getHeight() - 8;
    g.setColour (PPGLookAndFeel::accent().withAlpha (0.35f));
    g.fillRect (barX, barY, 1, barH);
}

// ==================== Timer ====================

void PPGWave3Editor::timerCallback()
{
    if (! processorRef.sequencer.enabled.load())
    {
        for (auto* sc : seqSteps)
            sc->setPlayingStep (false);
        return;
    }

    const int cur = processorRef.sequencer.currentStep.load();
    for (int i = 0; i < seqSteps.size(); ++i)
        seqSteps[i]->setPlayingStep (i == cur);
}

// ==================== Helpers de layout ====================

void PPGWave3Editor::layoutKnobStack (juce::Rectangle<int> col,
                                      std::initializer_list<juce::Component*> knobs,
                                      int itemHeight)
{
    const int n = (int) knobs.size();
    if (n == 0) return;

    juce::Component** arr = const_cast<juce::Component**> (knobs.begin());

    int usedH = n * itemHeight;
    if (usedH > col.getHeight())
        itemHeight = col.getHeight() / n;
    usedH = n * itemHeight;

    const int leftover = col.getHeight() - usedH;
    col.removeFromTop (leftover / 2);

    for (int i = 0; i < n; ++i)
        arr[i]->setBounds (col.removeFromTop (itemHeight).reduced (2, 0));
}

void PPGWave3Editor::layoutKnobStackBottom (juce::Rectangle<int> col,
                                            std::initializer_list<juce::Component*> knobs,
                                            int itemHeight)
{
    const int n = (int) knobs.size();
    if (n == 0) return;

    juce::Component** arr = const_cast<juce::Component**> (knobs.begin());

    int usedH = n * itemHeight;
    if (usedH > col.getHeight())
        itemHeight = col.getHeight() / n;
    usedH = n * itemHeight;

    col.removeFromTop (col.getHeight() - usedH);

    for (int i = 0; i < n; ++i)
        arr[i]->setBounds (col.removeFromTop (itemHeight).reduced (2, 0));
}

// ==================== resized ====================

void PPGWave3Editor::resized()
{
    currentScale = computeScale();
    applyScaleToAll (currentScale);

    auto r = getLocalBounds();

    // ===== Header =====
    auto header = r.removeFromTop (42);
    {
        auto h = header.reduced (8, 4);

        auto masterZone = h.removeFromRight (150);
        h.removeFromRight (6);

        auto logoZone = h.removeFromLeft (100);
        headerLogoArea = logoZone;
        h.removeFromLeft (6);

        auto presetZone = h;

        const int innerH = masterZone.getHeight();
        auto knobArea = masterZone.removeFromLeft (38);
        master.setBounds (knobArea.withSizeKeepingCentre (36, juce::jmin (innerH, 36)));
        masterZone.removeFromLeft (5);

        const int meterH = (innerH - 3) / 2;
        compGrMeter.setBounds (masterZone.removeFromTop (meterH));
        masterZone.removeFromTop (3);
        masterHztMeter.setBounds (masterZone);

        const int navW   = 24;
        const int smallW = 40;
        const int medW   = 40;
        const int starW  = 26;
        const int favW   = 40;

        prevBtn.setBounds (presetZone.removeFromLeft (navW));
        nextBtn.setBounds (presetZone.removeFromLeft (navW));
        presetZone.removeFromLeft (6);

        browseBtn.setBounds (presetZone.removeFromRight (medW));
        presetZone.removeFromRight (4);
        saveBtn  .setBounds (presetZone.removeFromRight (smallW));
        presetZone.removeFromRight (4);
        loadBtn  .setBounds (presetZone.removeFromRight (smallW));
        presetZone.removeFromRight (8);
        favoritesOnlyBtn.setBounds (presetZone.removeFromRight (favW));
        presetZone.removeFromRight (2);
        favoriteBtn.setBounds (presetZone.removeFromRight (starW));
        presetZone.removeFromRight (8);

        presetDisplay.setBounds (presetZone);
    }

    // ===== Keyboard strip =====
    auto keyboardStrip = r.removeFromBottom (108);
    keyboardArea = keyboardStrip;

    keyboardStrip.reduce (10, 8);

    const int wheelW = juce::jmax (28, juce::roundToInt (42.0f * currentScale));
    const int labelH = juce::jmax (10, juce::roundToInt (12.0f * currentScale));

    auto pitchArea = keyboardStrip.removeFromLeft (wheelW);
    pitchWheelLabel.setBounds (pitchArea.removeFromTop (labelH));
    pitchWheelSlider.setBounds (pitchArea.reduced (3, 0));

    keyboardStrip.removeFromLeft (6);

    auto modArea2 = keyboardStrip.removeFromLeft (wheelW);
    modWheelLabel.setBounds (modArea2.removeFromTop (labelH));
    modWheelSlider.setBounds (modArea2.reduced (3, 0));

    keyboardStrip.removeFromLeft (12);
    keyboardComponent.setBounds (keyboardStrip);

    {
        const float kw = (float) keyboardStrip.getWidth() / 50.0f;
        keyboardComponent.setKeyWidth (juce::jmax (12.0f, kw));
    }

    // ===== Contenido =====
    r.reduce (6, 6);

    const int gap = 6;
    const int availH = r.getHeight() - 2 * gap;

    const int topRowH = (int) ((float) availH * 0.32f);
    const int seqRowH = (int) ((float) availH * 0.36f);
    const int fxRowH  = availH - topRowH - seqRowH;

    auto topRow = r.removeFromTop (topRowH);
    r.removeFromTop (gap);
    auto seqRow = r.removeFromTop (seqRowH);
    r.removeFromTop (gap);
    auto fxRow  = r.removeFromTop (fxRowH);

    seqReservedArea = seqRow;
    fxArea = fxRow;

    // ===== FASE 10: layout del secuenciador =====
    if (seqSteps.size() == StepSequencer::numSteps)
    {
        auto seqArea = seqReservedArea.reduced (10, 22);

        // Columna izquierda: controles globales (~200px)
        auto controlsCol = seqArea.removeFromLeft (200);
        controlsCol.removeFromRight (10);

        auto row1 = controlsCol.removeFromTop (26);
        seqOnOffBtn.setBounds (row1.removeFromLeft (100));
        row1.removeFromLeft (8);
        seqResetBtn.setBounds (row1);
        controlsCol.removeFromTop (6);

        // RATE
        controlsCol.removeFromTop (8);
        seqRateCombo.setBounds (controlsCol.removeFromTop (24));
        controlsCol.removeFromTop (6);

        // DIRECTION
        controlsCol.removeFromTop (8);
        seqDirCombo.setBounds (controlsCol.removeFromTop (24));
        controlsCol.removeFromTop (8);

        // SWING
        controlsCol.removeFromTop (10);
        seqSwingSlider.setBounds (controlsCol.removeFromTop (22));
        controlsCol.removeFromTop (6);

        // LENGTH
        controlsCol.removeFromTop (10);
        seqLengthSlider.setBounds (controlsCol.removeFromTop (22));
        controlsCol.removeFromTop (6);

        // BASE NOTE
        controlsCol.removeFromTop (10);
        seqBaseNoteSlider.setBounds (controlsCol.removeFromTop (22));

        // 16 pasos en el resto del ancho
        const int stepW = seqArea.getWidth() / StepSequencer::numSteps;
        const int stepGap = 2;

        for (int i = 0; i < StepSequencer::numSteps; ++i)
        {
            const int x = seqArea.getX() + i * stepW;
            seqSteps[i]->setBounds (x, seqArea.getY(),
                                    stepW - stepGap, seqArea.getHeight());
        }
    }

    // Fila superior: 6 columnas
    {
        const int colGap = 6;
        const int usableW = topRow.getWidth() - 5 * colGap;

        const int osc1W  = (int) ((float) usableW * 0.15f);
        const int osc2W  = (int) ((float) usableW * 0.15f);
        const int filtW  = (int) ((float) usableW * 0.14f);
        const int envW   = (int) ((float) usableW * 0.19f);
        const int lfoW   = (int) ((float) usableW * 0.18f);

        osc1Area   = topRow.removeFromLeft (osc1W);   topRow.removeFromLeft (colGap);
        osc2Area   = topRow.removeFromLeft (osc2W);   topRow.removeFromLeft (colGap);
        filterArea = topRow.removeFromLeft (filtW);   topRow.removeFromLeft (colGap);
        envArea    = topRow.removeFromLeft (envW);    topRow.removeFromLeft (colGap);
        lfoArea    = topRow.removeFromLeft (lfoW);    topRow.removeFromLeft (colGap);
        modArea    = topRow;
    }

    auto infoWidthFor = [] (int sectionW)
    {
        return juce::jlimit (60, 200, (int) ((float) sectionW * 0.30f));
    };

    auto titleRowFor = [&] (juce::Rectangle<int> area, InfoDisplay& info,
                            juce::Rectangle<int>& innerOut)
    {
        innerOut = area.reduced (6);
        auto titleRow = innerOut.removeFromTop (
            juce::jmax (14, juce::roundToInt (15.0f * currentScale)));
        info.setBounds (titleRow.removeFromRight (
            infoWidthFor (area.getWidth())).reduced (0, 1));
        innerOut.removeFromTop (1);
    };

    // -------- OSC --------
    auto layoutOscSection = [&] (juce::Rectangle<int> area,
                                 InfoDisplay& info, ButtonSelector& waveSel,
                                 ui::WavetablePreview& preview,
                                 RotaryKnob& kPos, RotaryKnob& kOct, RotaryKnob& kSemi,
                                 RotaryKnob& kFine, RotaryKnob& kLevel)
    {
        juce::Rectangle<int> inner;
        titleRowFor (area, info, inner);

        waveSel.setBounds (inner.removeFromTop (20).reduced (0, 1));
        inner.removeFromTop (3);

        preview.setBounds (inner.removeFromTop (60).reduced (1, 0));
        inner.removeFromTop (4);

        auto topKnobRow = inner.removeFromTop (62);
        {
            const int kw = 48;
            const int spacing = (topKnobRow.getWidth() - 3 * kw) / 4;
            topKnobRow.removeFromLeft (spacing);
            kPos .setBounds (topKnobRow.removeFromLeft (kw));
            topKnobRow.removeFromLeft (spacing);
            kOct .setBounds (topKnobRow.removeFromLeft (kw));
            topKnobRow.removeFromLeft (spacing);
            kSemi.setBounds (topKnobRow.removeFromLeft (kw));
        }
        inner.removeFromTop (6);

        const int botH = juce::jmin (90, inner.getHeight());
        auto botKnobRow = inner.removeFromTop (botH);
        {
            const int kw = 57;
            const int spacing = (botKnobRow.getWidth() - 2 * kw) / 3;
            botKnobRow.removeFromLeft (spacing);
            kFine .setBounds (botKnobRow.removeFromLeft (kw));
            botKnobRow.removeFromLeft (spacing);
            kLevel.setBounds (botKnobRow.removeFromLeft (kw));
        }
    };

    layoutOscSection (osc1Area, osc1Info, *osc1Wave, osc1Preview,
                      osc1Pos, osc1Oct, osc1Semi, osc1Fine, osc1Level);
    layoutOscSection (osc2Area, osc2Info, *osc2Wave, osc2Preview,
                      osc2Pos, osc2Oct, osc2Semi, osc2Fine, osc2Level);

    // -------- FILTER --------
    {
        juce::Rectangle<int> inner;
        titleRowFor (filterArea, filterInfo, inner);

        filterType->setBounds (inner.removeFromTop (20).reduced (0, 1));
        inner.removeFromTop (4);

        const int rowH = juce::jmin (90, inner.getHeight() / 2);

        auto topKnobRow = inner.removeFromTop (rowH);
        {
            const int kw = 57;
            const int spacing = (topKnobRow.getWidth() - 2 * kw) / 3;
            topKnobRow.removeFromLeft (spacing);
            filterCutoff.setBounds (topKnobRow.removeFromLeft (kw));
            topKnobRow.removeFromLeft (spacing);
            filterReso  .setBounds (topKnobRow.removeFromLeft (kw));
        }
        inner.removeFromTop (4);
        auto botKnobRow = inner.removeFromTop (rowH);
        {
            const int kw = 57;
            const int spacing = (botKnobRow.getWidth() - 2 * kw) / 3;
            botKnobRow.removeFromLeft (spacing);
            filterEnvAmt.setBounds (botKnobRow.removeFromLeft (kw));
            botKnobRow.removeFromLeft (spacing);
            filterKeyTrack.setBounds (botKnobRow.removeFromLeft (kw));
        }
    }

    // -------- ENVELOPES --------
    {
        juce::Rectangle<int> inner;
        titleRowFor (envArea, envInfo, inner);

        auto tabRow = inner.removeFromTop (20);
        const int tabW = tabRow.getWidth() / 3;
        env1TabBtn.setBounds (tabRow.removeFromLeft (tabW).reduced (0, 1));
        env2TabBtn.setBounds (tabRow.removeFromLeft (tabW).reduced (0, 1));
        env3TabBtn.setBounds (tabRow.reduced (0, 1));
        inner.removeFromTop (3);

        auto knobRow = inner.removeFromBottom (62);
        {
            const int kw = 48;
            const int spacing = (knobRow.getWidth() - 4 * kw) / 5;
            knobRow.removeFromLeft (spacing);
            env1A.setBounds (knobRow.removeFromLeft (kw));
            knobRow.removeFromLeft (spacing);
            env1D.setBounds (knobRow.removeFromLeft (kw));
            knobRow.removeFromLeft (spacing);
            env1S.setBounds (knobRow.removeFromLeft (kw));
            knobRow.removeFromLeft (spacing);
            env1R.setBounds (knobRow.removeFromLeft (kw));
        }
        env2A.setBounds (env1A.getBounds());
        env2D.setBounds (env1D.getBounds());
        env2S.setBounds (env1S.getBounds());
        env2R.setBounds (env1R.getBounds());
        env3A.setBounds (env1A.getBounds());
        env3D.setBounds (env1D.getBounds());
        env3S.setBounds (env1S.getBounds());
        env3R.setBounds (env1R.getBounds());

        inner.removeFromBottom (4);

        auto dispRow = inner.removeFromTop (juce::jmin (120, inner.getHeight()));
        env1Display.setBounds (dispRow);
        env2Display.setBounds (dispRow);
        env3Display.setBounds (dispRow);
    }

    // -------- LFO --------
    {
        juce::Rectangle<int> inner;
        titleRowFor (lfoArea, lfoInfo, inner);

        const int halfH = (inner.getHeight() - 4) / 2;
        auto lfo1Zone = inner.removeFromTop (halfH);
        inner.removeFromTop (4);
        auto lfo2Zone = inner;

        auto layoutLFO = [&] (juce::Rectangle<int> area,
                              ComboBoxSelector& w, ComboBoxSelector& sync,
                              ui::LFODisplay& display,
                              RotaryKnob& rate, RotaryKnob& depth, RotaryKnob& phase)
        {
            auto comboRow = area.removeFromTop (20);
            const int comboW = comboRow.getWidth() / 2;
            w   .setBounds (comboRow.removeFromLeft (comboW).reduced (1, 0));
            sync.setBounds (comboRow.reduced (1, 0));
            area.removeFromTop (3);

            display.setBounds (area.removeFromTop (28).reduced (1, 0));
            area.removeFromTop (4);

            auto knobRow = area.removeFromTop (juce::jmin (62, area.getHeight()));
            const int kw = 48;
            const int spacing = (knobRow.getWidth() - 3 * kw) / 4;
            knobRow.removeFromLeft (spacing);
            rate .setBounds (knobRow.removeFromLeft (kw));
            knobRow.removeFromLeft (spacing);
            depth.setBounds (knobRow.removeFromLeft (kw));
            knobRow.removeFromLeft (spacing);
            phase.setBounds (knobRow.removeFromLeft (kw));
        };

        layoutLFO (lfo1Zone, *lfo1Wave, *lfo1Sync, lfo1Display,
                   lfo1Rate, lfo1Depth, lfo1Phase);
        layoutLFO (lfo2Zone, *lfo2Wave, *lfo2Sync, lfo2Display,
                   lfo2Rate, lfo2Depth, lfo2Phase);
    }

    // -------- MOD MATRIX --------
    {
        juce::Rectangle<int> inner;
        titleRowFor (modArea, modInfo, inner);

        const int rowH = 50;

        auto layoutRow = [&] (juce::Rectangle<int> row,
                              ComboBoxSelector& src, ComboBoxSelector& dst, HSlider& amt)
        {
            const int srcW = (int) ((float) row.getWidth() * 0.30f);
            const int dstW = (int) ((float) row.getWidth() * 0.30f);
            src.setBounds (row.removeFromLeft (srcW).reduced (1, 3));
            dst.setBounds (row.removeFromLeft (dstW).reduced (1, 3));

            // Alinear el fader verticalmente con la caja del ComboBox
            // (saltar la zona del label "SRC"/"DST" que hay encima)
            const int labelH = juce::jmax (9, juce::roundToInt (11.0f * currentScale));
            auto amtArea = row.reduced (1, 3);
            amtArea.removeFromTop (labelH);
            amt.setBounds (amtArea);
        };

        layoutRow (inner.removeFromTop (rowH), *mod1Src, *mod1Dst, mod1Amt);
        layoutRow (inner.removeFromTop (rowH), *mod2Src, *mod2Dst, mod2Amt);
        layoutRow (inner.removeFromTop (rowH), *mod3Src, *mod3Dst, mod3Amt);
        layoutRow (inner.removeFromTop (rowH), *mod4Src, *mod4Dst, mod4Amt);
    }

    // ==================== EFFECTS ====================
    {
        juce::Rectangle<int> inner = fxArea.reduced (6, 4);

        const int colGap = 6;
        const int numCols = 8;

        const float units[8] = { 1.0f, 1.0f, 1.2f, 1.2f, 1.0f, 1.8f, 2.5f, 1.8f };
        float totalUnits = 0.0f;
        for (float u : units) totalUnits += u;

        const int usableW = inner.getWidth() - (numCols - 1) * colGap;
        const float unitPx = (float) usableW / totalUnits;

        int x = inner.getX();
        const int y = inner.getY();
        const int colH = inner.getHeight();
        for (int i = 0; i < 8; ++i)
        {
            const int w = (int) (units[i] * unitPx);
            fxColumnAreas[i] = { x, y, w, colH };
            x += w + colGap;
        }

        const int headerH = 40;
        const int toggleRowH = 20;

        auto layoutCol = [&] (int idx, FxTab& tab, InfoDisplay& info,
                              std::initializer_list<juce::Component*> knobs)
        {
            auto col = fxColumnAreas[idx].reduced (6, 6);
            const int colW = col.getWidth();

            auto headerArea = col.removeFromTop (headerH);
            tab.setBounds (headerArea);

            auto infoRow = headerArea.withHeight (toggleRowH);
            auto infoArea = infoRow.removeFromRight (colW / 2);
            info.setBounds (infoArea.reduced (2, 3));

            col.removeFromTop (6);

            const int n = (int) knobs.size();
            if (n == 0) return;

            juce::Component** arr = const_cast<juce::Component**> (knobs.begin());
            const int h = col.getHeight() / n;
            for (int i = 0; i < n; ++i)
                arr[i]->setBounds (col.removeFromTop (h).reduced (2, 2));
        };

        // ---- DIST ----
        layoutCol (0, *driveTab, fxInfoDrive,
                   { &driveAmount, &driveTone, &driveMix });

        // ---- CHORUS ----
        layoutCol (1, *chorusTab, fxInfoChorus,
                   { &chorusRate, &chorusDepth, &chorusMix });

        // ---- PHASER (2x2) ----
        {
            auto col = fxColumnAreas[2].reduced (6, 6);
            const int colW = col.getWidth();

            auto headerArea = col.removeFromTop (headerH);
            phaserTab->setBounds (headerArea);

            auto infoRow = headerArea.withHeight (toggleRowH);
            auto infoArea = infoRow.removeFromRight (colW / 2);
            fxInfoPhaser.setBounds (infoArea.reduced (2, 3));

            col.removeFromTop (6);

            const int halfH = col.getHeight() / 2;
            auto row1 = col.removeFromTop (halfH);
            auto row2 = col;

            const int halfW1 = row1.getWidth() / 2;
            phaserRate .setBounds (row1.removeFromLeft (halfW1).reduced (2, 2));
            phaserDepth.setBounds (row1.reduced (2, 2));

            const int halfW2 = row2.getWidth() / 2;
            phaserFeedback.setBounds (row2.removeFromLeft (halfW2).reduced (2, 2));
            phaserMix     .setBounds (row2.reduced (2, 2));
        }

        // ---- DELAY ----
        {
            auto col = fxColumnAreas[3].reduced (6, 6);
            const int colW = col.getWidth();

            auto headerArea = col.removeFromTop (headerH);
            delayTab->setBounds (headerArea);

            auto infoRow = headerArea.withHeight (toggleRowH);
            auto infoArea = infoRow.removeFromRight (colW / 2);
            fxInfoDelay.setBounds (infoArea.reduced (2, 3));

            col.removeFromTop (6);

            delaySync->setBounds (col.removeFromTop (26));
            col.removeFromTop (6);

            juce::Component* arr[3] = {
                (juce::Component*) &delayTime,
                (juce::Component*) &delayFeedback,
                (juce::Component*) &delayMix
            };
            const int n = 3;
            const int h = col.getHeight() / n;
            for (int i = 0; i < n; ++i)
                arr[i]->setBounds (col.removeFromTop (h).reduced (2, 2));
        }

        // ---- REVERB ----
        layoutCol (4, *reverbTab, fxInfoReverb,
                   { &reverbSize, &reverbDamp, &reverbMix });

        // ---- VINTAGE (2 sub-columnas) ----
        {
            auto col = fxColumnAreas[5].reduced (6, 6);
            const int colW = col.getWidth();

            auto headerArea = col.removeFromTop (headerH);
            vintageTab->setBounds (headerArea);

            auto infoRow = headerArea.withHeight (toggleRowH);
            auto infoArea = infoRow.removeFromRight (colW / 2);
            fxInfoVintage.setBounds (infoArea.reduced (2, 3));

            col.removeFromTop (6);

            const int subGap = 4;
            const int subW = (col.getWidth() - subGap) / 2;
            auto leftSub  = col.removeFromLeft (subW);
            col.removeFromLeft (subGap);
            auto rightSub = col;

            juce::Component* arrL[3] = {
                (juce::Component*) &vintageAmount,
                (juce::Component*) &vintageBits,
                (juce::Component*) &vintageSr
            };
            juce::Component* arrR[3] = {
                (juce::Component*) &vintageNoise,
                (juce::Component*) &vintageDrift,
                (juce::Component*) &vintageVar
            };

            const int n = 3;
            const int hL = leftSub.getHeight() / n;
            const int hR = rightSub.getHeight() / n;
            for (int i = 0; i < n; ++i)
            {
                arrL[i]->setBounds (leftSub.removeFromTop (hL).reduced (1, 2));
                arrR[i]->setBounds (rightSub.removeFromTop (hR).reduced (1, 2));
            }
        }

        // ---- EQ ----
        {
            auto col = fxColumnAreas[6].reduced (6, 6);
            const int colW = col.getWidth();

            auto headerArea = col.removeFromTop (headerH);
            eqTab->setBounds (headerArea);

            auto infoRow = headerArea.withHeight (toggleRowH);
            auto infoArea = infoRow.removeFromRight (colW / 2);
            fxInfoEq.setBounds (infoArea.reduced (2, 3));

            col.removeFromTop (6);

            const int bandRowH  = 20;
            const int finalRowH = 65;
            const int gap1      = 6;
            const int gap2      = 8;

            const int curveH = col.getHeight()
                             - bandRowH - finalRowH - gap1 - gap2;

            eqCurveDisplay.setBounds (col.removeFromTop (juce::jmax (40, curveH))
                                         .reduced (0, 1));
            col.removeFromTop (gap1);

            auto bandRow = col.removeFromTop (bandRowH);
            const int bandW = bandRow.getWidth() / 4;
            eqLowBtn .setBounds (bandRow.removeFromLeft (bandW).reduced (1, 0));
            eqLmidBtn.setBounds (bandRow.removeFromLeft (bandW).reduced (1, 0));
            eqHmidBtn.setBounds (bandRow.removeFromLeft (bandW).reduced (1, 0));
            eqHighBtn.setBounds (bandRow.reduced (1, 0));
            col.removeFromTop (gap2);

            const int totalW = col.getWidth();
            const int elemGap = 4;
            const int elemW = (totalW - 4 * elemGap) / 5;

            eqHpOn->setBounds (col.removeFromLeft (elemW)
                                  .withSizeKeepingCentre (elemW - 4, 20));
            col.removeFromLeft (elemGap);
            eqFreqKnob.setBounds (col.removeFromLeft (elemW));
            col.removeFromLeft (elemGap);
            eqQKnob.setBounds (col.removeFromLeft (elemW));
            col.removeFromLeft (elemGap);
            eqGainKnob.setBounds (col.removeFromLeft (elemW));
            col.removeFromLeft (elemGap);
            eqLpOn->setBounds (col.removeFromLeft (elemW)
                                  .withSizeKeepingCentre (elemW - 4, 20));
        }

        // ---- COMP ----
        {
            auto col = fxColumnAreas[7].reduced (6, 6);
            const int colW = col.getWidth();

            auto headerArea = col.removeFromTop (headerH);
            compTab->setBounds (headerArea);

            auto infoRow = headerArea.withHeight (toggleRowH);
            auto infoArea = infoRow.removeFromRight (colW / 2);
            fxInfoComp.setBounds (infoArea.reduced (2, 3));

            col.removeFromTop (6);

            const int subGap = 4;
            const int subW = (col.getWidth() - subGap) / 2;
            auto leftSub  = col.removeFromLeft (subW);
            col.removeFromLeft (subGap);
            auto rightSub = col;

            juce::Component* arrL[4] = {
                (juce::Component*) &compThreshold,
                (juce::Component*) &compRatio,
                (juce::Component*) &compAttack,
                (juce::Component*) &compRelease
            };
            const int nL = 4;
            const int hL = leftSub.getHeight() / nL;
            for (int i = 0; i < nL; ++i)
                arrL[i]->setBounds (leftSub.removeFromTop (hL).reduced (1, 2));

            auto scArea = rightSub.removeFromBottom (24);
            compSidechain->setBounds (scArea.withSizeKeepingCentre (48, 20));
            rightSub.removeFromBottom (4);

            juce::Component* arrR[3] = {
                (juce::Component*) &compKnee,
                (juce::Component*) &compMakeup,
                (juce::Component*) &compScAmount
            };
            const int nR = 3;
            const int hR = rightSub.getHeight() / nR;
            for (int i = 0; i < nR; ++i)
                arrR[i]->setBounds (rightSub.removeFromTop (hR).reduced (1, 2));
        }
    }

    repaint();
}
