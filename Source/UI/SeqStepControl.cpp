#include "SeqStepControl.h"

namespace
{
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

            g.setColour (juce::Colour (0xff0a0a0a));
            g.fillRoundedRectangle (bounds, 2.0f);
            g.setColour (juce::Colour (0xff2a2a2a));
            g.drawRoundedRectangle (bounds.reduced (0.5f), 2.0f, 1.0f);

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

            const float top = juce::jmin (sliderPos, fillFrom);
            const float bot = juce::jmax (sliderPos, fillFrom);
            if (bot - top > 0.5f)
            {
                g.setColour (juce::Colour (0xffffaa00).withAlpha (0.85f));
                g.fillRoundedRectangle (bounds.getX() + 1.0f, top,
                                        bounds.getWidth() - 2.0f, bot - top, 2.0f);
            }

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

    valueSlider.setSliderStyle (juce::Slider::LinearVertical);
    valueSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    valueSlider.setDoubleClickReturnValue (true, 0.0);
    valueSlider.setLookAndFeel (&getSeqLnf());
    addAndMakeVisible (valueSlider);

    // Inicializar lane 0 (Pitch)
    setActiveLane (0);
}

SeqStepControl::~SeqStepControl()
{
    valueSlider.setLookAndFeel (nullptr);
}

void SeqStepControl::setActiveLane (int lane)
{
    activeLane = juce::jlimit (0, 3, lane);

    valueSlider.onValueChange = nullptr;

    switch (activeLane)
    {
        case 0:  // Pitch
            valueSlider.setRange (-24.0, 24.0, 1.0);
            valueSlider.setDoubleClickReturnValue (true, 0.0);
            valueSlider.setValue ((double) sequencerRef.steps[idx].pitch.load(),
                                  juce::dontSendNotification);
            break;

        case 1:  // Velocity
            valueSlider.setRange (0.0, 1.0, 0.01);
            valueSlider.setDoubleClickReturnValue (true, 0.8);
            valueSlider.setValue ((double) sequencerRef.steps[idx].velocity.load(),
                                  juce::dontSendNotification);
            break;

        case 2:  // Gate
            valueSlider.setRange (0.1, 2.0, 0.01);
            valueSlider.setDoubleClickReturnValue (true, 1.0);
            valueSlider.setValue ((double) sequencerRef.steps[idx].gate.load(),
                                  juce::dontSendNotification);
            break;

        case 3:  // Probability
            valueSlider.setRange (0.0, 1.0, 0.01);
            valueSlider.setDoubleClickReturnValue (true, 1.0);
            valueSlider.setValue ((double) sequencerRef.steps[idx].probability.load(),
                                  juce::dontSendNotification);
            break;
    }

    valueSlider.onValueChange = [this]()
    {
        const double v = valueSlider.getValue();
        auto& step = sequencerRef.steps[idx];

        switch (activeLane)
        {
            case 0: step.pitch.store       ((int) v);   break;
            case 1: step.velocity.store    ((float) v); break;
            case 2: step.gate.store        ((float) v); break;
            case 3: step.probability.store ((float) v); break;
        }
    };

    repaint();
}

void SeqStepControl::refreshFromModel()
{
    auto& step = sequencerRef.steps[idx];

    onOffBtn.setToggleState (step.active.load(), juce::dontSendNotification);

    valueSlider.onValueChange = nullptr;

    switch (activeLane)
    {
        case 0: valueSlider.setValue ((double) step.pitch.load(),       juce::dontSendNotification); break;
        case 1: valueSlider.setValue ((double) step.velocity.load(),    juce::dontSendNotification); break;
        case 2: valueSlider.setValue ((double) step.gate.load(),        juce::dontSendNotification); break;
        case 3: valueSlider.setValue ((double) step.probability.load(), juce::dontSendNotification); break;
    }

    valueSlider.onValueChange = [this]()
    {
        const double v = valueSlider.getValue();
        auto& s = sequencerRef.steps[idx];
        switch (activeLane)
        {
            case 0: s.pitch.store       ((int) v);   break;
            case 1: s.velocity.store    ((float) v); break;
            case 2: s.gate.store        ((float) v); break;
            case 3: s.probability.store ((float) v); break;
        }
    };
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
    auto r = getLocalBounds().toFloat().reduced (1.0f);

    // Si la probabilidad del paso es < 1, mostramos un pequeño indicador
    const float prob = sequencerRef.steps[idx].probability.load();

    if (prob < 0.999f)
    {
        g.setColour (juce::Colour (0x33ffffff));
        for (int i = 0; i < (int) r.getWidth() + (int) r.getHeight(); i += 6)
        {
            g.drawLine (r.getX() + i, r.getY(),
                        r.getX() + i - r.getHeight(), r.getBottom(),
                        1.0f);
        }
    }

    if (! isCurrentStep) return;

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
    valueSlider.setBounds (r);
}
