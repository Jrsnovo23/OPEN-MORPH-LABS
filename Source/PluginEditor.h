#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/PPGLookAndFeel.h"
#include "UI/Visualizers.h"
#include "UI/SeqStepControl.h"
#include "Presets/PresetManager.h"

class PPGWave3Editor : public juce::AudioProcessorEditor,
                       private juce::Timer
{
public:
    explicit PPGWave3Editor (PPGWave3Processor&);
    ~PPGWave3Editor() override;

    void paint   (juce::Graphics&) override;
    void resized () override;

private:
    void timerCallback() override;

    class InfoDisplay : public juce::Component
    {
    public:
        void setInfo (const juce::String& name, const juce::String& value);
        void setScale (float s) { scale = s; repaint(); }
        void paint (juce::Graphics&) override;
    private:
        juce::String paramName { "--" };
        juce::String paramValue;
        float scale = 1.0f;
    };

    class RotaryKnob : public juce::Component
    {
    public:
        RotaryKnob (juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& paramID,
                    const juce::String& labelText,
                    InfoDisplay* display);
        void resized() override;
        void paint   (juce::Graphics&) override;
        void setScale (float s);
    private:
        juce::Slider  slider;
        juce::Label   label;
        ValueBoxLabel valueLabel;
        InfoDisplay*  infoDisplay = nullptr;
        juce::String  paramName;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        float         scale = 1.0f;
    };

    class HSlider : public juce::Component
    {
    public:
        HSlider (juce::AudioProcessorValueTreeState& apvts,
                 const juce::String& paramID,
                 InfoDisplay* display);
        void resized() override;
        void paint   (juce::Graphics&) override;
        void setScale (float s);
    private:
        juce::Slider  slider;
        InfoDisplay*  infoDisplay = nullptr;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        float         scale = 1.0f;
    };

    class ButtonSelector : public juce::Component
    {
    public:
        ButtonSelector (juce::AudioProcessorValueTreeState& apvts,
                        const juce::String& paramID,
                        const juce::StringArray& names);
        void resized() override;
        void paint   (juce::Graphics&) override;
        void setScale (float s) { scale = s; }
    private:
        void refreshFromParameter();
        juce::AudioProcessorValueTreeState& apvtsRef;
        juce::String id;
        juce::OwnedArray<juce::TextButton> buttons;
        std::unique_ptr<juce::ParameterAttachment> attachment;
        int currentIndex = 0;
        float scale = 1.0f;
    };

    class ComboBoxSelector : public juce::Component
    {
    public:
        ComboBoxSelector (juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& paramID,
                          const juce::String& labelText,
                          InfoDisplay* display);
        void resized() override;
        void paint   (juce::Graphics&) override;
        void setScale (float s);
    private:
        juce::ComboBox combo;
        juce::Label    label;
        InfoDisplay*   infoDisplay = nullptr;
        juce::String   paramName;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
        float          scale = 1.0f;
    };

    class ToggleButton : public juce::Component
    {
    public:
        ToggleButton (juce::AudioProcessorValueTreeState& apvts,
                      const juce::String& paramID,
                      const juce::String& labelText,
                      InfoDisplay* display);
        void resized() override;
        void paint   (juce::Graphics&) override;
        void setScale (float s);
    private:
        juce::TextButton button;
        InfoDisplay*     infoDisplay = nullptr;
        juce::String     paramName;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
        float            scale = 1.0f;
    };

    class EQBandKnob : public juce::Component,
                       private juce::Timer
    {
    public:
        EQBandKnob (juce::AudioProcessorValueTreeState& apvts,
                    const juce::StringArray& paramIdsForBands,
                    const juce::String& labelText,
                    InfoDisplay* display);
        ~EQBandKnob() override;
        void setActiveBand (int band);
        void setScale (float s);
        void resized() override;
        void paint   (juce::Graphics&) override;
    private:
        void timerCallback() override;
        void sliderChanged();
        void refreshSliderFromParam();
        void updateInfoText();

        juce::AudioProcessorValueTreeState& apvtsRef;
        juce::StringArray ids;
        juce::Slider  slider;
        juce::Label   label;
        ValueBoxLabel valueLabel;
        InfoDisplay*  infoDisplay = nullptr;
        juce::String  paramName;
        int           activeBand = 0;
        bool          updatingFromParam = false;
        float         scale = 1.0f;
    };

    class EQCurveDisplay : public juce::Component,
                           private juce::Timer
    {
    public:
        explicit EQCurveDisplay (PPGWave3Processor& processor);
        ~EQCurveDisplay() override;
        void paint (juce::Graphics&) override;
    private:
        void timerCallback() override;
        struct Cache
        {
            bool  on = true;
            bool  hpOn = false, lpOn = false;
            float lowF = 100.0f,  lowQ = 0.707f, lowG = 0.0f;
            float lmidF = 500.0f, lmidQ = 0.707f, lmidG = 0.0f;
            float hmidF = 2000.0f, hmidQ = 0.707f, hmidG = 0.0f;
            float highF = 8000.0f, highQ = 0.707f, highG = 0.0f;
        };
        Cache readParams() const;
        static bool cacheChanged (const Cache& a, const Cache& b);

        PPGWave3Processor& processorRef;
        juce::AudioProcessorValueTreeState& apvtsRef;
        Cache cached;
        bool  hasCached = false;
        float scale = 1.0f;
        float sonarPhase = 0.0f;
    };

    class PresetDisplay : public juce::Component
    {
    public:
        void setInfo (const juce::String& name, const juce::String& category, bool factory);
        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;

        std::function<void()> onOpenMenu;
    private:
        juce::String presetName { "Init Saw" };
        juce::String presetCategory { "Init" };
        bool isFactory = true;
    };

    class FxTab : public juce::Component
    {
    public:
        FxTab (juce::AudioProcessorValueTreeState& apvts,
               const juce::String& toggleParamId,
               const juce::String& label);

        void resized() override;
        void paint   (juce::Graphics&) override;

    private:
        juce::String text;

        juce::TextButton toggleBtn;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    };

    void  drawSection (juce::Graphics& g, juce::Rectangle<int> area,
                       const juce::String& title) const;
    void  drawBox (juce::Graphics& g, juce::Rectangle<int> area) const;
    void  drawLogo (juce::Graphics& g, juce::Rectangle<int> area) const;
    float computeScale() const;
    void  applyScaleToAll (float s);

    void  updatePresetDisplay();
    void  onPrevPreset();
    void  onNextPreset();
    void  onLoadPreset();
    void  onSavePreset();
    void  onBrowsePreset();

    void  showPresetMenu();
    void  onToggleFavorite();
    void  onToggleFavoritesOnly();

    void  updateFxVisibility();
    void  updateEnvVisibility();
    void  setActiveEqBand (int band);

    void  layoutKnobStack (juce::Rectangle<int> col,
                           std::initializer_list<juce::Component*> knobs,
                           int itemHeight);
    void  layoutKnobStackBottom (juce::Rectangle<int> col,
                                 std::initializer_list<juce::Component*> knobs,
                                 int itemHeight);

    // FASE 10: reset del secuenciador (botón CLR)
    void  resetSequencer();

    PPGWave3Processor& processorRef;
    juce::AudioProcessorValueTreeState& apvts;
    PPGLookAndFeel ppgLnf;

    presets::PresetManager presetManager;

    juce::TextButton prevBtn, nextBtn;
    juce::TextButton favoriteBtn;
    juce::TextButton favoritesOnlyBtn;
    juce::TextButton loadBtn, saveBtn, browseBtn;
    PresetDisplay    presetDisplay;
    std::unique_ptr<juce::FileChooser> fileChooser;

    InfoDisplay osc1Info, osc2Info, filterInfo;
    InfoDisplay envInfo, masterInfo;
    InfoDisplay lfoInfo, modInfo;

    InfoDisplay fxInfoDrive, fxInfoChorus, fxInfoPhaser, fxInfoDelay;
    InfoDisplay fxInfoReverb, fxInfoVintage, fxInfoEq, fxInfoComp;

    std::unique_ptr<ButtonSelector> osc1Wave;
    ui::WavetablePreview osc1Preview;
    RotaryKnob osc1Pos, osc1Oct, osc1Semi, osc1Fine, osc1Level;
    std::unique_ptr<ButtonSelector> osc2Wave;
    ui::WavetablePreview osc2Preview;
    RotaryKnob osc2Pos, osc2Oct, osc2Semi, osc2Fine, osc2Level;

    std::unique_ptr<ButtonSelector> filterType;
    RotaryKnob filterCutoff, filterReso, filterEnvAmt, filterKeyTrack;

    ui::EnvelopeDisplay env1Display, env2Display, env3Display;
    RotaryKnob env1A, env1D, env1S, env1R;
    RotaryKnob env2A, env2D, env2S, env2R;
    RotaryKnob env3A, env3D, env3S, env3R;
    juce::TextButton env1TabBtn, env2TabBtn, env3TabBtn;
    int activeEnvTab = 0;

    RotaryKnob master;
    ui::StereoHorizontalMeter masterHztMeter;
    ui::HorizontalMeter       compGrMeter;
    std::unique_ptr<ComboBoxSelector> lfo1Wave;
    ui::LFODisplay lfo1Display;
    RotaryKnob lfo1Rate, lfo1Depth, lfo1Phase;
    std::unique_ptr<ComboBoxSelector> lfo1Sync;

    std::unique_ptr<ComboBoxSelector> lfo2Wave;
    ui::LFODisplay lfo2Display;
    RotaryKnob lfo2Rate, lfo2Depth, lfo2Phase;
    std::unique_ptr<ComboBoxSelector> lfo2Sync;

    std::unique_ptr<ComboBoxSelector> mod1Src, mod1Dst;  HSlider mod1Amt;
    std::unique_ptr<ComboBoxSelector> mod2Src, mod2Dst;  HSlider mod2Amt;
    std::unique_ptr<ComboBoxSelector> mod3Src, mod3Dst;  HSlider mod3Amt;
    std::unique_ptr<ComboBoxSelector> mod4Src, mod4Dst;  HSlider mod4Amt;

    std::unique_ptr<FxTab> driveTab, chorusTab, phaserTab;
    std::unique_ptr<FxTab> delayTab, reverbTab, vintageTab, eqTab, compTab;

    RotaryKnob driveAmount, driveTone, driveMix;
    RotaryKnob chorusRate, chorusDepth, chorusMix;
    std::unique_ptr<ComboBoxSelector> delaySync;
    RotaryKnob delayTime, delayFeedback, delayMix;
    RotaryKnob reverbSize, reverbDamp, reverbMix;
    RotaryKnob vintageAmount, vintageBits, vintageSr;
    RotaryKnob vintageNoise, vintageDrift, vintageVar;
    std::unique_ptr<ToggleButton> eqHpOn, eqLpOn;
    juce::TextButton eqLowBtn, eqLmidBtn, eqHmidBtn, eqHighBtn;
    EQBandKnob eqFreqKnob, eqQKnob, eqGainKnob;
    EQCurveDisplay eqCurveDisplay;
    int activeEqBand = 0;
    RotaryKnob phaserRate, phaserDepth, phaserFeedback, phaserMix;
    std::unique_ptr<ToggleButton> compSidechain;
    RotaryKnob compThreshold, compRatio, compAttack, compRelease;
    RotaryKnob compKnee, compMakeup, compScAmount;

    // FASE 10: secuenciador de pasos
    juce::OwnedArray<SeqStepControl> seqSteps;

    juce::TextButton seqOnOffBtn;
    juce::TextButton seqResetBtn;
    juce::ComboBox   seqRateCombo;
    juce::ComboBox   seqDirCombo;
    juce::Slider     seqSwingSlider;
    juce::Slider     seqLengthSlider;
    juce::Slider     seqBaseNoteSlider;

    juce::MidiKeyboardComponent keyboardComponent;
    juce::Slider pitchWheelSlider;
    juce::Slider modWheelSlider;
    juce::Label  pitchWheelLabel, modWheelLabel;

    juce::Rectangle<int> osc1Area, osc2Area, filterArea;
    juce::Rectangle<int> envArea;
    juce::Rectangle<int> lfoArea, modArea;
    juce::Rectangle<int> seqReservedArea;
    juce::Rectangle<int> fxArea;
    juce::Rectangle<int> headerLogoArea;
    juce::Rectangle<int> keyboardArea, wheelsArea;
    juce::Rectangle<int> fxColumnAreas[8];

    float currentScale = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PPGWave3Editor)
};
