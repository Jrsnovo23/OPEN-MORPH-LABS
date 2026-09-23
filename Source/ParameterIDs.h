#pragma once
#include <juce_core/juce_core.h>

namespace ParamIDs
{
    // --- Oscilador 1 ---
    inline constexpr auto osc1Wave   = "osc1_wave";
    inline constexpr auto osc1Pos    = "osc1_pos";
    inline constexpr auto osc1Octave = "osc1_octave";
    inline constexpr auto osc1Semi   = "osc1_semi";
    inline constexpr auto osc1Fine   = "osc1_fine";
    inline constexpr auto osc1Level  = "osc1_level";

    // --- Oscilador 2 ---
    inline constexpr auto osc2Wave   = "osc2_wave";
    inline constexpr auto osc2Pos    = "osc2_pos";
    inline constexpr auto osc2Octave = "osc2_octave";
    inline constexpr auto osc2Semi   = "osc2_semi";
    inline constexpr auto osc2Fine   = "osc2_fine";
    inline constexpr auto osc2Level  = "osc2_level";

    // --- Amp Envelope (ENV1) ---
    inline constexpr auto ampAttack  = "amp_attack";
    inline constexpr auto ampDecay   = "amp_decay";
    inline constexpr auto ampSustain = "amp_sustain";
    inline constexpr auto ampRelease = "amp_release";

    // --- Filter ---
    inline constexpr auto filterType     = "filter_type";
    inline constexpr auto filterCutoff   = "filter_cutoff";
    inline constexpr auto filterReso     = "filter_reso";
    inline constexpr auto filterEnvAmt   = "filter_env_amt";
    inline constexpr auto filterKeyTrack = "filter_key_track";

    // --- Filter Envelope (ENV2) ---
    inline constexpr auto filtAttack  = "filt_attack";
    inline constexpr auto filtDecay   = "filt_decay";
    inline constexpr auto filtSustain = "filt_sustain";
    inline constexpr auto filtRelease = "filt_release";

    // --- Envelope 3 (ENV3, libre) ---
    inline constexpr auto env3Attack  = "env3_attack";
    inline constexpr auto env3Decay   = "env3_decay";
    inline constexpr auto env3Sustain = "env3_sustain";
    inline constexpr auto env3Release = "env3_release";

    // --- Master ---
    inline constexpr auto masterGain = "master_gain";

    // --- LFO 1 ---
    inline constexpr auto lfo1Wave  = "lfo1_wave";
    inline constexpr auto lfo1Rate  = "lfo1_rate";
    inline constexpr auto lfo1Depth = "lfo1_depth";
    inline constexpr auto lfo1Phase = "lfo1_phase";
    inline constexpr auto lfo1Sync  = "lfo1_sync";

    // --- LFO 2 ---
    inline constexpr auto lfo2Wave  = "lfo2_wave";
    inline constexpr auto lfo2Rate  = "lfo2_rate";
    inline constexpr auto lfo2Depth = "lfo2_depth";
    inline constexpr auto lfo2Phase = "lfo2_phase";
    inline constexpr auto lfo2Sync  = "lfo2_sync";

    // --- Matriz de Modulación ---
    inline constexpr auto mod1Source = "mod1_source";
    inline constexpr auto mod1Dest   = "mod1_dest";
    inline constexpr auto mod1Amount = "mod1_amount";

    inline constexpr auto mod2Source = "mod2_source";
    inline constexpr auto mod2Dest   = "mod2_dest";
    inline constexpr auto mod2Amount = "mod2_amount";

    inline constexpr auto mod3Source = "mod3_source";
    inline constexpr auto mod3Dest   = "mod3_dest";
    inline constexpr auto mod3Amount = "mod3_amount";

    inline constexpr auto mod4Source = "mod4_source";
    inline constexpr auto mod4Dest   = "mod4_dest";
    inline constexpr auto mod4Amount = "mod4_amount";

    // ============ EFECTOS ============
    inline constexpr auto driveOn     = "drive_on";
    inline constexpr auto driveAmount = "drive_amount";
    inline constexpr auto driveTone   = "drive_tone";
    inline constexpr auto driveMix    = "drive_mix";

    inline constexpr auto chorusOn    = "chorus_on";
    inline constexpr auto chorusRate  = "chorus_rate";
    inline constexpr auto chorusDepth = "chorus_depth";
    inline constexpr auto chorusMix   = "chorus_mix";

    inline constexpr auto delayOn       = "delay_on";
    inline constexpr auto delaySync     = "delay_sync";
    inline constexpr auto delayTime     = "delay_time";
    inline constexpr auto delayFeedback = "delay_feedback";
    inline constexpr auto delayMix      = "delay_mix";

    inline constexpr auto reverbOn    = "reverb_on";
    inline constexpr auto reverbSize  = "reverb_size";
    inline constexpr auto reverbDamp  = "reverb_damp";
    inline constexpr auto reverbMix   = "reverb_mix";

    // ============ EQ 4 BANDAS ============
    inline constexpr auto eqOn   = "eq_on";
    inline constexpr auto eqHpOn = "eq_hp_on";
    inline constexpr auto eqLpOn = "eq_lp_on";

    inline constexpr auto eqLowFreq = "eq_low_freq";
    inline constexpr auto eqLowQ    = "eq_low_q";
    inline constexpr auto eqLowGain = "eq_low_gain";

    inline constexpr auto eqLmidFreq = "eq_lmid_freq";
    inline constexpr auto eqLmidQ    = "eq_lmid_q";
    inline constexpr auto eqLmidGain = "eq_lmid_gain";

    inline constexpr auto eqHmidFreq = "eq_hmid_freq";
    inline constexpr auto eqHmidQ    = "eq_hmid_q";
    inline constexpr auto eqHmidGain = "eq_hmid_gain";

    inline constexpr auto eqHighFreq = "eq_high_freq";
    inline constexpr auto eqHighQ    = "eq_high_q";
    inline constexpr auto eqHighGain = "eq_high_gain";

    // ============ PHASER ============
    inline constexpr auto phaserOn       = "phaser_on";
    inline constexpr auto phaserRate     = "phaser_rate";
    inline constexpr auto phaserDepth    = "phaser_depth";
    inline constexpr auto phaserFeedback = "phaser_feedback";
    inline constexpr auto phaserMix      = "phaser_mix";

    // ============ VINTAGE CHARACTER ============
    inline constexpr auto vintageOn     = "vintage_on";
    inline constexpr auto vintageAmount = "vintage_amount";
    inline constexpr auto vintageBits   = "vintage_bits";
    inline constexpr auto vintageSr     = "vintage_sr";
    inline constexpr auto vintageNoise  = "vintage_noise";
    inline constexpr auto vintageDrift  = "vintage_drift";
    inline constexpr auto vintageVar    = "vintage_var";

    // ============ FASE 11: COMPRESOR ============
    inline constexpr auto compOn        = "comp_on";
    inline constexpr auto compThreshold = "comp_threshold";
    inline constexpr auto compRatio     = "comp_ratio";
    inline constexpr auto compAttack    = "comp_attack";
    inline constexpr auto compRelease   = "comp_release";
    inline constexpr auto compKnee      = "comp_knee";
    inline constexpr auto compMakeup    = "comp_makeup";
    inline constexpr auto compSidechain = "comp_sidechain";
    inline constexpr auto compScAmount  = "comp_sc_amount";
}
