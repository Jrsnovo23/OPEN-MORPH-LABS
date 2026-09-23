#include "PresetManager.h"

namespace presets
{
    // ============ Factory presets (JSON) ============
    static const char* kFactoryJSON[] =
    {
        // INIT
        R"({"name":"Init Saw","category":"Init","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.0,"osc1_octave":2,"osc1_level":0.8,"osc2_wave":0,"osc2_level":0.0,"filter_type":0,"filter_cutoff":8000,"filter_reso":0.0,"filter_env_amt":0.0,"amp_attack":0.005,"amp_decay":0.1,"amp_sustain":0.7,"amp_release":0.3,"filt_attack":0.005,"filt_decay":0.2,"filt_sustain":0.5,"filt_release":0.3,"master_gain":0.7}})",

        // BASS
        R"({"name":"Deep Sub Bass","category":"Bass","author":"Factory","params":{"osc1_wave":0,"osc1_octave":0,"osc1_level":0.95,"osc2_wave":0,"osc2_octave":1,"osc2_level":0.2,"filter_type":0,"filter_cutoff":500,"filter_reso":0.1,"amp_attack":0.005,"amp_decay":0.3,"amp_sustain":0.9,"amp_release":0.2,"master_gain":0.75}})",
        R"({"name":"Acid Pulse","category":"Bass","author":"Factory","params":{"osc1_wave":3,"osc1_octave":1,"osc1_level":0.9,"filter_type":0,"filter_cutoff":700,"filter_reso":0.75,"filter_env_amt":0.6,"amp_attack":0.001,"amp_decay":0.25,"amp_sustain":0.3,"amp_release":0.1,"filt_attack":0.001,"filt_decay":0.2,"filt_sustain":0.0,"filt_release":0.1,"drive_on":1,"drive_amount":5.0,"drive_tone":0.5,"drive_mix":0.4,"master_gain":0.6}})",
        R"({"name":"Digital Bass","category":"Bass","author":"Factory","params":{"osc1_wave":2,"osc1_octave":1,"osc1_level":0.85,"osc2_wave":3,"osc2_octave":1,"osc2_semi":-12,"osc2_level":0.5,"filter_type":0,"filter_cutoff":1200,"filter_reso":0.35,"filter_env_amt":0.4,"amp_attack":0.001,"amp_decay":0.4,"amp_sustain":0.6,"amp_release":0.15,"filt_attack":0.001,"filt_decay":0.3,"filt_sustain":0.2,"filt_release":0.15,"drive_on":1,"drive_amount":3.0,"drive_tone":0.5,"drive_mix":0.35,"master_gain":0.65}})",
        R"({"name":"Wobble Bass","category":"Bass","author":"Factory","params":{"osc1_wave":2,"osc1_octave":1,"osc1_level":0.9,"osc2_wave":3,"osc2_octave":1,"osc2_level":0.4,"filter_type":0,"filter_cutoff":900,"filter_reso":0.7,"amp_attack":0.005,"amp_decay":0.3,"amp_sustain":0.85,"amp_release":0.2,"lfo1_wave":0,"lfo1_rate":2.5,"lfo1_depth":1.0,"lfo1_sync":4,"mod1_source":1,"mod1_dest":5,"mod1_amount":0.7,"drive_on":1,"drive_amount":4.0,"drive_tone":0.4,"drive_mix":0.4,"master_gain":0.6}})",
        R"({"name":"FM Bass","category":"Bass","author":"Factory","params":{"osc1_wave":2,"osc1_octave":1,"osc1_level":0.8,"osc2_wave":0,"osc2_octave":3,"osc2_level":0.6,"filter_type":0,"filter_cutoff":1400,"filter_reso":0.3,"filter_env_amt":0.5,"amp_attack":0.001,"amp_decay":0.3,"amp_sustain":0.4,"amp_release":0.15,"filt_attack":0.001,"filt_decay":0.2,"filt_sustain":0.1,"filt_release":0.1,"master_gain":0.65}})",
        R"({"name":"Reese Bass","category":"Bass","author":"Factory","params":{"osc1_wave":2,"osc1_octave":1,"osc1_level":0.7,"osc1_fine":-12.0,"osc2_wave":2,"osc2_octave":1,"osc2_level":0.7,"osc2_fine":12.0,"filter_type":0,"filter_cutoff":800,"filter_reso":0.3,"amp_attack":0.005,"amp_decay":0.5,"amp_sustain":0.95,"amp_release":0.3,"lfo1_wave":0,"lfo1_rate":0.3,"lfo1_depth":0.5,"mod1_source":1,"mod1_dest":1,"mod1_amount":0.05,"drive_on":1,"drive_amount":3.0,"drive_tone":0.5,"drive_mix":0.3,"master_gain":0.65}})",
        R"({"name":"Sub Punch","category":"Bass","author":"Factory","params":{"osc1_wave":0,"osc1_octave":0,"osc1_level":0.95,"osc2_wave":0,"osc2_octave":2,"osc2_level":0.4,"filter_type":0,"filter_cutoff":300,"filter_reso":0.15,"filter_env_amt":0.6,"amp_attack":0.001,"amp_decay":0.5,"amp_sustain":0.2,"amp_release":0.15,"filt_attack":0.001,"filt_decay":0.15,"filt_sustain":0.0,"filt_release":0.1,"master_gain":0.75}})",
        R"({"name":"Tech House Bass","category":"Bass","author":"Factory","params":{"osc1_wave":3,"osc1_octave":1,"osc1_level":0.85,"filter_type":0,"filter_cutoff":1200,"filter_reso":0.55,"filter_env_amt":0.5,"amp_attack":0.001,"amp_decay":0.25,"amp_sustain":0.3,"amp_release":0.12,"filt_attack":0.001,"filt_decay":0.18,"filt_sustain":0.1,"filt_release":0.1,"drive_on":1,"drive_amount":4.0,"drive_tone":0.55,"drive_mix":0.35,"master_gain":0.6}})",
        R"({"name":"Square Bass","category":"Bass","author":"Factory","params":{"osc1_wave":3,"osc1_octave":0,"osc1_level":0.9,"osc2_wave":0,"osc2_octave":0,"osc2_level":0.25,"filter_type":0,"filter_cutoff":700,"filter_reso":0.25,"amp_attack":0.005,"amp_decay":0.4,"amp_sustain":0.85,"amp_release":0.2,"master_gain":0.7}})",
        R"({"name":"Grit Bass","category":"Bass","author":"Factory","params":{"osc1_wave":2,"osc1_octave":1,"osc1_level":0.8,"osc2_wave":2,"osc2_octave":1,"osc2_semi":-5,"osc2_level":0.6,"filter_type":0,"filter_cutoff":1000,"filter_reso":0.6,"filter_env_amt":0.5,"amp_attack":0.001,"amp_decay":0.35,"amp_sustain":0.5,"amp_release":0.2,"drive_on":1,"drive_amount":8.0,"drive_tone":0.5,"drive_mix":0.55,"master_gain":0.55}})",

        // PAD
        R"({"name":"Warm Analog Pad","category":"Pad","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.2,"osc1_level":0.55,"osc1_fine":-5.0,"osc2_wave":2,"osc2_pos":0.4,"osc2_level":0.55,"osc2_fine":5.0,"filter_type":0,"filter_cutoff":3200,"filter_reso":0.15,"amp_attack":0.7,"amp_decay":1.2,"amp_sustain":0.8,"amp_release":1.8,"chorus_on":1,"chorus_rate":0.35,"chorus_depth":0.5,"chorus_mix":0.45,"reverb_on":1,"reverb_size":0.7,"reverb_damp":0.5,"reverb_mix":0.35,"master_gain":0.6}})",
        R"({"name":"Crystal Pad","category":"Pad","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.6,"osc1_level":0.6,"osc2_wave":3,"osc2_pos":0.3,"osc2_semi":7,"osc2_level":0.5,"filter_type":0,"filter_cutoff":5500,"filter_reso":0.25,"amp_attack":0.9,"amp_decay":1.5,"amp_sustain":0.75,"amp_release":2.2,"reverb_on":1,"reverb_size":0.85,"reverb_damp":0.4,"reverb_mix":0.5,"master_gain":0.55}})",
        R"({"name":"Motion Pad","category":"Pad","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.0,"osc1_level":0.65,"osc2_wave":3,"osc2_pos":0.0,"osc2_semi":-5,"osc2_level":0.5,"filter_type":0,"filter_cutoff":2000,"filter_reso":0.4,"amp_attack":1.0,"amp_decay":2.0,"amp_sustain":0.8,"amp_release":2.5,"lfo1_wave":0,"lfo1_rate":0.2,"lfo1_depth":0.85,"mod1_source":1,"mod1_dest":3,"mod1_amount":0.6,"mod2_source":2,"mod2_dest":4,"mod2_amount":0.5,"reverb_on":1,"reverb_size":0.75,"reverb_damp":0.4,"reverb_mix":0.4,"master_gain":0.55}})",
        R"({"name":"Dark Pad","category":"Pad","author":"Factory","params":{"osc1_wave":2,"osc1_octave":1,"osc1_level":0.65,"osc2_wave":3,"osc2_octave":1,"osc2_semi":-7,"osc2_level":0.5,"filter_type":0,"filter_cutoff":900,"filter_reso":0.35,"amp_attack":1.5,"amp_decay":2.0,"amp_sustain":0.85,"amp_release":3.0,"reverb_on":1,"reverb_size":0.8,"reverb_damp":0.6,"reverb_mix":0.45,"master_gain":0.55}})",
        R"({"name":"String Pad","category":"Pad","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.1,"osc1_level":0.55,"osc1_fine":-3.0,"osc2_wave":2,"osc2_pos":0.1,"osc2_level":0.55,"osc2_fine":3.0,"filter_type":0,"filter_cutoff":4200,"filter_reso":0.15,"amp_attack":0.6,"amp_decay":1.0,"amp_sustain":0.85,"amp_release":1.5,"chorus_on":1,"chorus_rate":0.5,"chorus_depth":0.55,"chorus_mix":0.5,"reverb_on":1,"reverb_size":0.65,"reverb_damp":0.5,"reverb_mix":0.35,"master_gain":0.6}})",
        R"({"name":"Glass Pad","category":"Pad","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.7,"osc1_level":0.55,"osc2_wave":3,"osc2_pos":0.4,"osc2_semi":12,"osc2_level":0.45,"filter_type":0,"filter_cutoff":6500,"filter_reso":0.3,"amp_attack":0.8,"amp_decay":1.6,"amp_sustain":0.7,"amp_release":2.5,"reverb_on":1,"reverb_size":0.85,"reverb_damp":0.35,"reverb_mix":0.55,"delay_on":1,"delay_sync":4,"delay_feedback":0.4,"delay_mix":0.3,"master_gain":0.55}})",
        R"({"name":"Vocal Pad","category":"Pad","author":"Factory","params":{"osc1_wave":0,"osc1_level":0.6,"osc2_wave":1,"osc2_semi":7,"osc2_level":0.5,"filter_type":2,"filter_cutoff":1500,"filter_reso":0.55,"amp_attack":0.9,"amp_decay":1.4,"amp_sustain":0.8,"amp_release":2.0,"lfo1_wave":0,"lfo1_rate":5.0,"lfo1_depth":0.3,"mod1_source":1,"mod1_dest":1,"mod1_amount":0.08,"chorus_on":1,"chorus_rate":0.4,"chorus_depth":0.5,"chorus_mix":0.45,"reverb_on":1,"reverb_size":0.75,"reverb_damp":0.5,"reverb_mix":0.45,"master_gain":0.6}})",
        R"({"name":"Deep Warm Pad","category":"Pad","author":"Factory","params":{"osc1_wave":2,"osc1_octave":0,"osc1_level":0.6,"osc2_wave":1,"osc2_octave":1,"osc2_level":0.4,"filter_type":0,"filter_cutoff":1200,"filter_reso":0.25,"amp_attack":1.5,"amp_decay":2.0,"amp_sustain":0.85,"amp_release":3.5,"reverb_on":1,"reverb_size":0.85,"reverb_damp":0.5,"reverb_mix":0.5,"master_gain":0.6}})",
        R"({"name":"Analog Strings","category":"Pad","author":"Factory","params":{"osc1_wave":2,"osc1_fine":-8.0,"osc1_level":0.5,"osc2_wave":2,"osc2_fine":8.0,"osc2_level":0.5,"filter_type":0,"filter_cutoff":3800,"filter_reso":0.2,"amp_attack":0.7,"amp_decay":1.2,"amp_sustain":0.85,"amp_release":1.8,"chorus_on":1,"chorus_rate":0.45,"chorus_depth":0.6,"chorus_mix":0.5,"reverb_on":1,"reverb_size":0.6,"reverb_damp":0.5,"reverb_mix":0.35,"master_gain":0.6}})",
        R"({"name":"Shimmer Pad","category":"Pad","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.5,"osc1_level":0.55,"osc2_wave":3,"osc2_pos":0.8,"osc2_semi":19,"osc2_level":0.4,"filter_type":0,"filter_cutoff":7000,"filter_reso":0.25,"amp_attack":1.2,"amp_decay":2.0,"amp_sustain":0.75,"amp_release":3.0,"reverb_on":1,"reverb_size":0.9,"reverb_damp":0.3,"reverb_mix":0.55,"delay_on":1,"delay_sync":3,"delay_feedback":0.5,"delay_mix":0.3,"master_gain":0.5}})",

        // LEAD
        R"({"name":"Digital Lead","category":"Lead","author":"Factory","params":{"osc1_wave":2,"osc1_level":0.75,"osc2_wave":3,"osc2_semi":12,"osc2_level":0.4,"filter_type":0,"filter_cutoff":7500,"filter_reso":0.35,"amp_attack":0.01,"amp_decay":0.2,"amp_sustain":0.85,"amp_release":0.3,"drive_on":1,"drive_amount":2.5,"drive_tone":0.6,"drive_mix":0.3,"delay_on":1,"delay_sync":4,"delay_feedback":0.35,"delay_mix":0.3,"master_gain":0.65}})",
        R"({"name":"Sync Lead","category":"Lead","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.3,"osc1_level":0.7,"osc2_wave":3,"osc2_semi":7,"osc2_level":0.5,"filter_type":0,"filter_cutoff":6000,"filter_reso":0.45,"amp_attack":0.005,"amp_decay":0.15,"amp_sustain":0.8,"amp_release":0.25,"delay_on":1,"delay_sync":4,"delay_feedback":0.4,"delay_mix":0.35,"master_gain":0.65}})",
        R"({"name":"Solo Lead","category":"Lead","author":"Factory","params":{"osc1_wave":2,"osc1_level":0.75,"osc2_wave":2,"osc2_fine":6.0,"osc2_level":0.5,"filter_type":0,"filter_cutoff":5200,"filter_reso":0.3,"amp_attack":0.02,"amp_decay":0.25,"amp_sustain":0.85,"amp_release":0.4,"lfo1_wave":0,"lfo1_rate":5.5,"lfo1_depth":0.3,"mod1_source":1,"mod1_dest":1,"mod1_amount":0.15,"reverb_on":1,"reverb_size":0.4,"reverb_damp":0.6,"reverb_mix":0.25,"master_gain":0.65}})",
        R"({"name":"Hard Lead","category":"Lead","author":"Factory","params":{"osc1_wave":2,"osc1_level":0.8,"osc2_wave":3,"osc2_semi":5,"osc2_level":0.6,"filter_type":0,"filter_cutoff":6500,"filter_reso":0.4,"amp_attack":0.005,"amp_decay":0.2,"amp_sustain":0.85,"amp_release":0.3,"drive_on":1,"drive_amount":6.0,"drive_tone":0.5,"drive_mix":0.5,"master_gain":0.55}})",
        R"({"name":"Soft Lead","category":"Lead","author":"Factory","params":{"osc1_wave":0,"osc1_level":0.7,"osc2_wave":2,"osc2_level":0.3,"filter_type":0,"filter_cutoff":4500,"filter_reso":0.15,"amp_attack":0.05,"amp_decay":0.3,"amp_sustain":0.8,"amp_release":0.5,"chorus_on":1,"chorus_rate":0.5,"chorus_depth":0.4,"chorus_mix":0.4,"reverb_on":1,"reverb_size":0.5,"reverb_damp":0.5,"reverb_mix":0.3,"master_gain":0.7}})",
        R"({"name":"Detuned Lead","category":"Lead","author":"Factory","params":{"osc1_wave":2,"osc1_fine":-14.0,"osc1_level":0.6,"osc2_wave":2,"osc2_fine":14.0,"osc2_level":0.6,"filter_type":0,"filter_cutoff":5500,"filter_reso":0.3,"amp_attack":0.01,"amp_decay":0.25,"amp_sustain":0.85,"amp_release":0.35,"delay_on":1,"delay_sync":4,"delay_feedback":0.35,"delay_mix":0.3,"master_gain":0.6}})",
        R"({"name":"Chiptune Lead","category":"Lead","author":"Factory","params":{"osc1_wave":3,"osc1_level":0.8,"osc2_wave":3,"osc2_semi":12,"osc2_level":0.4,"filter_type":0,"filter_cutoff":9000,"filter_reso":0.1,"amp_attack":0.001,"amp_decay":0.15,"amp_sustain":0.6,"amp_release":0.1,"master_gain":0.65}})",
        R"({"name":"Supersaw Lead","category":"Lead","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.15,"osc1_level":0.5,"osc1_fine":-20.0,"osc2_wave":2,"osc2_pos":0.15,"osc2_level":0.5,"osc2_fine":20.0,"filter_type":0,"filter_cutoff":6000,"filter_reso":0.25,"amp_attack":0.01,"amp_decay":0.25,"amp_sustain":0.9,"amp_release":0.4,"chorus_on":1,"chorus_rate":0.4,"chorus_depth":0.35,"chorus_mix":0.4,"delay_on":1,"delay_sync":4,"delay_feedback":0.4,"delay_mix":0.3,"master_gain":0.55}})",
        R"({"name":"Screamer Lead","category":"Lead","author":"Factory","params":{"osc1_wave":2,"osc1_level":0.8,"osc2_wave":2,"osc2_semi":7,"osc2_level":0.6,"filter_type":0,"filter_cutoff":3000,"filter_reso":0.7,"filter_env_amt":0.5,"amp_attack":0.005,"amp_decay":0.2,"amp_sustain":0.85,"amp_release":0.25,"filt_attack":0.001,"filt_decay":0.15,"filt_sustain":0.6,"filt_release":0.2,"drive_on":1,"drive_amount":8.0,"drive_tone":0.5,"drive_mix":0.5,"master_gain":0.55}})",
        R"({"name":"Flute Lead","category":"Lead","author":"Factory","params":{"osc1_wave":1,"osc1_level":0.75,"osc2_wave":0,"osc2_level":0.3,"filter_type":0,"filter_cutoff":4500,"filter_reso":0.2,"amp_attack":0.08,"amp_decay":0.3,"amp_sustain":0.85,"amp_release":0.4,"lfo1_wave":0,"lfo1_rate":6.0,"lfo1_depth":0.25,"mod1_source":1,"mod1_dest":1,"mod1_amount":0.1,"reverb_on":1,"reverb_size":0.5,"reverb_damp":0.6,"reverb_mix":0.3,"master_gain":0.7}})",

        // KEYS
        R"({"name":"Electric Piano","category":"Keys","author":"Factory","params":{"osc1_wave":0,"osc1_level":0.7,"osc2_wave":3,"osc2_semi":19,"osc2_level":0.3,"filter_type":0,"filter_cutoff":5000,"filter_reso":0.2,"filter_env_amt":0.3,"amp_attack":0.001,"amp_decay":0.9,"amp_sustain":0.15,"amp_release":0.8,"filt_attack":0.001,"filt_decay":0.5,"filt_sustain":0.1,"filt_release":0.5,"chorus_on":1,"chorus_rate":0.3,"chorus_depth":0.35,"chorus_mix":0.35,"master_gain":0.7}})",
        R"({"name":"Bell Keys","category":"Keys","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.7,"osc1_level":0.75,"osc2_wave":3,"osc2_semi":19,"osc2_level":0.4,"filter_type":0,"filter_cutoff":6000,"filter_reso":0.5,"filter_env_amt":-0.2,"amp_attack":0.001,"amp_decay":1.5,"amp_sustain":0.1,"amp_release":1.2,"reverb_on":1,"reverb_size":0.5,"reverb_damp":0.4,"reverb_mix":0.4,"master_gain":0.6}})",
        R"({"name":"Vintage Keys","category":"Keys","author":"Factory","params":{"osc1_wave":2,"osc1_level":0.55,"osc1_fine":-8.0,"osc2_wave":2,"osc2_level":0.55,"osc2_fine":8.0,"filter_type":0,"filter_cutoff":3500,"filter_reso":0.2,"amp_attack":0.005,"amp_decay":0.6,"amp_sustain":0.4,"amp_release":0.5,"chorus_on":1,"chorus_rate":0.4,"chorus_depth":0.5,"chorus_mix":0.5,"master_gain":0.65}})",
        R"({"name":"Digital Keys","category":"Keys","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.2,"osc1_level":0.7,"osc2_wave":3,"osc2_pos":0.5,"osc2_semi":12,"osc2_level":0.4,"filter_type":0,"filter_cutoff":7000,"filter_reso":0.3,"amp_attack":0.001,"amp_decay":0.5,"amp_sustain":0.2,"amp_release":0.4,"delay_on":1,"delay_sync":4,"delay_feedback":0.3,"delay_mix":0.25,"master_gain":0.65}})",
        R"({"name":"Organ Keys","category":"Keys","author":"Factory","params":{"osc1_wave":0,"osc1_level":0.5,"osc2_wave":3,"osc2_octave":3,"osc2_level":0.4,"filter_type":0,"filter_cutoff":5500,"filter_reso":0.15,"amp_attack":0.005,"amp_decay":0.05,"amp_sustain":0.95,"amp_release":0.15,"chorus_on":1,"chorus_rate":0.6,"chorus_depth":0.4,"chorus_mix":0.4,"master_gain":0.7}})",
        R"({"name":"Synth Bell","category":"Keys","author":"Factory","params":{"osc1_wave":0,"osc1_level":0.7,"osc2_wave":0,"osc2_semi":19,"osc2_level":0.5,"filter_type":0,"filter_cutoff":8000,"filter_reso":0.2,"amp_attack":0.001,"amp_decay":1.8,"amp_sustain":0.0,"amp_release":1.5,"reverb_on":1,"reverb_size":0.6,"reverb_damp":0.4,"reverb_mix":0.45,"master_gain":0.6}})",
        R"({"name":"Wurly Keys","category":"Keys","author":"Factory","params":{"osc1_wave":1,"osc1_level":0.7,"osc2_wave":0,"osc2_semi":12,"osc2_level":0.35,"filter_type":0,"filter_cutoff":4000,"filter_reso":0.25,"amp_attack":0.001,"amp_decay":0.7,"amp_sustain":0.25,"amp_release":0.6,"drive_on":1,"drive_amount":2.0,"drive_tone":0.6,"drive_mix":0.25,"chorus_on":1,"chorus_rate":0.35,"chorus_depth":0.4,"chorus_mix":0.35,"master_gain":0.7}})",
        R"({"name":"Pluck Keys","category":"Keys","author":"Factory","params":{"osc1_wave":2,"osc1_level":0.75,"osc2_wave":3,"osc2_semi":12,"osc2_level":0.3,"filter_type":0,"filter_cutoff":4500,"filter_reso":0.4,"filter_env_amt":0.5,"amp_attack":0.001,"amp_decay":0.35,"amp_sustain":0.0,"amp_release":0.25,"filt_attack":0.001,"filt_decay":0.3,"filt_sustain":0.0,"filt_release":0.2,"delay_on":1,"delay_sync":4,"delay_feedback":0.35,"delay_mix":0.25,"master_gain":0.7}})",
        R"({"name":"Marimba Keys","category":"Keys","author":"Factory","params":{"osc1_wave":0,"osc1_level":0.8,"osc2_wave":0,"osc2_semi":12,"osc2_fine":3.0,"osc2_level":0.4,"filter_type":0,"filter_cutoff":6000,"filter_reso":0.15,"amp_attack":0.001,"amp_decay":0.5,"amp_sustain":0.0,"amp_release":0.4,"reverb_on":1,"reverb_size":0.5,"reverb_damp":0.5,"reverb_mix":0.35,"master_gain":0.7}})",
        R"({"name":"Piano-ish Keys","category":"Keys","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.1,"osc1_level":0.6,"osc2_wave":0,"osc2_semi":12,"osc2_level":0.4,"filter_type":0,"filter_cutoff":5000,"filter_reso":0.15,"filter_env_amt":0.35,"amp_attack":0.001,"amp_decay":1.2,"amp_sustain":0.15,"amp_release":0.9,"filt_attack":0.001,"filt_decay":0.7,"filt_sustain":0.15,"filt_release":0.6,"chorus_on":1,"chorus_rate":0.3,"chorus_depth":0.3,"chorus_mix":0.3,"master_gain":0.7}})",

        // ARP
        R"({"name":"Arp Sequence","category":"Arp","author":"Factory","params":{"osc1_wave":2,"osc1_level":0.7,"osc2_wave":3,"osc2_semi":12,"osc2_level":0.4,"filter_type":0,"filter_cutoff":4500,"filter_reso":0.4,"filter_env_amt":0.4,"amp_attack":0.001,"amp_decay":0.18,"amp_sustain":0.0,"amp_release":0.15,"filt_attack":0.001,"filt_decay":0.15,"filt_sustain":0.0,"filt_release":0.1,"delay_on":1,"delay_sync":5,"delay_feedback":0.4,"delay_mix":0.35,"master_gain":0.65}})",
        R"({"name":"Fast Arp","category":"Arp","author":"Factory","params":{"osc1_wave":3,"osc1_level":0.75,"osc2_wave":2,"osc2_semi":7,"osc2_level":0.4,"filter_type":0,"filter_cutoff":5500,"filter_reso":0.35,"amp_attack":0.001,"amp_decay":0.1,"amp_sustain":0.0,"amp_release":0.1,"master_gain":0.7}})",
        R"({"name":"Trance Arp","category":"Arp","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.3,"osc1_level":0.7,"osc2_wave":3,"osc2_semi":12,"osc2_level":0.5,"filter_type":0,"filter_cutoff":3500,"filter_reso":0.55,"filter_env_amt":0.5,"amp_attack":0.001,"amp_decay":0.15,"amp_sustain":0.0,"amp_release":0.15,"filt_attack":0.001,"filt_decay":0.12,"filt_sustain":0.0,"filt_release":0.1,"delay_on":1,"delay_sync":4,"delay_feedback":0.5,"delay_mix":0.4,"reverb_on":1,"reverb_size":0.5,"reverb_damp":0.5,"reverb_mix":0.25,"master_gain":0.6}})",
        R"({"name":"Digital Arp","category":"Arp","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.5,"osc1_level":0.7,"osc2_wave":3,"osc2_pos":0.8,"osc2_semi":7,"osc2_level":0.5,"filter_type":0,"filter_cutoff":6000,"filter_reso":0.3,"amp_attack":0.001,"amp_decay":0.2,"amp_sustain":0.1,"amp_release":0.2,"lfo1_wave":6,"lfo1_rate":8.0,"lfo1_depth":0.5,"mod1_source":1,"mod1_dest":3,"mod1_amount":0.5,"master_gain":0.65}})",
        R"({"name":"Random Arp","category":"Arp","author":"Factory","params":{"osc1_wave":3,"osc1_level":0.7,"osc2_wave":2,"osc2_semi":12,"osc2_level":0.4,"filter_type":0,"filter_cutoff":5000,"filter_reso":0.3,"amp_attack":0.001,"amp_decay":0.15,"amp_sustain":0.0,"amp_release":0.15,"lfo1_wave":5,"lfo1_rate":6.0,"lfo1_depth":0.4,"mod1_source":1,"mod1_dest":1,"mod1_amount":0.2,"master_gain":0.65}})",
        R"({"name":"Up Arp","category":"Arp","author":"Factory","params":{"osc1_wave":3,"osc1_level":0.75,"osc2_wave":2,"osc2_semi":7,"osc2_level":0.4,"filter_type":0,"filter_cutoff":5500,"filter_reso":0.35,"filter_env_amt":0.4,"amp_attack":0.001,"amp_decay":0.2,"amp_sustain":0.0,"amp_release":0.15,"delay_on":1,"delay_sync":4,"delay_feedback":0.4,"delay_mix":0.3,"master_gain":0.65}})",
        R"({"name":"Down Arp","category":"Arp","author":"Factory","params":{"osc1_wave":2,"osc1_level":0.7,"osc2_wave":3,"osc2_semi":-12,"osc2_level":0.5,"filter_type":0,"filter_cutoff":4000,"filter_reso":0.4,"amp_attack":0.001,"amp_decay":0.22,"amp_sustain":0.0,"amp_release":0.18,"delay_on":1,"delay_sync":5,"delay_feedback":0.45,"delay_mix":0.35,"master_gain":0.65}})",
        R"({"name":"Chord Arp","category":"Arp","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.2,"osc1_level":0.6,"osc2_wave":3,"osc2_pos":0.4,"osc2_semi":7,"osc2_level":0.55,"filter_type":0,"filter_cutoff":4500,"filter_reso":0.3,"amp_attack":0.005,"amp_decay":0.35,"amp_sustain":0.2,"amp_release":0.3,"chorus_on":1,"chorus_rate":0.4,"chorus_depth":0.45,"chorus_mix":0.45,"reverb_on":1,"reverb_size":0.6,"reverb_damp":0.5,"reverb_mix":0.35,"master_gain":0.6}})",
        R"({"name":"Techno Arp","category":"Arp","author":"Factory","params":{"osc1_wave":3,"osc1_level":0.7,"osc2_wave":3,"osc2_semi":12,"osc2_level":0.4,"filter_type":0,"filter_cutoff":2500,"filter_reso":0.7,"filter_env_amt":0.6,"amp_attack":0.001,"amp_decay":0.12,"amp_sustain":0.0,"amp_release":0.1,"filt_attack":0.001,"filt_decay":0.1,"filt_sustain":0.0,"filt_release":0.1,"drive_on":1,"drive_amount":4.0,"drive_tone":0.5,"drive_mix":0.4,"delay_on":1,"delay_sync":5,"delay_feedback":0.5,"delay_mix":0.35,"master_gain":0.55}})",
        R"({"name":"Bouncing Arp","category":"Arp","author":"Factory","params":{"osc1_wave":3,"osc1_level":0.7,"osc2_wave":2,"osc2_semi":7,"osc2_level":0.5,"filter_type":0,"filter_cutoff":5000,"filter_reso":0.4,"amp_attack":0.001,"amp_decay":0.18,"amp_sustain":0.0,"amp_release":0.15,"lfo1_wave":0,"lfo1_rate":3.0,"lfo1_depth":0.5,"lfo1_sync":4,"mod1_source":1,"mod1_dest":1,"mod1_amount":0.15,"delay_on":1,"delay_sync":6,"delay_feedback":0.4,"delay_mix":0.3,"master_gain":0.65}})",

        // ATMOSPHERIC
        R"({"name":"Deep Space","category":"Atmospheric","author":"Factory","params":{"osc1_wave":0,"osc1_octave":0,"osc1_level":0.7,"osc2_wave":2,"osc2_octave":1,"osc2_semi":-5,"osc2_level":0.4,"filter_type":0,"filter_cutoff":1200,"filter_reso":0.3,"amp_attack":2.0,"amp_decay":3.0,"amp_sustain":0.9,"amp_release":4.0,"reverb_on":1,"reverb_size":0.95,"reverb_damp":0.3,"reverb_mix":0.55,"master_gain":0.5}})",
        R"({"name":"Sub Drone","category":"Atmospheric","author":"Factory","params":{"osc1_wave":0,"osc1_octave":0,"osc1_level":0.9,"osc2_wave":2,"osc2_octave":0,"osc2_level":0.3,"filter_type":0,"filter_cutoff":600,"filter_reso":0.15,"amp_attack":1.5,"amp_decay":2.0,"amp_sustain":0.85,"amp_release":3.0,"lfo1_wave":0,"lfo1_rate":0.08,"lfo1_depth":0.4,"mod1_source":1,"mod1_dest":5,"mod1_amount":0.3,"reverb_on":1,"reverb_size":0.9,"reverb_damp":0.2,"reverb_mix":0.5,"master_gain":0.5}})",
        R"({"name":"Ethereal","category":"Atmospheric","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.6,"osc1_level":0.55,"osc2_wave":3,"osc2_pos":0.9,"osc2_semi":12,"osc2_level":0.45,"filter_type":0,"filter_cutoff":6500,"filter_reso":0.25,"amp_attack":1.8,"amp_decay":2.5,"amp_sustain":0.7,"amp_release":3.5,"reverb_on":1,"reverb_size":0.9,"reverb_damp":0.4,"reverb_mix":0.55,"delay_on":1,"delay_sync":4,"delay_feedback":0.5,"delay_mix":0.3,"master_gain":0.5}})",
        R"({"name":"Frozen","category":"Atmospheric","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.4,"osc1_level":0.6,"osc2_wave":3,"osc2_pos":0.7,"osc2_semi":19,"osc2_level":0.4,"filter_type":0,"filter_cutoff":5000,"filter_reso":0.5,"amp_attack":1.2,"amp_decay":2.0,"amp_sustain":0.75,"amp_release":3.0,"reverb_on":1,"reverb_size":0.85,"reverb_damp":0.35,"reverb_mix":0.5,"master_gain":0.5}})",
        R"({"name":"Night Sky","category":"Atmospheric","author":"Factory","params":{"osc1_wave":0,"osc1_level":0.65,"osc2_wave":3,"osc2_semi":12,"osc2_level":0.35,"filter_type":0,"filter_cutoff":3000,"filter_reso":0.3,"amp_attack":2.5,"amp_decay":3.0,"amp_sustain":0.85,"amp_release":4.0,"lfo1_wave":0,"lfo1_rate":0.1,"lfo1_depth":0.5,"mod1_source":1,"mod1_dest":3,"mod1_amount":0.4,"reverb_on":1,"reverb_size":0.95,"reverb_damp":0.2,"reverb_mix":0.6,"master_gain":0.5}})",
        R"({"name":"Crystal Drone","category":"Atmospheric","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.9,"osc1_level":0.5,"osc2_wave":3,"osc2_pos":0.6,"osc2_semi":19,"osc2_level":0.4,"filter_type":0,"filter_cutoff":8000,"filter_reso":0.4,"amp_attack":2.0,"amp_decay":2.5,"amp_sustain":0.8,"amp_release":4.0,"reverb_on":1,"reverb_size":0.95,"reverb_damp":0.25,"reverb_mix":0.6,"delay_on":1,"delay_sync":3,"delay_feedback":0.5,"delay_mix":0.35,"master_gain":0.5}})",
        R"({"name":"Ambient Wash","category":"Atmospheric","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.5,"osc1_level":0.55,"osc2_wave":3,"osc2_pos":0.8,"osc2_level":0.45,"filter_type":0,"filter_cutoff":3500,"filter_reso":0.35,"amp_attack":2.5,"amp_decay":3.0,"amp_sustain":0.85,"amp_release":5.0,"lfo1_wave":0,"lfo1_rate":0.15,"lfo1_depth":0.6,"mod1_source":1,"mod1_dest":5,"mod1_amount":0.5,"reverb_on":1,"reverb_size":0.95,"reverb_damp":0.4,"reverb_mix":0.6,"master_gain":0.5}})",
        R"({"name":"Deep Ocean","category":"Atmospheric","author":"Factory","params":{"osc1_wave":0,"osc1_octave":0,"osc1_level":0.8,"osc2_wave":2,"osc2_octave":0,"osc2_semi":-7,"osc2_level":0.4,"filter_type":0,"filter_cutoff":800,"filter_reso":0.3,"amp_attack":3.0,"amp_decay":3.0,"amp_sustain":0.9,"amp_release":5.0,"lfo1_wave":0,"lfo1_rate":0.05,"lfo1_depth":0.5,"mod1_source":1,"mod1_dest":3,"mod1_amount":0.3,"reverb_on":1,"reverb_size":0.95,"reverb_damp":0.3,"reverb_mix":0.55,"master_gain":0.5}})",
        R"({"name":"Cosmic Wind","category":"Atmospheric","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.3,"osc1_level":0.55,"osc2_wave":3,"osc2_pos":0.7,"osc2_level":0.5,"filter_type":2,"filter_cutoff":1500,"filter_reso":0.6,"amp_attack":2.0,"amp_decay":3.0,"amp_sustain":0.7,"amp_release":4.5,"lfo1_wave":5,"lfo1_rate":0.3,"lfo1_depth":0.7,"mod1_source":1,"mod1_dest":5,"mod1_amount":0.6,"reverb_on":1,"reverb_size":0.9,"reverb_damp":0.3,"reverb_mix":0.55,"master_gain":0.5}})",
        R"({"name":"Long Evolution","category":"Atmospheric","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.0,"osc1_level":0.6,"osc2_wave":3,"osc2_pos":0.0,"osc2_semi":7,"osc2_level":0.4,"filter_type":0,"filter_cutoff":2500,"filter_reso":0.35,"amp_attack":4.0,"amp_decay":5.0,"amp_sustain":0.85,"amp_release":6.0,"lfo1_wave":0,"lfo1_rate":0.03,"lfo1_depth":0.9,"mod1_source":1,"mod1_dest":3,"mod1_amount":0.8,"mod2_source":1,"mod2_dest":4,"mod2_amount":0.8,"reverb_on":1,"reverb_size":0.95,"reverb_damp":0.3,"reverb_mix":0.55,"master_gain":0.5}})",

        // FX
        R"({"name":"Digital FX","category":"FX","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.5,"osc1_level":0.6,"osc2_wave":3,"osc2_pos":0.8,"osc2_semi":7,"osc2_level":0.5,"filter_type":2,"filter_cutoff":2000,"filter_reso":0.7,"amp_attack":0.05,"amp_decay":0.5,"amp_sustain":0.5,"amp_release":1.5,"lfo1_wave":5,"lfo1_rate":8.0,"lfo1_depth":0.7,"mod1_source":1,"mod1_dest":3,"mod1_amount":0.6,"mod2_source":1,"mod2_dest":5,"mod2_amount":0.5,"delay_on":1,"delay_sync":6,"delay_feedback":0.6,"delay_mix":0.4,"reverb_on":1,"reverb_size":0.6,"reverb_damp":0.3,"reverb_mix":0.3,"master_gain":0.5}})",
        R"({"name":"Riser","category":"FX","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.0,"osc1_level":0.7,"osc2_wave":3,"osc2_pos":0.0,"osc2_semi":12,"osc2_level":0.5,"filter_type":0,"filter_cutoff":800,"filter_reso":0.6,"amp_attack":0.5,"amp_decay":4.0,"amp_sustain":1.0,"amp_release":0.5,"filt_attack":0.5,"filt_decay":4.0,"filt_sustain":1.0,"filt_release":0.3,"reverb_on":1,"reverb_size":0.8,"reverb_damp":0.4,"reverb_mix":0.4,"master_gain":0.55}})",
        R"({"name":"Noise Sweep","category":"FX","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.0,"osc1_level":0.6,"osc2_wave":3,"osc2_pos":0.0,"osc2_semi":7,"osc2_level":0.5,"filter_type":2,"filter_cutoff":600,"filter_reso":0.85,"amp_attack":0.1,"amp_decay":3.0,"amp_sustain":0.0,"amp_release":0.5,"filt_attack":0.1,"filt_decay":3.0,"filt_sustain":0.0,"filt_release":0.3,"reverb_on":1,"reverb_size":0.7,"reverb_damp":0.3,"reverb_mix":0.35,"master_gain":0.55}})",
        R"({"name":"Alien Voice","category":"FX","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.5,"osc1_level":0.7,"osc2_wave":3,"osc2_pos":0.9,"osc2_semi":5,"osc2_level":0.5,"filter_type":1,"filter_cutoff":1500,"filter_reso":0.6,"amp_attack":0.02,"amp_decay":0.8,"amp_sustain":0.5,"amp_release":0.5,"lfo1_wave":5,"lfo1_rate":3.0,"lfo1_depth":0.8,"mod1_source":1,"mod1_dest":1,"mod1_amount":0.6,"mod2_source":1,"mod2_dest":5,"mod2_amount":0.5,"delay_on":1,"delay_sync":7,"delay_feedback":0.5,"delay_mix":0.35,"master_gain":0.6}})",
        R"({"name":"Glitch","category":"FX","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.7,"osc1_level":0.6,"osc2_wave":3,"osc2_pos":0.2,"osc2_semi":7,"osc2_level":0.5,"filter_type":2,"filter_cutoff":3000,"filter_reso":0.7,"amp_attack":0.001,"amp_decay":0.1,"amp_sustain":0.3,"amp_release":0.15,"lfo1_wave":6,"lfo1_rate":10.0,"lfo1_depth":0.9,"mod1_source":1,"mod1_dest":3,"mod1_amount":0.7,"mod2_source":1,"mod2_dest":5,"mod2_amount":0.6,"delay_on":1,"delay_sync":8,"delay_feedback":0.6,"delay_mix":0.4,"master_gain":0.55}})",
        R"({"name":"Siren FX","category":"FX","author":"Factory","params":{"osc1_wave":2,"osc1_level":0.8,"osc2_wave":3,"osc2_level":0.5,"filter_type":0,"filter_cutoff":4000,"filter_reso":0.3,"amp_attack":0.05,"amp_decay":0.5,"amp_sustain":0.9,"amp_release":0.5,"lfo1_wave":0,"lfo1_rate":2.0,"lfo1_depth":1.0,"mod1_source":1,"mod1_dest":1,"mod1_amount":0.8,"delay_on":1,"delay_sync":4,"delay_feedback":0.5,"delay_mix":0.35,"master_gain":0.55}})",
        R"({"name":"Whoosh FX","category":"FX","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.0,"osc1_level":0.6,"osc2_wave":3,"osc2_pos":0.0,"osc2_level":0.5,"filter_type":2,"filter_cutoff":500,"filter_reso":0.85,"amp_attack":0.05,"amp_decay":1.5,"amp_sustain":0.0,"amp_release":0.3,"filt_attack":0.1,"filt_decay":1.5,"filt_sustain":0.0,"filt_release":0.2,"reverb_on":1,"reverb_size":0.8,"reverb_damp":0.4,"reverb_mix":0.4,"master_gain":0.55}})",
        R"({"name":"Blip FX","category":"FX","author":"Factory","params":{"osc1_wave":3,"osc1_level":0.8,"osc2_wave":0,"osc2_level":0.3,"filter_type":0,"filter_cutoff":9000,"filter_reso":0.2,"amp_attack":0.001,"amp_decay":0.05,"amp_sustain":0.0,"amp_release":0.05,"master_gain":0.65}})",
        R"({"name":"Sci-Fi Zap","category":"FX","author":"Factory","params":{"osc1_wave":2,"osc1_level":0.8,"osc2_wave":0,"osc2_semi":24,"osc2_level":0.4,"filter_type":0,"filter_cutoff":6000,"filter_reso":0.5,"amp_attack":0.001,"amp_decay":0.3,"amp_sustain":0.0,"amp_release":0.15,"filt_attack":0.001,"filt_decay":0.2,"filt_sustain":0.0,"filt_release":0.1,"lfo1_wave":4,"lfo1_rate":20.0,"lfo1_depth":0.8,"mod1_source":1,"mod1_dest":1,"mod1_amount":0.6,"master_gain":0.6}})",
        R"({"name":"Broken Radio","category":"FX","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.5,"osc1_level":0.65,"osc2_wave":3,"osc2_pos":0.2,"osc2_semi":7,"osc2_level":0.55,"filter_type":2,"filter_cutoff":1200,"filter_reso":0.75,"amp_attack":0.001,"amp_decay":0.15,"amp_sustain":0.5,"amp_release":0.2,"lfo1_wave":6,"lfo1_rate":15.0,"lfo1_depth":0.9,"mod1_source":1,"mod1_dest":1,"mod1_amount":0.5,"mod2_source":1,"mod2_dest":5,"mod2_amount":0.6,"drive_on":1,"drive_amount":6.0,"drive_tone":0.4,"drive_mix":0.5,"master_gain":0.55}})",

        // CHORD
        R"({"name":"Maj7 Pad","category":"Chord","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.2,"osc1_level":0.55,"osc2_wave":3,"osc2_pos":0.4,"osc2_semi":4,"osc2_level":0.55,"filter_type":0,"filter_cutoff":3800,"filter_reso":0.2,"amp_attack":0.6,"amp_decay":1.2,"amp_sustain":0.8,"amp_release":1.8,"chorus_on":1,"chorus_rate":0.4,"chorus_depth":0.4,"chorus_mix":0.4,"reverb_on":1,"reverb_size":0.7,"reverb_damp":0.5,"reverb_mix":0.35,"master_gain":0.6}})",
        R"({"name":"Minor Chord","category":"Chord","author":"Factory","params":{"osc1_wave":2,"osc1_level":0.55,"osc2_wave":3,"osc2_semi":3,"osc2_level":0.55,"filter_type":0,"filter_cutoff":3200,"filter_reso":0.25,"amp_attack":0.5,"amp_decay":1.0,"amp_sustain":0.8,"amp_release":1.5,"reverb_on":1,"reverb_size":0.65,"reverb_damp":0.5,"reverb_mix":0.35,"master_gain":0.6}})",
        R"({"name":"Stacked Saw","category":"Chord","author":"Factory","params":{"osc1_wave":2,"osc1_level":0.5,"osc1_fine":-10.0,"osc2_wave":2,"osc2_level":0.5,"osc2_fine":10.0,"filter_type":0,"filter_cutoff":4200,"filter_reso":0.2,"amp_attack":0.3,"amp_decay":0.9,"amp_sustain":0.85,"amp_release":1.2,"drive_on":1,"drive_amount":2.0,"drive_tone":0.6,"drive_mix":0.25,"reverb_on":1,"reverb_size":0.55,"reverb_damp":0.5,"reverb_mix":0.3,"master_gain":0.6}})",
        R"({"name":"Detuned Chord","category":"Chord","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.3,"osc1_level":0.5,"osc1_fine":-15.0,"osc2_wave":2,"osc2_pos":0.6,"osc2_semi":7,"osc2_level":0.5,"osc2_fine":15.0,"filter_type":0,"filter_cutoff":3500,"filter_reso":0.3,"amp_attack":0.5,"amp_decay":1.1,"amp_sustain":0.8,"amp_release":1.5,"chorus_on":1,"chorus_rate":0.5,"chorus_depth":0.5,"chorus_mix":0.45,"reverb_on":1,"reverb_size":0.7,"reverb_damp":0.45,"reverb_mix":0.35,"master_gain":0.6}})",
        R"({"name":"Wide Chord","category":"Chord","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.4,"osc1_level":0.55,"osc2_wave":3,"osc2_pos":0.7,"osc2_semi":12,"osc2_level":0.5,"filter_type":0,"filter_cutoff":5500,"filter_reso":0.2,"amp_attack":0.4,"amp_decay":1.0,"amp_sustain":0.85,"amp_release":1.5,"chorus_on":1,"chorus_rate":0.35,"chorus_depth":0.4,"chorus_mix":0.4,"reverb_on":1,"reverb_size":0.75,"reverb_damp":0.4,"reverb_mix":0.4,"master_gain":0.55}})",
        R"({"name":"Sus4 Pad Chord","category":"Chord","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.2,"osc1_level":0.55,"osc2_wave":3,"osc2_semi":5,"osc2_level":0.55,"filter_type":0,"filter_cutoff":4000,"filter_reso":0.25,"amp_attack":0.7,"amp_decay":1.3,"amp_sustain":0.8,"amp_release":2.0,"chorus_on":1,"chorus_rate":0.4,"chorus_depth":0.45,"chorus_mix":0.4,"reverb_on":1,"reverb_size":0.75,"reverb_damp":0.5,"reverb_mix":0.4,"master_gain":0.6}})",
        R"({"name":"Minor 7 Pad","category":"Chord","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.3,"osc1_level":0.55,"osc2_wave":3,"osc2_semi":3,"osc2_level":0.5,"filter_type":0,"filter_cutoff":3500,"filter_reso":0.3,"amp_attack":0.8,"amp_decay":1.4,"amp_sustain":0.8,"amp_release":2.0,"chorus_on":1,"chorus_rate":0.45,"chorus_depth":0.5,"chorus_mix":0.45,"reverb_on":1,"reverb_size":0.75,"reverb_damp":0.5,"reverb_mix":0.4,"master_gain":0.6}})",
        R"({"name":"Major 9 Chord","category":"Chord","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.4,"osc1_level":0.5,"osc2_wave":3,"osc2_pos":0.6,"osc2_semi":4,"osc2_level":0.5,"filter_type":0,"filter_cutoff":4500,"filter_reso":0.25,"amp_attack":0.6,"amp_decay":1.2,"amp_sustain":0.85,"amp_release":1.8,"chorus_on":1,"chorus_rate":0.4,"chorus_depth":0.4,"chorus_mix":0.4,"reverb_on":1,"reverb_size":0.7,"reverb_damp":0.5,"reverb_mix":0.4,"master_gain":0.6}})",
        R"({"name":"Cinematic Chord","category":"Chord","author":"Factory","params":{"osc1_wave":2,"osc1_pos":0.3,"osc1_level":0.55,"osc1_fine":-7.0,"osc2_wave":3,"osc2_pos":0.5,"osc2_semi":7,"osc2_level":0.55,"osc2_fine":7.0,"filter_type":0,"filter_cutoff":3800,"filter_reso":0.35,"amp_attack":1.2,"amp_decay":1.8,"amp_sustain":0.85,"amp_release":2.8,"reverb_on":1,"reverb_size":0.85,"reverb_damp":0.45,"reverb_mix":0.5,"delay_on":1,"delay_sync":3,"delay_feedback":0.4,"delay_mix":0.25,"master_gain":0.55}})",
        R"({"name":"Ambient Chord","category":"Chord","author":"Factory","params":{"osc1_wave":3,"osc1_pos":0.5,"osc1_level":0.5,"osc2_wave":3,"osc2_pos":0.8,"osc2_semi":12,"osc2_level":0.5,"filter_type":0,"filter_cutoff":5500,"filter_reso":0.25,"amp_attack":1.5,"amp_decay":2.0,"amp_sustain":0.8,"amp_release":3.0,"chorus_on":1,"chorus_rate":0.3,"chorus_depth":0.5,"chorus_mix":0.45,"reverb_on":1,"reverb_size":0.9,"reverb_damp":0.4,"reverb_mix":0.55,"master_gain":0.55}})"
    };

    // ============ PresetManager ============

    PresetManager::PresetManager (juce::AudioProcessorValueTreeState& a) : apvts (a)
    {
        loadFactoryPresets();
        refresh();
        loadFavorites();
    }

    void PresetManager::loadFactoryPresets()
    {
        for (auto* jsonStr : kFactoryJSON)
        {
            auto parsed = juce::JSON::parse (juce::String (jsonStr));
            if (auto* obj = parsed.getDynamicObject())
            {
                PresetInfo info;
                info.name     = obj->getProperty ("name").toString();
                info.category = obj->getProperty ("category").toString();
                info.author   = obj->getProperty ("author").toString();
                info.isFactory = true;
                allPresets.add (info);
            }
        }
    }

    juce::File PresetManager::getUserPresetDirectory() const
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                       .getChildFile ("OpenWave Audio")
                       .getChildFile ("PPGWave3Clone")
                       .getChildFile ("Presets");
        if (! dir.exists())
            dir.createDirectory();
        return dir;
    }

    // ============ FAVORITOS ============

    juce::File PresetManager::getFavoritesFile() const
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                       .getChildFile ("OpenWave Audio")
                       .getChildFile ("PPGWave3Clone");
        if (! dir.exists())
            dir.createDirectory();
        return dir.getChildFile ("favorites.json");
    }

    void PresetManager::loadFavorites()
    {
        favorites.clear();

        auto file = getFavoritesFile();
        if (! file.existsAsFile()) return;

        auto parsed = juce::JSON::parse (file.loadFileAsString());
        if (auto* arr = parsed.getArray())
        {
            for (const auto& v : *arr)
                favorites.add (v.toString());
        }
    }

    void PresetManager::saveFavorites()
    {
        juce::Array<juce::var> arr;
        for (const auto& s : favorites)
            arr.add (s);

        auto* jsonVar = new juce::DynamicObject();
        juce::ignoreUnused (jsonVar);   // sin uso, solo por claridad

        // Crear JSON: array de strings
        juce::var root (arr);
        auto file = getFavoritesFile();
        file.replaceWithText (juce::JSON::toString (root));
    }

    bool PresetManager::isFavorite (const juce::String& presetName) const
    {
        return favorites.contains (presetName);
    }

    void PresetManager::toggleFavorite (const juce::String& presetName)
    {
        if (presetName.isEmpty()) return;

        if (favorites.contains (presetName))
            favorites.removeString (presetName);
        else
            favorites.add (presetName);

        saveFavorites();
    }

    juce::Array<int> PresetManager::getFavoriteIndices() const
    {
        juce::Array<int> result;
        for (int i = 0; i < allPresets.size(); ++i)
            if (isFavorite (allPresets.getReference (i).name))
                result.add (i);
        return result;
    }

    // ============ RESTO ============

    void PresetManager::refresh()
    {
        juce::Array<PresetInfo> factoryOnly;
        for (const auto& p : allPresets)
            if (p.isFactory) factoryOnly.add (p);

        allPresets = factoryOnly;

        auto dir = getUserPresetDirectory();
        juce::Array<juce::File> files;
        dir.findChildFiles (files, juce::File::findFiles, false, "*.json");

        for (const auto& f : files)
        {
            auto text = f.loadFileAsString();
            auto parsed = juce::JSON::parse (text);
            if (auto* obj = parsed.getDynamicObject())
            {
                PresetInfo info;
                info.name     = obj->getProperty ("name").toString();
                info.category = obj->getProperty ("category").toString();
                info.author   = obj->getProperty ("author").toString();
                info.isFactory = false;
                info.file      = f;
                allPresets.add (info);
            }
        }
    }

    void PresetManager::applyPreset (const juce::var& preset)
    {
        auto* obj = preset.getDynamicObject();
        if (! obj) return;

        auto paramsVar = obj->getProperty ("params");
        auto* paramsObj = paramsVar.getDynamicObject();
        if (! paramsObj) return;

        for (const auto& prop : paramsObj->getProperties())
        {
            const auto id = prop.name.toString();
            if (auto* param = apvts.getParameter (id))
            {
                if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
                {
                    const float targetValue = (float) prop.value;
                    const auto& range = ranged->getNormalisableRange();
                    const float normalized = range.convertTo0to1 (targetValue);
                    param->setValueNotifyingHost (normalized);
                }
            }
        }
    }

    juce::var PresetManager::captureCurrentState() const
    {
        auto* root = new juce::DynamicObject();
        auto* params = new juce::DynamicObject();

        for (auto* p : apvts.processor.getParameters())
        {
            if (auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*> (p))
            {
                if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p))
                {
                    const float norm = p->getValue();
                    const float real = ranged->getNormalisableRange().convertFrom0to1 (norm);
                    params->setProperty (withId->paramID, real);
                }
            }
        }

        root->setProperty ("params", juce::var (params));
        return juce::var (root);
    }

    void PresetManager::loadByIndex (int index)
    {
        if (allPresets.isEmpty()) return;
        index = juce::jlimit (0, allPresets.size() - 1, index);
        currentIndex = index;

        const auto& info = allPresets.getReference (index);
        juce::var preset;

        if (info.isFactory)
        {
            int i = 0;
            for (auto* jsonStr : kFactoryJSON)
            {
                if (i == index)
                {
                    preset = juce::JSON::parse (juce::String (jsonStr));
                    break;
                }
                ++i;
            }
        }
        else
        {
            preset = juce::JSON::parse (info.file.loadFileAsString());
        }

        if (! preset.isVoid())
            applyPreset (preset);
    }

    void PresetManager::loadFromFile (const juce::File& f)
    {
        if (! f.existsAsFile()) return;
        auto preset = juce::JSON::parse (f.loadFileAsString());
        if (! preset.isVoid())
            applyPreset (preset);
    }

    void PresetManager::next()
    {
        if (allPresets.isEmpty()) return;

        // Si NO está activo el modo favoritos: siguiente normal.
        if (! favoritesOnly)
        {
            loadByIndex ((currentIndex + 1) % allPresets.size());
            return;
        }

        // Modo favoritos: buscar el siguiente favorito.
        const int total = allPresets.size();
        for (int step = 1; step <= total; ++step)
        {
            const int i = (currentIndex + step) % total;
            if (isFavorite (allPresets.getReference (i).name))
            {
                loadByIndex (i);
                return;
            }
        }
        // Si no hay favoritos, no navegar.
    }

    void PresetManager::prev()
    {
        if (allPresets.isEmpty()) return;

        if (! favoritesOnly)
        {
            int i = currentIndex - 1;
            if (i < 0) i = allPresets.size() - 1;
            loadByIndex (i);
            return;
        }

        const int total = allPresets.size();
        for (int step = 1; step <= total; ++step)
        {
            const int i = (currentIndex - step + total * 2) % total;
            if (isFavorite (allPresets.getReference (i).name))
            {
                loadByIndex (i);
                return;
            }
        }
    }

    bool PresetManager::saveUserPreset (const juce::String& name, const juce::String& category)
    {
        if (name.isEmpty()) return false;

        auto root = captureCurrentState();
        auto* obj = root.getDynamicObject();
        if (! obj) return false;

        obj->setProperty ("name", name);
        obj->setProperty ("category", category);
        obj->setProperty ("author", "User");

        auto dir = getUserPresetDirectory();
        auto safeName = juce::File::createLegalFileName (name);
        auto file = dir.getChildFile (safeName + ".json");

        if (! file.replaceWithText (juce::JSON::toString (root, true)))
            return false;

        refresh();
        return true;
    }

    juce::String PresetManager::getCurrentName() const
    {
        if (allPresets.isEmpty()) return "-";
        return allPresets.getReference (currentIndex).name;
    }

    juce::String PresetManager::getCurrentCategory() const
    {
        if (allPresets.isEmpty()) return "-";
        return allPresets.getReference (currentIndex).category;
    }
}
