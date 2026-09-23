#include "SynthVoice.h"
#include "SynthSound.h"
#include "../ParameterIDs.h"
#include <juce_core/juce_core.h>
#include <cmath>

namespace synth
{
    static inline float noteToHz (int midiNote) noexcept
    {
        return 440.0f * std::pow (2.0f, (float) (midiNote - 69) / 12.0f);
    }

    SynthVoice::SynthVoice (juce::AudioProcessorValueTreeState& s) : apvts (s)
    {
        vintageRandom.setSeed ((juce::int64) juce::Time::currentTimeMillis());
    }

    bool SynthVoice::canPlaySound (juce::SynthesiserSound* s)
    {
        return dynamic_cast<SynthSound*> (s) != nullptr;
    }

    float SynthVoice::syncIndexToBeats (int idx) noexcept
    {
        switch (idx)
        {
            case 1:  return 4.0f;
            case 2:  return 2.0f;
            case 3:  return 1.0f;
            case 4:  return 0.5f;
            case 5:  return 0.25f;
            case 6:  return 1.0f * 2.0f/3.0f;
            case 7:  return 0.5f * 2.0f/3.0f;
            case 8:  return 0.25f * 2.0f/3.0f;
            case 9:  return 1.5f;
            case 10: return 0.75f;
            default: return 0.0f;
        }
    }

    void SynthVoice::setCurrentPlaybackSampleRate (double newRate)
    {
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);
        if (newRate > 0.0)
        {
            osc1.prepare (newRate);
            osc2.prepare (newRate);
            filter.prepare (newRate);
            lfo1.prepare (newRate);
            lfo2.prepare (newRate);
            adsr.setSampleRate (newRate);
            adsr.setParameters (adsrParams);
            filtAdsr.setSampleRate (newRate);
            filtAdsr.setParameters (filtAdsrParams);
            env3.setSampleRate (newRate);
            env3.setParameters (env3Params);
        }
    }

    void SynthVoice::startNote (int midiNote, float velocity,
                                juce::SynthesiserSound*, int)
    {
        currentFreq     = noteToHz (midiNote);
        currentMidiNote = (float) midiNote;
        velocityGain    = juce::jlimit (0.0f, 1.0f, velocity);

        osc1.reset (0.0f);
        osc2.reset (0.0f);
        filter.reset();
        lfo1.reset (0.0f);
        lfo2.reset (0.0f);
        adsr.noteOn();
        filtAdsr.noteOn();
        env3.noteOn();

        juce::Random r;
        randomValue = r.nextFloat() * 2.0f - 1.0f;

        // ===== FASE 8: Voice variation =====
        float varAmt = 0.0f;
        if (auto* p = apvts.getRawParameterValue (ParamIDs::vintageVar))
            varAmt = p->load();

        if (varAmt > 0.0f)
        {
            voiceDetune       = (vintageRandom.nextFloat() * 2.0f - 1.0f) * varAmt * 15.0f;    // ±15 cents
            voiceFilterOffset = (vintageRandom.nextFloat() * 2.0f - 1.0f) * varAmt * 0.25f;   // ±0.25 octavas
            voiceLevelOffset  = 1.0f + (vintageRandom.nextFloat() * 2.0f - 1.0f) * varAmt * 0.1f; // ±10%
        }
        else
        {
            voiceDetune       = 0.0f;
            voiceFilterOffset = 0.0f;
            voiceLevelOffset  = 1.0f;
        }

        driftSeed  = vintageRandom.nextFloat();
        driftPhase = 0.0f;

        isActive = true;
    }

    void SynthVoice::stopNote (float, bool allowTailOff)
    {
        if (allowTailOff)
        {
            adsr.noteOff();
            filtAdsr.noteOff();
            env3.noteOff();
        }
        else
        {
            adsr.reset();
            filtAdsr.reset();
            env3.reset();
            clearCurrentNote();
            isActive = false;
        }
    }

    void SynthVoice::pitchWheelMoved (int newValue)
    {
        pitchBendValue = ((float) newValue - 8192.0f) / 8192.0f;
    }

    void SynthVoice::controllerMoved (int controllerNumber, int newValue)
    {
        const float v = (float) newValue / 127.0f;
        if (controllerNumber == 1)   modWheelValue   = v;
        if (controllerNumber == 128) aftertouchValue = v;
    }

    void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& output,
                                      int startSample, int numSamples)
    {
        if (! isActive) return;

        auto getF = [&] (const char* id, float def) -> float
        {
            if (auto* p = apvts.getRawParameterValue (id)) return p->load();
            return def;
        };
        auto getI = [&] (const char* id, int def) -> int
        {
            if (auto* p = apvts.getRawParameterValue (id)) return (int) p->load();
            return def;
        };

        // ---------- Envolventes ----------
        adsrParams.attack  = getF (ParamIDs::ampAttack,  0.005f);
        adsrParams.decay   = getF (ParamIDs::ampDecay,   0.100f);
        adsrParams.sustain = getF (ParamIDs::ampSustain, 0.700f);
        adsrParams.release = getF (ParamIDs::ampRelease, 0.300f);
        adsr.setParameters (adsrParams);

        filtAdsrParams.attack  = getF (ParamIDs::filtAttack,  0.005f);
        filtAdsrParams.decay   = getF (ParamIDs::filtDecay,   0.200f);
        filtAdsrParams.sustain = getF (ParamIDs::filtSustain, 0.500f);
        filtAdsrParams.release = getF (ParamIDs::filtRelease, 0.300f);
        filtAdsr.setParameters (filtAdsrParams);

        env3Params.attack  = getF (ParamIDs::env3Attack,  0.005f);
        env3Params.decay   = getF (ParamIDs::env3Decay,   0.200f);
        env3Params.sustain = getF (ParamIDs::env3Sustain, 0.500f);
        env3Params.release = getF (ParamIDs::env3Release, 0.300f);
        env3.setParameters (env3Params);

        // ---------- Wavetables ----------
        const int w1 = getI (ParamIDs::osc1Wave, 0);
        const int w2 = getI (ParamIDs::osc2Wave, 2);
        if (w1 != currentWave1Idx) { wave1 = dsp::wavetables::makeByIndex (w1); currentWave1Idx = w1; }
        if (w2 != currentWave2Idx) { wave2 = dsp::wavetables::makeByIndex (w2); currentWave2Idx = w2; }

        // ---------- Osciladores ----------
        const float basePos1 = getF (ParamIDs::osc1Pos,   0.0f);
        const float basePos2 = getF (ParamIDs::osc2Pos,   0.0f);
        const float lvl1     = getF (ParamIDs::osc1Level, 0.8f);
        const float lvl2     = getF (ParamIDs::osc2Level, 0.0f);
        const float master   = getF (ParamIDs::masterGain,0.7f);
        const int   oct1     = getI (ParamIDs::osc1Octave, 2) - 2;
        const int   oct2     = getI (ParamIDs::osc2Octave, 2) - 2;
        const float sem1     = getF (ParamIDs::osc1Semi, 0.0f);
        const float sem2     = getF (ParamIDs::osc2Semi, 0.0f);
        const float fin1     = getF (ParamIDs::osc1Fine, 0.0f);
        const float fin2     = getF (ParamIDs::osc2Fine, 0.0f);

        const float bendSemis = pitchBendValue * 2.0f;

        // ===== FASE 8: Vintage drift =====
        const float driftAmt = getF (ParamIDs::vintageDrift, 0.0f);
        const double sr     = getSampleRate();
        const float driftPhaseInc = (sr > 0.0) ? (float) (0.2 / sr) : 0.0f;
        const float twoPiF  = juce::MathConstants<float>::twoPi;

        auto freqOf = [&] (int oct, float semis, float cents) noexcept
        {
            const float st = (float) oct * 12.0f + semis + cents / 100.0f + bendSemis;
            return currentFreq * std::pow (2.0f, st / 12.0f);
        };

        // ---------- Filtro ----------
        const int   fType      = getI (ParamIDs::filterType,     0);
        const float baseCutoff = getF (ParamIDs::filterCutoff,   8000.0f);
        const float baseReso   = getF (ParamIDs::filterReso,     0.0f);
        const float fEnvAmt    = getF (ParamIDs::filterEnvAmt,   0.0f);
        const float fKeyTrack  = getF (ParamIDs::filterKeyTrack, 0.0f);

        filter.setType (fType);

        const float keyTrackedCutoff = baseCutoff
            * std::pow (2.0f, fKeyTrack * (currentMidiNote - 60.0f) / 12.0f)
            * std::pow (2.0f, voiceFilterOffset);

        // ---------- LFOs ----------
        const double bpm = bpmSource ? bpmSource->load() : 120.0;
        const double beatSec = 60.0 / juce::jmax (1.0, bpm);

        auto computeLfoRate = [&] (const char* syncId, float freeRate) -> float
        {
            const int syncIdx = getI (syncId, 0);
            if (syncIdx == 0) return freeRate;
            const float beats = syncIndexToBeats (syncIdx);
            if (beats <= 0.0f) return freeRate;
            return (float) (1.0 / (beatSec * beats));
        };

        const int   lfo1W = getI (ParamIDs::lfo1Wave,  0);
        const float lfo1R = computeLfoRate (ParamIDs::lfo1Sync, getF (ParamIDs::lfo1Rate, 1.0f));
        const float lfo1D = getF (ParamIDs::lfo1Depth, 0.0f);
        const float lfo1P = getF (ParamIDs::lfo1Phase, 0.0f);
        const int   lfo2W = getI (ParamIDs::lfo2Wave,  0);
        const float lfo2R = computeLfoRate (ParamIDs::lfo2Sync, getF (ParamIDs::lfo2Rate, 1.0f));
        const float lfo2D = getF (ParamIDs::lfo2Depth, 0.0f);
        const float lfo2P = getF (ParamIDs::lfo2Phase, 0.0f);

        const auto lfo1WaveEnum = (dsp::LFO::Wave) lfo1W;
        const auto lfo2WaveEnum = (dsp::LFO::Wave) lfo2W;

        // ---------- Matriz ----------
        ModSlot mods[4];
        mods[0] = { getI (ParamIDs::mod1Source, 0), getI (ParamIDs::mod1Dest, 0),
                    getF (ParamIDs::mod1Amount, 0.0f) };
        mods[1] = { getI (ParamIDs::mod2Source, 0), getI (ParamIDs::mod2Dest, 0),
                    getF (ParamIDs::mod2Amount, 0.0f) };
        mods[2] = { getI (ParamIDs::mod3Source, 0), getI (ParamIDs::mod3Dest, 0),
                    getF (ParamIDs::mod3Amount, 0.0f) };
        mods[3] = { getI (ParamIDs::mod4Source, 0), getI (ParamIDs::mod4Dest, 0),
                    getF (ParamIDs::mod4Amount, 0.0f) };

        auto* L = output.getWritePointer (0, startSample);
        auto* R = output.getNumChannels() > 1 ? output.getWritePointer (1, startSample) : nullptr;

        constexpr int updateInterval = 32;

        for (int i = 0; i < numSamples; ++i)
        {
            const float env      = adsr.getNextSample();
            const float filtEnv  = filtAdsr.getNextSample();
            const float env3Val  = env3.getNextSample();

            // ===== FASE 8: drift LFO (slow sine) =====
            float driftCents = 0.0f;
            if (driftAmt > 0.0f)
            {
                driftPhase += driftPhaseInc;
                if (driftPhase >= 1.0f) driftPhase -= 1.0f;
                driftCents = std::sin (driftPhase * twoPiF + driftSeed * twoPiF) * driftAmt * 10.0f;
            }

            const float totalCents = driftCents + voiceDetune;

            const float lfo1Val = lfo1.getNextSample (lfo1WaveEnum, lfo1R, lfo1P) * lfo1D;
            const float lfo2Val = lfo2.getNextSample (lfo2WaveEnum, lfo2R, lfo2P) * lfo2D;

            const float noteNumberVal = juce::jlimit (-1.0f, 1.0f,
                (currentMidiNote - 60.0f) / 60.0f);

            float modPitch1 = 0.0f;
            float modPitch2 = 0.0f;
            float modWT1    = 0.0f;
            float modWT2    = 0.0f;
            float modCutoff = 0.0f;
            float modAmp    = 1.0f;
            float modReso   = 0.0f;
            float modFine1  = 0.0f;
            float modFine2  = 0.0f;

            for (const auto& m : mods)
            {
                if (m.source == 0 || m.dest == 0 || m.amount == 0.0f) continue;

                float srcVal = 0.0f;
                switch (m.source)
                {
                    case 1:  srcVal = lfo1Val;         break;
                    case 2:  srcVal = lfo2Val;         break;
                    case 3:  srcVal = env;             break;
                    case 4:  srcVal = filtEnv;         break;
                    case 5:  srcVal = velocityGain;    break;
                    case 6:  srcVal = modWheelValue;   break;
                    case 7:  srcVal = aftertouchValue; break;
                    case 8:  srcVal = noteNumberVal;   break;
                    case 9:  srcVal = randomValue;     break;
                    case 10: srcVal = env3Val;         break;
                    default: break;
                }

                const float v = srcVal * m.amount;

                switch (m.dest)
                {
                    case 1: modPitch1 += v; break;
                    case 2: modPitch2 += v; break;
                    case 3: modWT1    += v; break;
                    case 4: modWT2    += v; break;
                    case 5: modCutoff += v; break;
                    case 6: modAmp    += v; break;
                    case 7: modReso   += v; break;
                    case 8: modFine2  += v; break;
                    case 9: modFine1  += v; break;
                    default: break;
                }
            }

            // Aplicar totalCents (drift + voiceDetune) a ambos osciladores.
            const float f1 = freqOf (oct1, sem1, fin1 + totalCents + modFine1)
                             * std::pow (2.0f, modPitch1 * 2.0f);
            const float f2 = freqOf (oct2, sem2, fin2 + totalCents + modFine2)
                             * std::pow (2.0f, modPitch2 * 2.0f);

            const float pos1 = juce::jlimit (0.0f, 1.0f, basePos1 + modWT1);
            const float pos2 = juce::jlimit (0.0f, 1.0f, basePos2 + modWT2);

            if (i % updateInterval == 0)
            {
                const float envMod = std::pow (2.0f, fEnvAmt * filtEnv * 5.0f);
                const float modMod = std::pow (2.0f, modCutoff * 5.0f);
                filter.setCutoff (keyTrackedCutoff * envMod * modMod);
                filter.setResonance (juce::jlimit (0.0f, 1.0f, baseReso + modReso));
            }

            const float s1 = osc1.getNextSample (wave1, f1, pos1);
            const float s2 = osc2.getNextSample (wave2, f2, pos2);
            float mix = s1 * lvl1 + s2 * lvl2;

            mix = filter.processSample (mix);
            mix *= env * velocityGain * master * voiceLevelOffset
                 * juce::jlimit (0.0f, 2.0f, modAmp);

            L[i] += mix;
            if (R) R[i] += mix;
        }

        if (! adsr.isActive())
        {
            clearCurrentNote();
            isActive = false;
        }
    }
}
