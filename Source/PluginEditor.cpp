#include "PluginEditor.h"
#include "ParameterIDs.h"
#include "UI/PPGLookAndFeel.h"
#include "UI/Visualizers.h"

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
                                  juce::jmax (7.0f, 9.5f * scale), juce::Font::plain));

    if (paramValue.isEmpty())
    {
        g.drawText (paramName, textArea, juce::Justification::centredLeft);
    }
    else
    {
        auto nameArea = textArea.removeFromLeft ((int) ((float) textArea.getWidth() * 0.55f));
        g.drawText (paramName,  nameArea, juce::Justification::centredLeft);
        g.drawText (paramValue, textArea, juce::Justification::centredRight);
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

    // FASE 7: icono ▼ a la derecha indicando que se puede desplegar
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

// FASE 7: click abre el menú de presets.
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

    slider.onValueChange = [this]()
    {
        if (infoDisplay != nullptr)
            infoDisplay->setInfo (paramName, slider.getTextFromValue (slider.getValue()));
    };

    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffcccccc));
    addAndMakeVisible (label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        state, paramID, slider);
}

void PPGWave3Editor::RotaryKnob::setScale (float s)
{
    scale = s;
    label.setFont (juce::FontOptions (juce::jmax (7.0f, 9.0f * scale)));
    resized();
}

void PPGWave3Editor::RotaryKnob::resized()
{
    auto r = getLocalBounds();
    const int labelH = juce::jmax (9, juce::roundToInt (11.0f * scale));
    label.setBounds (r.removeFromTop (labelH));
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
    slider.setBounds (getLocalBounds().reduced (2, 4));
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
    label.setColour (juce::Label::textColourId, juce::Colour (0xffcccccc));
    addAndMakeVisible (label);

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
}

void PPGWave3Editor::EQBandKnob::setScale (float s)
{
    scale = s;
    label.setFont (juce::FontOptions (juce::jmax (7.0f, 9.0f * scale)));
    resized();
}

void PPGWave3Editor::EQBandKnob::resized()
{
    auto r = getLocalBounds();
    const int labelH = juce::jmax (9, juce::roundToInt (11.0f * scale));
    label.setBounds (r.removeFromTop (labelH));
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
    if (infoDisplay == nullptr) return;

    const auto id = ids[activeBand];
    auto* param = apvtsRef.getParameter (id);
    if (param == nullptr) return;

    infoDisplay->setInfo (paramName, param->getCurrentValueAsText());
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

PPGWave3Editor::EQCurveDisplay::EQCurveDisplay (juce::AudioProcessorValueTreeState& apvts)
    : apvtsRef (apvts)
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
    const auto now = readParams();
    if (! hasCached || cacheChanged (now, cached))
    {
        cached = now;
        hasCached = true;
        repaint();
    }
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
}

// ==================== Editor ====================

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
      masterHztMeter (p.peakLevel, "MST", false),
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
      driveOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::driveOn, "DRIVE", &fxInfo)),
      driveAmount (p.apvts, ParamIDs::driveAmount, "AMT",  &fxInfo),
      driveTone   (p.apvts, ParamIDs::driveTone,   "TONE", &fxInfo),
      driveMix    (p.apvts, ParamIDs::driveMix,    "MIX",  &fxInfo),
      chorusOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::chorusOn, "CHORUS", &fxInfo)),
      chorusRate  (p.apvts, ParamIDs::chorusRate,  "RATE",  &fxInfo),
      chorusDepth (p.apvts, ParamIDs::chorusDepth, "DEPTH", &fxInfo),
      chorusMix   (p.apvts, ParamIDs::chorusMix,   "MIX",   &fxInfo),
      delayOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::delayOn, "DELAY", &fxInfo)),
      delaySync (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::delaySync, "SYNC", &fxInfo)),
      delayTime     (p.apvts, ParamIDs::delayTime,     "TIME",  &fxInfo),
      delayFeedback (p.apvts, ParamIDs::delayFeedback, "FEEDBK",&fxInfo),
      delayMix      (p.apvts, ParamIDs::delayMix,      "MIX",   &fxInfo),
      reverbOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::reverbOn, "REVERB", &fxInfo)),
      reverbSize (p.apvts, ParamIDs::reverbSize, "SIZE", &fxInfo),
      reverbDamp (p.apvts, ParamIDs::reverbDamp, "DAMP", &fxInfo),
      reverbMix  (p.apvts, ParamIDs::reverbMix,  "MIX",  &fxInfo),
      eqOn   (std::make_unique<ToggleButton> (p.apvts, ParamIDs::eqOn,   "EQ", &fxInfo)),
      eqHpOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::eqHpOn, "HP", &fxInfo)),
      eqLpOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::eqLpOn, "LP", &fxInfo)),
      eqFreqKnob (p.apvts,
                  { ParamIDs::eqLowFreq, ParamIDs::eqLmidFreq,
                    ParamIDs::eqHmidFreq, ParamIDs::eqHighFreq },
                  "FREQ", &fxInfo),
      eqQKnob    (p.apvts,
                  { ParamIDs::eqLowQ, ParamIDs::eqLmidQ,
                    ParamIDs::eqHmidQ, ParamIDs::eqHighQ },
                  "Q", &fxInfo),
      eqGainKnob (p.apvts,
                  { ParamIDs::eqLowGain, ParamIDs::eqLmidGain,
                    ParamIDs::eqHmidGain, ParamIDs::eqHighGain },
                  "GAIN", &fxInfo),
      eqCurveDisplay (p.apvts),
      phaserOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::phaserOn, "PHASER", &fxInfo)),
      phaserRate     (p.apvts, ParamIDs::phaserRate,     "RATE",  &fxInfo),
      phaserDepth    (p.apvts, ParamIDs::phaserDepth,    "DEPTH", &fxInfo),
      phaserFeedback (p.apvts, ParamIDs::phaserFeedback, "FEEDBK",&fxInfo),
      phaserMix      (p.apvts, ParamIDs::phaserMix,      "MIX",   &fxInfo),
      vintageOn     (std::make_unique<ToggleButton> (p.apvts, ParamIDs::vintageOn, "VINTAGE", &fxInfo)),
      vintageAmount (p.apvts, ParamIDs::vintageAmount, "AMOUNT", &fxInfo),
      vintageBits   (p.apvts, ParamIDs::vintageBits,   "BITS",   &fxInfo),
      vintageSr     (p.apvts, ParamIDs::vintageSr,     "SR",     &fxInfo),
      vintageNoise  (p.apvts, ParamIDs::vintageNoise,  "NOISE",  &fxInfo),
      vintageDrift  (p.apvts, ParamIDs::vintageDrift,  "DRIFT",  &fxInfo),
      vintageVar    (p.apvts, ParamIDs::vintageVar,    "VAR",    &fxInfo),
      compOn        (std::make_unique<ToggleButton> (p.apvts, ParamIDs::compOn, "COMP", &fxInfo)),
      compSidechain (std::make_unique<ToggleButton> (p.apvts, ParamIDs::compSidechain, "SC", &fxInfo)),
      compThreshold (p.apvts, ParamIDs::compThreshold, "THRSH",  &fxInfo),
      compRatio     (p.apvts, ParamIDs::compRatio,     "RATIO",  &fxInfo),
      compAttack    (p.apvts, ParamIDs::compAttack,    "ATTACK", &fxInfo),
      compRelease   (p.apvts, ParamIDs::compRelease,   "RELSE",  &fxInfo),
      compKnee      (p.apvts, ParamIDs::compKnee,      "KNEE",   &fxInfo),
      compMakeup    (p.apvts, ParamIDs::compMakeup,    "MAKEUP", &fxInfo),
      compScAmount  (p.apvts, ParamIDs::compScAmount,  "SC AMT", &fxInfo),
      keyboardComponent (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    juce::ignoreUnused (processorRef, apvts);

    // === Preset bar ===
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

    // === FX tab buttons (8) ===
    for (auto* b : { &driveTabBtn, &chorusTabBtn, &phaserTabBtn,
                     &delayTabBtn, &reverbTabBtn, &vintageTabBtn,
                     &eqTabBtn, &compTabBtn })
    {
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff1c1c1c));
        b->setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
        b->setColour (juce::TextButton::textColourOffId,  juce::Colour (0xffaaaaaa));
        b->setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
        addAndMakeVisible (b);
    }

    driveTabBtn  .setButtonText ("DIST");
    chorusTabBtn .setButtonText ("CHORUS");
    phaserTabBtn .setButtonText ("PHASER");
    delayTabBtn  .setButtonText ("DELAY");
    reverbTabBtn .setButtonText ("REVERB");
    vintageTabBtn.setButtonText ("VINTAGE");
    eqTabBtn     .setButtonText ("EQ");
    compTabBtn   .setButtonText ("COMP");

    driveTabBtn  .onClick = [this]() { activeFxTab = 0; updateFxVisibility(); repaint(); };
    chorusTabBtn .onClick = [this]() { activeFxTab = 1; updateFxVisibility(); repaint(); };
    phaserTabBtn .onClick = [this]() { activeFxTab = 2; updateFxVisibility(); repaint(); };
    delayTabBtn  .onClick = [this]() { activeFxTab = 3; updateFxVisibility(); repaint(); };
    reverbTabBtn .onClick = [this]() { activeFxTab = 4; updateFxVisibility(); repaint(); };
    vintageTabBtn.onClick = [this]() { activeFxTab = 5; updateFxVisibility(); repaint(); };
    eqTabBtn     .onClick = [this]() { activeFxTab = 6; updateFxVisibility(); repaint(); };
    compTabBtn   .onClick = [this]() { activeFxTab = 7; updateFxVisibility(); repaint(); };

    for (auto* b : { &env1TabBtn, &env2TabBtn, &env3TabBtn })
    {
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff1c1c1c));
        b->setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
        b->setColour (juce::TextButton::textColourOffId,  juce::Colour (0xffaaaaaa));
        b->setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
        addAndMakeVisible (b);
    }

    env1TabBtn.setButtonText ("ENV1 - AMP");
    env2TabBtn.setButtonText ("ENV2 - FILTER");
    env3TabBtn.setButtonText ("ENV3 - FREE");

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
    eqLmidBtn.setButtonText ("LOW MID");
    eqHmidBtn.setButtonText ("HI MID");
    eqHighBtn.setButtonText ("HIGH");

    eqLowBtn .onClick = [this]() { setActiveEqBand (0); };
    eqLmidBtn.onClick = [this]() { setActiveEqBand (1); };
    eqHmidBtn.onClick = [this]() { setActiveEqBand (2); };
    eqHighBtn.onClick = [this]() { setActiveEqBand (3); };

    setActiveEqBand (0);
    addAndMakeVisible (eqCurveDisplay);

    // === Teclado virtual ===
    keyboardComponent.setAvailableRange (36, 96);
    keyboardComponent.setLowestVisibleKey (36);
    keyboardComponent.setKeyWidth (20.0f);
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

    // === Pitch wheel ===
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

    // === Mod wheel ===
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

    // === Visualizadores ===
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
    addAndMakeVisible (*driveOn);
    addAndMakeVisible (*chorusOn);
    addAndMakeVisible (*delayOn);    addAndMakeVisible (*delaySync);
    addAndMakeVisible (*reverbOn);
    addAndMakeVisible (*eqOn);
    addAndMakeVisible (*eqHpOn);
    addAndMakeVisible (*eqLpOn);
    addAndMakeVisible (*phaserOn);
    addAndMakeVisible (*vintageOn);
    addAndMakeVisible (*compOn);
    addAndMakeVisible (*compSidechain);

    addAndMakeVisible (eqFreqKnob);
    addAndMakeVisible (eqQKnob);
    addAndMakeVisible (eqGainKnob);

    for (auto* d : { &osc1Info, &osc2Info, &filterInfo,
                     &envInfo, &masterInfo,
                     &lfoInfo, &modInfo, &fxInfo })
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
    setSize (1280, 1040);

    updateFxVisibility();
    updateEnvVisibility();
}

PPGWave3Editor::~PPGWave3Editor()
{
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

// ==================== Menú de presets ====================

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

// ==================== Botones ★ y FAV ====================

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

// ==================== FX Tab visibility ====================

void PPGWave3Editor::updateFxVisibility()
{
    // 0=DIST, 1=CHORUS, 2=PHASER, 3=DELAY, 4=REVERB,
    // 5=VINTAGE, 6=EQ, 7=COMP
    const bool d  = (activeFxTab == 0);
    const bool c  = (activeFxTab == 1);
    const bool ph = (activeFxTab == 2);
    const bool dl = (activeFxTab == 3);
    const bool r  = (activeFxTab == 4);
    const bool vi = (activeFxTab == 5);
    const bool eq = (activeFxTab == 6);
    const bool cp = (activeFxTab == 7);

    driveOn    ->setVisible (d);
    driveAmount.setVisible (d);
    driveTone  .setVisible (d);
    driveMix   .setVisible (d);

    chorusOn   ->setVisible (c);
    chorusRate .setVisible (c);
    chorusDepth.setVisible (c);
    chorusMix  .setVisible (c);

    phaserOn      ->setVisible (ph);
    phaserRate    .setVisible (ph);
    phaserDepth   .setVisible (ph);
    phaserFeedback.setVisible (ph);
    phaserMix     .setVisible (ph);

    delayOn    ->setVisible (dl);
    delaySync  ->setVisible (dl);
    delayTime  .setVisible (dl);
    delayFeedback.setVisible (dl);
    delayMix   .setVisible (dl);

    reverbOn   ->setVisible (r);
    reverbSize .setVisible (r);
    reverbDamp .setVisible (r);
    reverbMix  .setVisible (r);

    vintageOn    ->setVisible (vi);
    vintageAmount.setVisible (vi);
    vintageBits  .setVisible (vi);
    vintageSr    .setVisible (vi);
    vintageNoise .setVisible (vi);
    vintageDrift .setVisible (vi);
    vintageVar   .setVisible (vi);

    eqOn   ->setVisible (eq);
    eqHpOn ->setVisible (eq);
    eqLpOn ->setVisible (eq);
    eqLowBtn .setVisible (eq);
    eqLmidBtn.setVisible (eq);
    eqHmidBtn.setVisible (eq);
    eqHighBtn.setVisible (eq);
    eqFreqKnob.setVisible (eq);
    eqQKnob   .setVisible (eq);
    eqGainKnob.setVisible (eq);
    eqCurveDisplay.setVisible (eq);

    compOn       ->setVisible (cp);
    compSidechain->setVisible (cp);
    compThreshold.setVisible (cp);
    compRatio    .setVisible (cp);
    compAttack   .setVisible (cp);
    compRelease  .setVisible (cp);
    compKnee     .setVisible (cp);
    compMakeup   .setVisible (cp);
    compScAmount .setVisible (cp);

    driveTabBtn  .setToggleState (d,  juce::dontSendNotification);
    chorusTabBtn .setToggleState (c,  juce::dontSendNotification);
    phaserTabBtn .setToggleState (ph, juce::dontSendNotification);
    delayTabBtn  .setToggleState (dl, juce::dontSendNotification);
    reverbTabBtn .setToggleState (r,  juce::dontSendNotification);
    vintageTabBtn.setToggleState (vi, juce::dontSendNotification);
    eqTabBtn     .setToggleState (eq, juce::dontSendNotification);
    compTabBtn   .setToggleState (cp, juce::dontSendNotification);
}

// ==================== Env Tab visibility ====================

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
    const float refH = 1040.0f;
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
                     &lfoInfo, &modInfo, &fxInfo })
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

    driveOn     ->setScale (scaleValue);
    chorusOn    ->setScale (scaleValue);
    delayOn     ->setScale (scaleValue);
    reverbOn    ->setScale (scaleValue);
    eqOn        ->setScale (scaleValue);
    eqHpOn      ->setScale (scaleValue);
    eqLpOn      ->setScale (scaleValue);
    phaserOn    ->setScale (scaleValue);
    vintageOn   ->setScale (scaleValue);
    compOn      ->setScale (scaleValue);
    compSidechain->setScale (scaleValue);

    pitchWheelLabel.setFont (juce::FontOptions (juce::jmax (7.0f, 9.0f * scaleValue),
                                                juce::Font::bold));
    modWheelLabel  .setFont (juce::FontOptions (juce::jmax (7.0f, 9.0f * scaleValue),
                                                juce::Font::bold));
}

// ==================== paint ====================

void PPGWave3Editor::paint (juce::Graphics& g)
{
    g.fillAll (PPGLookAndFeel::bgApp());

    auto top = getLocalBounds().removeFromTop (90);
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

    drawSection (g, osc1Area,    "OSCILLATOR 1");
    drawSection (g, osc2Area,    "OSCILLATOR 2");
    drawSection (g, filterArea,  "FILTER");
    drawSection (g, lfoArea,     "LFO");
    drawSection (g, modArea,     "MODULATION MATRIX");
    drawSection (g, fxArea,      "EFFECTS");
    drawSection (g, envArea,     "ENVELOPES");

    if (! keyboardArea.isEmpty())
    {
        const auto r = keyboardArea.toFloat();
        juce::ColourGradient grad (juce::Colour (0xff1c1c1c), r.getX(), r.getY(),
                                   juce::Colour (0xff151515), r.getX(), r.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (r, 4.0f);

        g.setColour (juce::Colour (0xff2f2f2f));
        g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.0f);

        g.setColour (PPGLookAndFeel::accent());
        g.setFont (juce::Font (juce::FontOptions (9.5f * currentScale, juce::Font::bold)));
        g.drawText ("KEYBOARD", keyboardArea.getX() + 8, keyboardArea.getY() + 3,
                    200, 12, juce::Justification::centredLeft);

        g.setColour (PPGLookAndFeel::accent().withAlpha (0.35f));
        g.fillRect (keyboardArea.getX() + 8, keyboardArea.getY() + 16, 200, 1);
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

void PPGWave3Editor::drawLogo (juce::Graphics& g, juce::Rectangle<int> area) const
{
    // FASE 7.1a: nuevo logo OML / Open Morph Labs en dos líneas.
    const float h = (float) area.getHeight();

    auto topRow = area.removeFromTop (juce::roundToInt (h * 0.62f));

    const float omlSize = juce::jmax (18.0f, h * 0.65f * currentScale);
    g.setFont (juce::Font (juce::FontOptions (omlSize, juce::Font::bold)));
    g.setColour (PPGLookAndFeel::accent());
    g.drawText ("OML", topRow, juce::Justification::centredLeft, false);

    const float subSize = juce::jmax (8.0f, h * 0.22f * currentScale);
    g.setFont (juce::Font (juce::FontOptions (subSize, juce::Font::plain)));
    g.setColour (PPGLookAndFeel::textDim());
    g.drawText ("Open Morph Labs", area, juce::Justification::centredLeft, false);
}

// ==================== resized ====================

void PPGWave3Editor::resized()
{
    currentScale = computeScale();
    applyScaleToAll (currentScale);

    auto r = getLocalBounds();

    // ===== Header (90px) =====
    auto header = r.removeFromTop (90);
    {
        auto h = header.reduced (10, 6);

        // --- Zona derecha: MASTER + 2 VU meters ---
        auto masterZone = h.removeFromRight (260);
        h.removeFromRight (8);

        // --- Zona izquierda: logo OML ---
        auto logoZone = h.removeFromLeft (170);
        drawLogoCacheArea = logoZone;   // guardamos para paint()
        h.removeFromLeft (8);

        // --- Zona central: preset bar ---
        auto presetZone = h;

        // Layout del master zone:
        // [MASTER label 14px]
        // [knob 62x62] [gap 8] [meters column: GR 26 / gap 4 / MST 26]
        masterZone.removeFromTop (12);   // reservamos para la etiqueta "MASTER" (dibujada en paint del knob)
        const int innerH = masterZone.getHeight();

        auto knobArea = masterZone.removeFromLeft (72);
        master.setBounds (knobArea.withSizeKeepingCentre (62, juce::jmin (innerH, 70)));

        masterZone.removeFromLeft (8);

        const int meterH = (innerH - 4) / 2;
        auto grArea  = masterZone.removeFromTop (meterH);
        masterZone.removeFromTop (4);
        auto mstArea = masterZone;

        compGrMeter.setBounds (grArea);
        masterHztMeter.setBounds (mstArea);

        // Layout del preset zone:
        // [<] [>] [gap] [PresetDisplay flexible] [gap] [LOAD] [SAVE] [BROWSE] [gap] [★] [FAV]
        const int navW   = 28;
        const int smallW = 68;
        const int medW   = 86;
        const int starW  = 30;
        const int favW   = 46;

        prevBtn.setBounds (presetZone.removeFromLeft (navW));
        nextBtn.setBounds (presetZone.removeFromLeft (navW));
        presetZone.removeFromLeft (8);

        browseBtn.setBounds (presetZone.removeFromRight (medW));
        presetZone.removeFromRight (4);
        saveBtn  .setBounds (presetZone.removeFromRight (smallW));
        presetZone.removeFromRight (4);
        loadBtn  .setBounds (presetZone.removeFromRight (smallW));
        presetZone.removeFromRight (10);

        favoritesOnlyBtn.setBounds (presetZone.removeFromRight (favW));
        presetZone.removeFromRight (2);
        favoriteBtn.setBounds (presetZone.removeFromRight (starW));
        presetZone.removeFromRight (10);

        presetDisplay.setBounds (presetZone);
    }

    // ===== Tira inferior: teclado + ruedas =====
    auto keyboardStrip = r.removeFromBottom (92);
    keyboardArea = keyboardStrip;

    keyboardStrip.reduce (10, 8);

    const int wheelW = juce::jmax (30, juce::roundToInt (44.0f * currentScale));
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

    // ===== Área de módulos =====
    r.reduce (6, 6);

    const int h      = r.getHeight();
    const int gap    = 6;
    const int availH = h - 6 * gap;

    const float availHf = (float) availH;
    const int oscH  = (int) (availHf * 0.095f);
    const int filtH = (int) (availHf * 0.095f);
    const int lfoH  = (int) (availHf * 0.105f);
    const int modH  = (int) (availHf * 0.155f);
    const int fxH   = (int) (availHf * 0.265f);
    const int bottomH = availH - 2 * oscH - filtH - lfoH - modH - fxH;

    osc1Area = r.removeFromTop (oscH);          r.removeFromTop (gap);
    osc2Area = r.removeFromTop (oscH);          r.removeFromTop (gap);
    filterArea = r.removeFromTop (filtH);       r.removeFromTop (gap);
    lfoArea = r.removeFromTop (lfoH);           r.removeFromTop (gap);
    modArea = r.removeFromTop (modH);           r.removeFromTop (gap);
    fxArea  = r.removeFromTop (fxH);            r.removeFromTop (gap);

    // FASE 7.1a: la fila inferior es SOLO envelopes (el master subió al header).
    envArea = r;

    auto infoWidthFor = [] (int sectionW)
    {
        return juce::jlimit (90, 260, (int) ((float) sectionW * 0.22f));
    };

    auto titleRowFor = [&] (juce::Rectangle<int> area, InfoDisplay& info,
                            juce::Rectangle<int>& innerOut)
    {
        innerOut = area.reduced (8);
        auto titleRow = innerOut.removeFromTop (
            juce::jmax (14, juce::roundToInt (15.0f * currentScale)));
        info.setBounds (titleRow.removeFromRight (
            infoWidthFor (area.getWidth())).reduced (0, 1));
        innerOut.removeFromTop (1);
    };

    auto layoutOscSection = [&] (juce::Rectangle<int> area,
                                 InfoDisplay& info, ButtonSelector& waveSel,
                                 ui::WavetablePreview& preview,
                                 RotaryKnob& kPos, RotaryKnob& kOct, RotaryKnob& kSemi,
                                 RotaryKnob& kFine, RotaryKnob& kLevel)
    {
        juce::Rectangle<int> inner;
        titleRowFor (area, info, inner);

        const int selW = (int) ((float) inner.getWidth() * 0.24f);
        waveSel.setBounds (inner.removeFromLeft (selW).reduced (2, 3));
        inner.removeFromLeft (4);
        const int previewW = (int) ((float) inner.getWidth() * 0.20f);
        preview.setBounds (inner.removeFromLeft (previewW).reduced (2, 3));
        inner.removeFromLeft (4);

        const int kw = inner.getWidth() / 5;
        kPos  .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        kOct  .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        kSemi .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        kFine .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        kLevel.setBounds (inner.reduced (1, 0));
    };

    layoutOscSection (osc1Area, osc1Info, *osc1Wave, osc1Preview,
                      osc1Pos, osc1Oct, osc1Semi, osc1Fine, osc1Level);
    layoutOscSection (osc2Area, osc2Info, *osc2Wave, osc2Preview,
                      osc2Pos, osc2Oct, osc2Semi, osc2Fine, osc2Level);

    {
        juce::Rectangle<int> inner;
        titleRowFor (filterArea, filterInfo, inner);
        const int selW = (int) ((float) inner.getWidth() * 0.22f);
        filterType->setBounds (inner.removeFromLeft (selW).reduced (2, 3));
        inner.removeFromLeft (4);
        const int kw = inner.getWidth() / 4;
        filterCutoff  .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filterReso    .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filterEnvAmt  .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filterKeyTrack.setBounds (inner.reduced (1, 0));
    }

    {
        juce::Rectangle<int> inner;
        titleRowFor (lfoArea, lfoInfo, inner);
        const int halfW = (inner.getWidth() - gap) / 2;

        auto layoutLFO = [&] (juce::Rectangle<int> area,
                              ComboBoxSelector& w, ComboBoxSelector& sync,
                              ui::LFODisplay& display,
                              RotaryKnob& rate, RotaryKnob& depth, RotaryKnob& phase)
        {
            const int comboW = (int) ((float) area.getWidth() * 0.16f);
            w.setBounds (area.removeFromLeft (comboW).reduced (2, 0));
            area.removeFromLeft (2);
            sync.setBounds (area.removeFromLeft (comboW).reduced (2, 0));
            area.removeFromLeft (4);
            const int dispW = (int) ((float) area.getWidth() * 0.30f);
            display.setBounds (area.removeFromLeft (dispW).reduced (2, 3));
            area.removeFromLeft (4);
            const int kw = area.getWidth() / 3;
            rate .setBounds (area.removeFromLeft (kw).reduced (1, 0));
            depth.setBounds (area.removeFromLeft (kw).reduced (1, 0));
            phase.setBounds (area.reduced (1, 0));
        };

        auto lfo1Zone = inner.removeFromLeft (halfW);
        inner.removeFromLeft (gap);
        auto lfo2Zone = inner;

        layoutLFO (lfo1Zone, *lfo1Wave, *lfo1Sync, lfo1Display,
                   lfo1Rate, lfo1Depth, lfo1Phase);
        layoutLFO (lfo2Zone, *lfo2Wave, *lfo2Sync, lfo2Display,
                   lfo2Rate, lfo2Depth, lfo2Phase);
    }

    {
        juce::Rectangle<int> inner;
        titleRowFor (modArea, modInfo, inner);
        const int rowH = inner.getHeight() / 4;

        auto layoutRow = [&] (juce::Rectangle<int> row,
                              ComboBoxSelector& src, ComboBoxSelector& dst, HSlider& amt)
        {
            const int srcW = (int) ((float) row.getWidth() * 0.35f);
            const int dstW = (int) ((float) row.getWidth() * 0.35f);
            src.setBounds (row.removeFromLeft (srcW).reduced (2, 1));
            dst.setBounds (row.removeFromLeft (dstW).reduced (2, 1));
            amt.setBounds (row.reduced (2, 1));
        };

        layoutRow (inner.removeFromTop (rowH), *mod1Src, *mod1Dst, mod1Amt);
        layoutRow (inner.removeFromTop (rowH), *mod2Src, *mod2Dst, mod2Amt);
        layoutRow (inner.removeFromTop (rowH), *mod3Src, *mod3Dst, mod3Amt);
        layoutRow (inner,                        *mod4Src, *mod4Dst, mod4Amt);
    }

    {
        juce::Rectangle<int> inner;
        titleRowFor (fxArea, fxInfo, inner);

        auto tabRow = inner.removeFromTop (24);
        const int tabW = tabRow.getWidth() / 8;
        driveTabBtn  .setBounds (tabRow.removeFromLeft (tabW).reduced (1, 0));
        chorusTabBtn .setBounds (tabRow.removeFromLeft (tabW).reduced (1, 0));
        phaserTabBtn .setBounds (tabRow.removeFromLeft (tabW).reduced (1, 0));
        delayTabBtn  .setBounds (tabRow.removeFromLeft (tabW).reduced (1, 0));
        reverbTabBtn .setBounds (tabRow.removeFromLeft (tabW).reduced (1, 0));
        vintageTabBtn.setBounds (tabRow.removeFromLeft (tabW).reduced (1, 0));
        eqTabBtn     .setBounds (tabRow.removeFromLeft (tabW).reduced (1, 0));
        compTabBtn   .setBounds (tabRow.reduced (1, 0));

        inner.removeFromTop (6);

        const auto controlsArea = inner.reduced (30, 4);

        auto layoutFullRow = [] (juce::Rectangle<int> area,
                                 ToggleButton& on,
                                 RotaryKnob& k1, RotaryKnob& k2, RotaryKnob& k3)
        {
            auto toggleArea = area.removeFromLeft (110);
            on.setBounds (toggleArea.withSizeKeepingCentre (
                toggleArea.getWidth() - 12, 40));

            area.removeFromLeft (24);

            const int kw = area.getWidth() / 3;
            k1.setBounds (area.removeFromLeft (kw).reduced (6, 0));
            k2.setBounds (area.removeFromLeft (kw).reduced (6, 0));
            k3.setBounds (area.reduced (6, 0));
        };

        { auto area = controlsArea; layoutFullRow (area, *driveOn, driveAmount, driveTone, driveMix); }
        { auto area = controlsArea; layoutFullRow (area, *chorusOn, chorusRate, chorusDepth, chorusMix); }

        // Phaser
        {
            auto area = controlsArea;
            auto toggleArea = area.removeFromLeft (110);
            phaserOn->setBounds (toggleArea.withSizeKeepingCentre (
                toggleArea.getWidth() - 12, 40));
            area.removeFromLeft (24);
            const int kw = area.getWidth() / 4;
            phaserRate    .setBounds (area.removeFromLeft (kw).reduced (4, 0));
            phaserDepth   .setBounds (area.removeFromLeft (kw).reduced (4, 0));
            phaserFeedback.setBounds (area.removeFromLeft (kw).reduced (4, 0));
            phaserMix     .setBounds (area.reduced (4, 0));
        }

        // Delay
        {
            auto area = controlsArea;
            auto toggleArea = area.removeFromLeft (110);
            delayOn->setBounds (toggleArea.withSizeKeepingCentre (toggleArea.getWidth() - 12, 40));
            area.removeFromLeft (16);
            auto syncArea = area.removeFromLeft (150);
            delaySync->setBounds (syncArea.withSizeKeepingCentre (syncArea.getWidth() - 12, 52));
            area.removeFromLeft (16);
            const int kw = area.getWidth() / 3;
            delayTime    .setBounds (area.removeFromLeft (kw).reduced (6, 0));
            delayFeedback.setBounds (area.removeFromLeft (kw).reduced (6, 0));
            delayMix     .setBounds (area.reduced (6, 0));
        }

        { auto area = controlsArea; layoutFullRow (area, *reverbOn, reverbSize, reverbDamp, reverbMix); }

        // Vintage
        {
            auto area = controlsArea;

            const int toggleW = 110;
            auto toggleCol = area.removeFromLeft (toggleW);
            vintageOn->setBounds (toggleCol.withSizeKeepingCentre (
                toggleCol.getWidth() - 12, 40));

            area.removeFromLeft (24);

            const int rowGap = 6;
            const int rowH = (area.getHeight() - rowGap) / 2;
            auto topKnobRow = area.removeFromTop (rowH);
            area.removeFromTop (rowGap);
            auto botKnobRow = area;

            const int kwTop = topKnobRow.getWidth() / 3;
            vintageAmount.setBounds (topKnobRow.removeFromLeft (kwTop).reduced (6, 0));
            vintageBits  .setBounds (topKnobRow.removeFromLeft (kwTop).reduced (6, 0));
            vintageSr    .setBounds (topKnobRow.reduced (6, 0));

            const int kwBot = botKnobRow.getWidth() / 3;
            vintageNoise .setBounds (botKnobRow.removeFromLeft (kwBot).reduced (6, 0));
            vintageDrift .setBounds (botKnobRow.removeFromLeft (kwBot).reduced (6, 0));
            vintageVar   .setBounds (botKnobRow.reduced (6, 0));
        }

        // EQ
        {
            auto area = controlsArea;

            const int topRowH = 26;
            auto topRow = area.removeFromTop (topRowH);
            area.removeFromTop (4);

            const int eqOnW = 70;
            eqOn->setBounds (topRow.removeFromLeft (eqOnW).reduced (2, 2));
            topRow.removeFromLeft (8);

            const int bandW = (int) ((float) topRow.getWidth() * 0.55f / 4.0f);
            eqLowBtn .setBounds (topRow.removeFromLeft (bandW).reduced (2, 2));
            eqLmidBtn.setBounds (topRow.removeFromLeft (bandW).reduced (2, 2));
            eqHmidBtn.setBounds (topRow.removeFromLeft (bandW).reduced (2, 2));
            eqHighBtn.setBounds (topRow.removeFromLeft (bandW).reduced (2, 2));

            topRow.removeFromLeft (8);

            const int hpLpW = juce::jmin (70, topRow.getWidth() / 2);
            eqHpOn->setBounds (topRow.removeFromLeft (hpLpW).reduced (2, 2));
            eqLpOn->setBounds (topRow.removeFromLeft (hpLpW).reduced (2, 2));

            auto bottomRow = area;

            const int knobZoneW = (int) ((float) bottomRow.getWidth() * 0.40f);
            auto knobZone = bottomRow.removeFromRight (knobZoneW);
            bottomRow.removeFromRight (6);
            auto curveZone = bottomRow;

            eqCurveDisplay.setBounds (curveZone.reduced (2, 0));

            const int kw = knobZone.getWidth() / 3;
            eqFreqKnob.setBounds (knobZone.removeFromLeft (kw).reduced (4, 0));
            eqQKnob   .setBounds (knobZone.removeFromLeft (kw).reduced (4, 0));
            eqGainKnob.setBounds (knobZone.reduced (4, 0));
        }

        // Compressor
        {
            auto area = controlsArea;

            const int toggleColW = 110;
            auto toggleCol = area.removeFromLeft (toggleColW);
            const int halfToggleH = toggleCol.getHeight() / 2;
            compOn->setBounds (toggleCol.removeFromTop (halfToggleH).reduced (0, 2));
            compSidechain->setBounds (toggleCol.reduced (0, 2));

            area.removeFromLeft (24);

            const int rowGap = 6;
            const int rowH = (area.getHeight() - rowGap) / 2;
            auto topKnobRow = area.removeFromTop (rowH);
            area.removeFromTop (rowGap);
            auto botKnobRow = area;

            const int kwTop = topKnobRow.getWidth() / 4;
            compThreshold.setBounds (topKnobRow.removeFromLeft (kwTop).reduced (4, 0));
            compRatio    .setBounds (topKnobRow.removeFromLeft (kwTop).reduced (4, 0));
            compAttack   .setBounds (topKnobRow.removeFromLeft (kwTop).reduced (4, 0));
            compRelease  .setBounds (topKnobRow.reduced (4, 0));

            const int kwBot = botKnobRow.getWidth() / 3;
            compKnee     .setBounds (botKnobRow.removeFromLeft (kwBot).reduced (4, 0));
            compMakeup   .setBounds (botKnobRow.removeFromLeft (kwBot).reduced (4, 0));
            compScAmount .setBounds (botKnobRow.reduced (4, 0));
        }
    }

    // Envelopes (ahora ocupan todo el ancho)
    {
        juce::Rectangle<int> inner;
        titleRowFor (envArea, envInfo, inner);

        auto tabRow = inner.removeFromTop (24);
        const int tabW = tabRow.getWidth() / 3;
        env1TabBtn.setBounds (tabRow.removeFromLeft (tabW).reduced (1, 0));
        env2TabBtn.setBounds (tabRow.removeFromLeft (tabW).reduced (1, 0));
        env3TabBtn.setBounds (tabRow.reduced (1, 0));

        inner.removeFromTop (6);

        auto drawArea = inner;

        const int dispH = (int) ((float) drawArea.getHeight() * 0.42f);
        auto dispRow = drawArea.removeFromTop (dispH).reduced (2, 2);
        env1Display.setBounds (dispRow);
        env2Display.setBounds (dispRow);
        env3Display.setBounds (dispRow);

        drawArea.removeFromTop (2);

        const int kw = drawArea.getWidth() / 4;
        auto layoutKnobRow = [&] (RotaryKnob& a, RotaryKnob& d,
                                  RotaryKnob& s, RotaryKnob& rr)
        {
            auto row = drawArea;
            a .setBounds (row.removeFromLeft (kw).reduced (1, 0));
            d .setBounds (row.removeFromLeft (kw).reduced (1, 0));
            s .setBounds (row.removeFromLeft (kw).reduced (1, 0));
            rr.setBounds (row.reduced (1, 0));
        };

        layoutKnobRow (env1A, env1D, env1S, env1R);
        layoutKnobRow (env2A, env2D, env2S, env2R);
        layoutKnobRow (env3A, env3D, env3S, env3R);
    }

    repaint();
}
