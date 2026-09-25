#include "SeqStepControl.h"

namespace
{
    // Look and feel propio para los sliders verticales del secuenciador.
    // No depende del PPGLookAndFeel, para no dibujarlos como wheels.
    class SeqSliderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawLinearSlider (juce::Graphics& g,
                               int x, int y, int width, int height,
                               float sliderPos, float /*min*/, float /*max*/,
                               juce::Slider::SliderStyle /*style*/,
                               juce::Slider& slider) override
        {
            const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
            const auto range = slider.getRange();
            const bool bipolar = (range.getStart() < 0.0 && range.getEnd() > 0.0);

            // Fondo
            g.setColour (juce::Colour (0xff0a0a0a));
            g.fillRoundedRectangle (bounds, 2.0f);
            g.setColour (juce::Colour (0xff2a2a2a));
            g.drawRoundedRectangle (bounds.reduced (0.5f), 2.0f, 1.0f);

            // Línea de cero si es bipolar (pitch)
            float fillFrom = bounds.getBottom();
            if (bipolar)
            {
                const float zeroNorm = (float) ((0.0 - range.getStart())
                                              / (range.getEnd() - range.getStart()));
                const float zeroY = bounds.getBottom() - zeroNorm * bounds.getHeight();
                g.setColour (juce::Colour (0xff555555));
                g.fillRect (bounds.getX() + 1.0f, zeroY - 0.5f,
                            bounds.getWidth() - 2.0f, 1.0f);
                fillFrom = zeroY;
            }

            // Fill desde línea base hasta el thumb
            const float top = juce::jmin (sliderPos, fillFrom);
            const float bot = juce::jmax (sliderPos, fillFrom);
            if (bot - top > 0.5f)
            {
                g.setColour (juce::Colour (0xffffaa00).withAlpha (0.85f));
                g.fillRoundedRectangle (bounds.getX() + 1.0f, top,
                                        bounds.getWidth() - 2.0f, bot - top, 2.0f);
            }

            // Marcador del thumb
            g.setColour (juce::Colour (0xffffe08a));
            g.fillRect (bounds.getX() + 1.0f, sliderPos - 0.5f,
                        bounds.getWidth() - 2.0f, 1.2f);
        }
    };
}

juce::LookAndFeel_V4& SeqStepControl::getSeqLnf()
{
    static SeqSliderLookAndFeel lnf;
    return lnf;
}

SeqStepControl::SeqStepControl (StepSequencer& seq, int stepIndex)
    : sequencerRef (seq), idx (stepIndex)
{
    // ON/OFF con número de paso
    onOffBtn.setButtonText (juce::String (idx + 1));
    onOffBtn.setClickingTogglesState (true);
    onOffBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff202020));
    onOffBtn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
    onOffBtn.setColour (juce::TextButton::textColourOffId,  juce::Colour (0xff808080));
    onOffBtn.setColour (juce::TextButton::textColourOnId,   juce::Colours::black);

    onOffBtn.onClick = [this]()
    {
        sequencerRef.steps[idx].active.store (onOffBtn.getToggleState());
    };
    addAndMakeVisible (onOffBtn);

    // Pitch
    pitchSlider.setSliderStyle (juce::Slider::LinearVertical);
    pitchSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    pitchSlider.setRange (-24.0, 24.0, 1.0);
    pitchSlider.setValue (0.0, juce::dontSendNotification);
    pitchSlider.setDoubleClickReturnValue (true, 0.0);
    pitchSlider.setLookAndFeel (&getSeqLnf());
    pitchSlider.onValueChange = [this]()
    {
        sequencerRef.steps[idx].pitch.store ((int) pitchSlider.getValue());
    };
    addAndMakeVisible (pitchSlider);

    // Velocity
    velSlider.setSliderStyle (juce::Slider::LinearVertical);
    velSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    velSlider.setRange (0.0, 1.0, 0.01);
    velSlider.setValue (0.8, juce::dontSendNotification);
    velSlider.setDoubleClickReturnValue (true, 0.8);
    velSlider.setLookAndFeel (&getSeqLnf());
    velSlider.onValueChange = [this]()
    {
        sequencerRef.steps[idx].velocity.store ((float) velSlider.getValue());
    };
    addAndMakeVisible (velSlider);
}

SeqStepControl::~SeqStepControl()
{
    pitchSlider.setLookAndFeel (nullptr);
    velSlider.setLookAndFeel (nullptr);
}

void SeqStepControl::refreshFromModel()
{
    auto& step = sequencerRef.steps[idx];
    onOffBtn.setToggleState (step.active.load(), juce::dontSendNotification);
    pitchSlider.setValue ((double) step.pitch.load(), juce::dontSendNotification);
    velSlider.setValue ((double) step.velocity.load(), juce::dontSendNotification);
}

void SeqStepControl::setPlayingStep (bool isCurrent)
{
    if (isCurrentStep != isCurrent)
    {
        isCurrentStep = isCurrent;
        repaint();
    }
}

void SeqStepControl::paint (juce::Graphics& g)
{
    if (! isCurrentStep) return;

    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colour (0xffffaa00).withAlpha (0.18f));
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (juce::Colour (0xffffcc55).withAlpha (0.55f));
    g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);
}

void SeqStepControl::resized()
{
    auto r = getLocalBounds();
    onOffBtn.setBounds (r.removeFromTop (18));
    r.removeFromTop (2);

    const int halfH = r.getHeight() / 2;
    pitchSlider.setBounds (r.removeFromTop (halfH - 2));
    r.removeFromTop (4);
    velSlider.setBounds (r);
}
