#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../DSP/StepSequencer.h"

class SeqStepControl : public juce::Component
{
public:
    SeqStepControl (StepSequencer& seq, int stepIndex);
    ~SeqStepControl() override;

    void resized() override;
    void paint   (juce::Graphics&) override;

    void refreshFromModel();
    void setPlayingStep (bool isCurrent);

    // Lane activa: 0=Pitch, 1=Velocity, 2=Gate, 3=Probability
    void setActiveLane (int lane);

    int getStepIndex() const noexcept { return idx; }

private:
    static juce::LookAndFeel_V4& getSeqLnf();

    StepSequencer& sequencerRef;
    const int idx;
    int activeLane = 0;

    juce::TextButton onOffBtn;
    juce::Slider     valueSlider;

    bool isCurrentStep = false;
};
