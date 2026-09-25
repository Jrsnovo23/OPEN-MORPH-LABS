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

    // Sincroniza los controles con el modelo (al cargar preset, reset, etc.).
    void refreshFromModel();

    // Marca este paso como el que está sonando en este momento.
    void setPlayingStep (bool isCurrent);

    int getStepIndex() const noexcept { return idx; }

private:
    static juce::LookAndFeel_V4& getSeqLnf();

    StepSequencer& sequencerRef;
    const int idx;

    juce::TextButton onOffBtn;
    juce::Slider     pitchSlider;
    juce::Slider     velSlider;

    bool isCurrentStep = false;
};
