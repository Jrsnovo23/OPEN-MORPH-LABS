#include "ParameterLayout.h"
#include "../ParameterIDs.h"

namespace Params
{
    juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        auto makeChoice = [&] (const juce::String& id, const juce::String& name,
                               const juce::StringArray& items, int def)
        {
            params.push_back (std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID { id, 1 }, name, items, def));
        };

        auto makeFloat = [&] (const juce::String& id, const juce::String& name,
                              float lo, float hi, float def,
                              const juce::String& suffix = {})
        {
            params.push_back (std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { id, 1 }, name,
                juce::NormalisableRange<float> { lo, hi }, def,
                juce::AudioParameterFloatAttributes().withLabel (suffix)));
        };

        auto makeFloatLog = [&] (const juce::String& id, const juce::String& name,
                                 float lo, float hi, float def,
                                 const juce::String& suffix = {})
        {
            juce::NormalisableRange<float> range (lo, hi);
            range.setSkewForCentre (std::sqrt (lo * hi));
            params.push_back (std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { id, 1 }, name, range, def,
                juce::AudioParameterFloatAttributes().withLabel (suffix)));
        };

        auto makeBool = [&] (const juce::String& id, const juce::String& name, bool def)
        {
            params.push_back (std::make_unique<juce::AudioParameterBool>(
                juce::ParameterID { id, 1 }, name, def));
        };

        // ============ Listas ============
        const juce::StringArray waveChoices   { "Sine", "Triangle", "Saw", "Square" };
        const juce::StringArray octaveChoices { "-2", "-1", "0", "+1", "+2" };
        const juce::StringArray filterTypes   { "Low Pass", "High Pass", "Band Pass" };

        const juce::StringArray lfoWaves {
            "Sine", "Triangle", "Square", "Saw Up", "Saw Down", "Random", "Sample & Hold"
        };

        const juce::StringArray syncDivisions {
            "Free", "1/1", "1/2", "1/4", "1/8", "1/16",
            "1/4T", "1/8T", "1/16T", "1/4.", "1/8."
        };

        const juce::StringArray modSources {
            "None", "LFO 1", "LFO 2", "Envelope 1", "Envelope 2",
            "Velocity", "Mod Wheel", "Aftertouch", "Note Number", "Random",
            "Envelope 3"
        };

        const juce::StringArray modDests {
            "None", "OSC1 Pitch", "OSC2 Pitch", "OSC1 Wave Pos", "OSC2 Wave Pos",
            "Filter Cutoff", "Amplifier", "Filter Reso", "OSC2 Fine", "OSC1 Fine"
        };

        // ============ Osciladores ============
        makeChoice (ParamIDs::osc1Wave,   "Osc1 Wave",   waveChoices,   0);
        makeFloat  (ParamIDs::osc1Pos,    "Osc1 Pos",    0.0f, 1.0f, 0.0f);
        makeChoice (ParamIDs::osc1Octave, "Osc1 Octave", octaveChoices, 2);
        makeFloat  (ParamIDs::osc1Semi,   "Osc1 Semi",  -12.0f, 12.0f, 0.0f, "st");
        makeFloat  (ParamIDs::osc1Fine,   "Osc1 Fine", -100.0f, 100.0f, 0.0f, "ct");
        makeFloat  (ParamIDs::osc1Level,  "Osc1 Level",  0.0f, 1.0f, 0.8f);

        makeChoice (ParamIDs::osc2Wave,   "Osc2 Wave",   waveChoices,   2);
        makeFloat  (ParamIDs::osc2Pos,    "Osc2 Pos",    0.0f, 1.0f, 0.0f);
        makeChoice (ParamIDs::osc2Octave, "Osc2 Octave", octaveChoices, 2);
        makeFloat  (ParamIDs::osc2Semi,   "Osc2 Semi",  -12.0f, 12.0f, 0.0f, "st");
        makeFloat  (ParamIDs::osc2Fine,   "Osc2 Fine", -100.0f, 100.0f, 0.0f, "ct");
        makeFloat  (ParamIDs::osc2Level,  "Osc2 Level",  0.0f, 1.0f, 0.0f);

        // ============ Envelopes ============
        makeFloat (ParamIDs::ampAttack,  "Amp Attack",  0.001f, 5.0f, 0.005f, "s");
        makeFloat (ParamIDs::ampDecay,   "Amp Decay",   0.001f, 5.0f, 0.100f, "s");
        makeFloat (ParamIDs::ampSustain, "Amp Sustain", 0.0f,   1.0f, 0.700f);
        makeFloat (ParamIDs::ampRelease, "Amp Release", 0.001f, 10.0f, 0.300f, "s");

        makeFloat (ParamIDs::filtAttack,  "Filt Attack",  0.001f, 5.0f, 0.005f, "s");
        makeFloat (ParamIDs::filtDecay,   "Filt Decay",   0.001f, 5.0f, 0.200f, "s");
        makeFloat (ParamIDs::filtSustain, "Filt Sustain", 0.0f,   1.0f, 0.500f);
        makeFloat (ParamIDs::filtRelease, "Filt Release", 0.001f, 10.0f, 0.300f, "s");

        makeFloat (ParamIDs::env3Attack,  "Env3 Attack",  0.001f, 5.0f, 0.005f, "s");
        makeFloat (ParamIDs::env3Decay,   "Env3 Decay",   0.001f, 5.0f, 0.200f, "s");
        makeFloat (ParamIDs::env3Sustain, "Env3 Sustain", 0.0f,   1.0f, 0.500f);
        makeFloat (ParamIDs::env3Release, "Env3 Release", 0.001f, 10.0f, 0.300f, "s");

        // ============ Filtro ============
        makeChoice   (ParamIDs::filterType,     "Filter Type",     filterTypes, 0);
        makeFloatLog (ParamIDs::filterCutoff,   "Filter Cutoff",   20.0f, 20000.0f, 8000.0f, "Hz");
        makeFloat    (ParamIDs::filterReso,     "Filter Reso",     0.0f, 1.0f, 0.0f);
        makeFloat    (ParamIDs::filterEnvAmt,   "Filter Env Amt", -1.0f, 1.0f, 0.0f);
        makeFloat    (ParamIDs::filterKeyTrack, "Filter Key Track", 0.0f, 1.0f, 0.0f);

        // ============ Master ============
        makeFloat (ParamIDs::masterGain, "Master", 0.0f, 1.0f, 0.7f);

        // ============ LFOs ============
        makeChoice   (ParamIDs::lfo1Wave,  "LFO1 Wave",  lfoWaves, 0);
        makeFloatLog (ParamIDs::lfo1Rate,  "LFO1 Rate",  0.01f, 40.0f, 1.0f, "Hz");
        makeFloat    (ParamIDs::lfo1Depth, "LFO1 Depth", 0.0f, 1.0f, 0.0f);
        makeFloat    (ParamIDs::lfo1Phase, "LFO1 Phase", 0.0f, 1.0f, 0.0f);
        makeChoice   (ParamIDs::lfo1Sync,  "LFO1 Sync",  syncDivisions, 0);

        makeChoice   (ParamIDs::lfo2Wave,  "LFO2 Wave",  lfoWaves, 0);
        makeFloatLog (ParamIDs::lfo2Rate,  "LFO2 Rate",  0.01f, 40.0f, 1.0f, "Hz");
        makeFloat    (ParamIDs::lfo2Depth, "LFO2 Depth", 0.0f, 1.0f, 0.0f);
        makeFloat    (ParamIDs::lfo2Phase, "LFO2 Phase", 0.0f, 1.0f, 0.0f);
        makeChoice   (ParamIDs::lfo2Sync,  "LFO2 Sync",  syncDivisions, 0);

        // ============ Matriz ============
        auto makeModSlot = [&] (const juce::String& srcId, const juce::String& dstId,
                                const juce::String& amtId, const juce::String& num)
        {
            makeChoice (srcId, "Mod" + num + " Src",  modSources, 0);
            makeChoice (dstId, "Mod" + num + " Dest", modDests,   0);
            makeFloat  (amtId, "Mod" + num + " Amt", -1.0f, 1.0f, 0.0f);
        };

        makeModSlot (ParamIDs::mod1Source, ParamIDs::mod1Dest, ParamIDs::mod1Amount, "1");
        makeModSlot (ParamIDs::mod2Source, ParamIDs::mod2Dest, ParamIDs::mod2Amount, "2");
        makeModSlot (ParamIDs::mod3Source, ParamIDs::mod3Dest, ParamIDs::mod3Amount, "3");
        makeModSlot (ParamIDs::mod4Source, ParamIDs::mod4Dest, ParamIDs::mod4Amount, "4");

        // ============ Efectos ============
        makeBool  (ParamIDs::driveOn,     "Drive On",     false);
        makeFloat (ParamIDs::driveAmount, "Drive",         1.0f, 20.0f, 3.0f);
        makeFloat (ParamIDs::driveTone,   "Drive Tone",    0.0f,  1.0f, 0.5f);
        makeFloat (ParamIDs::driveMix,    "Drive Mix",     0.0f,  1.0f, 0.5f);

        makeBool  (ParamIDs::chorusOn,    "Chorus On",   false);
        makeFloat (ParamIDs::chorusRate,  "Chorus Rate",  0.05f, 5.0f, 0.5f, "Hz");
        makeFloat (ParamIDs::chorusDepth, "Chorus Depth", 0.0f,  1.0f, 0.25f);
        makeFloat (ParamIDs::chorusMix,   "Chorus Mix",   0.0f,  1.0f, 0.5f);

        makeBool  (ParamIDs::delayOn,       "Delay On",     false);
        makeChoice(ParamIDs::delaySync,     "Delay Sync",   syncDivisions, 3);
        makeFloat (ParamIDs::delayTime,     "Delay Time",   0.01f, 2.0f, 0.3f, "s");
        makeFloat (ParamIDs::delayFeedback, "Delay Feedbk", 0.0f, 0.95f, 0.4f);
        makeFloat (ParamIDs::delayMix,      "Delay Mix",    0.0f,  1.0f, 0.3f);

        makeBool  (ParamIDs::reverbOn,   "Reverb On",   false);
        makeFloat (ParamIDs::reverbSize, "Reverb Size", 0.0f, 1.0f, 0.6f);
        makeFloat (ParamIDs::reverbDamp, "Reverb Damp", 0.0f, 1.0f, 0.5f);
        makeFloat (ParamIDs::reverbMix,  "Reverb Mix",  0.0f, 1.0f, 0.3f);

        // ============ EQ ============
        makeBool (ParamIDs::eqOn,   "EQ On",    true);
        makeBool (ParamIDs::eqHpOn, "EQ HP On", false);
        makeBool (ParamIDs::eqLpOn, "EQ LP On", false);

        makeFloatLog (ParamIDs::eqLowFreq, "EQ Low Freq",  30.0f, 500.0f,   100.0f, "Hz");
        makeFloat    (ParamIDs::eqLowQ,    "EQ Low Q",     0.1f,  10.0f,    0.707f);
        makeFloat    (ParamIDs::eqLowGain, "EQ Low Gain", -18.0f, 18.0f,    0.0f, "dB");

        makeFloatLog (ParamIDs::eqLmidFreq, "EQ LMid Freq", 100.0f, 2000.0f, 500.0f, "Hz");
        makeFloat    (ParamIDs::eqLmidQ,    "EQ LMid Q",    0.1f,  10.0f,    0.707f);
        makeFloat    (ParamIDs::eqLmidGain, "EQ LMid Gain",-18.0f, 18.0f,    0.0f, "dB");

        makeFloatLog (ParamIDs::eqHmidFreq, "EQ HMid Freq", 1000.0f, 8000.0f, 2000.0f, "Hz");
        makeFloat    (ParamIDs::eqHmidQ,    "EQ HMid Q",    0.1f,  10.0f,    0.707f);
        makeFloat    (ParamIDs::eqHmidGain, "EQ HMid Gain",-18.0f, 18.0f,    0.0f, "dB");

        makeFloatLog (ParamIDs::eqHighFreq, "EQ High Freq", 2000.0f, 20000.0f, 8000.0f, "Hz");
        makeFloat    (ParamIDs::eqHighQ,    "EQ High Q",    0.1f,  10.0f,    0.707f);
        makeFloat    (ParamIDs::eqHighGain, "EQ High Gain",-18.0f, 18.0f,    0.0f, "dB");

        // ============ Phaser ============
        makeBool  (ParamIDs::phaserOn,       "Phaser On",       false);
        makeFloat (ParamIDs::phaserRate,     "Phaser Rate",     0.05f, 5.0f, 0.5f, "Hz");
        makeFloat (ParamIDs::phaserDepth,    "Phaser Depth",    0.0f,  1.0f, 0.5f);
        makeFloat (ParamIDs::phaserFeedback, "Phaser Feedbk",   0.0f,  0.95f, 0.5f);
        makeFloat (ParamIDs::phaserMix,      "Phaser Mix",      0.0f,  1.0f, 0.5f);

        // ============ Vintage ============
        makeBool  (ParamIDs::vintageOn,     "Vintage On",     false);
        makeFloat (ParamIDs::vintageAmount, "Vintage Amount", 0.0f, 1.0f, 0.5f);
        makeFloat (ParamIDs::vintageBits,   "Vintage Bits",   4.0f, 16.0f, 12.0f, "bit");
        makeFloat (ParamIDs::vintageSr,     "Vintage SR",     1.0f, 32.0f, 1.0f, "x");
        makeFloat (ParamIDs::vintageNoise,  "Vintage Noise",  0.0f, 1.0f, 0.15f);
        makeFloat (ParamIDs::vintageDrift,  "Vintage Drift",  0.0f, 1.0f, 0.3f);
        makeFloat (ParamIDs::vintageVar,    "Vintage Var",    0.0f, 1.0f, 0.3f);

        // ============ FASE 11: Compresor ============
        makeBool     (ParamIDs::compOn,        "Comp On",       false);
        makeFloat    (ParamIDs::compThreshold, "Comp Threshold", -60.0f, 0.0f, -12.0f, "dB");
        makeFloat    (ParamIDs::compRatio,     "Comp Ratio",     1.0f, 20.0f, 4.0f, ":1");
        makeFloatLog (ParamIDs::compAttack,    "Comp Attack",    0.1f, 100.0f, 10.0f, "ms");
        makeFloatLog (ParamIDs::compRelease,   "Comp Release",   10.0f, 1000.0f, 100.0f, "ms");
        makeFloat    (ParamIDs::compKnee,      "Comp Knee",      0.0f, 24.0f, 6.0f, "dB");
        makeFloat    (ParamIDs::compMakeup,    "Comp Makeup",    0.0f, 24.0f, 0.0f, "dB");
        makeBool     (ParamIDs::compSidechain, "Comp Sidechain", false);
        makeFloat    (ParamIDs::compScAmount,  "Comp SC Amount", 0.0f, 1.0f, 1.0f);

        return { params.begin(), params.end() };
    }
}
