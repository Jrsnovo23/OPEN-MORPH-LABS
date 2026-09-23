#pragma once
#include <juce_dsp/juce_dsp.h>

namespace dsp
{
    // Cadena global post-mix:
    //   Drive -> Chorus -> Phaser -> Delay -> Reverb -> Vintage -> EQ -> Compressor
    class Effects
    {
    public:
        void prepare (double sampleRate, int maxBlockSize, int numChannels);
        void reset();

        // FASE 11: process() acepta un sidechain opcional (bus de entrada).
        // Si es nullptr o vacío, el compresor cae a self-sidechain.
        void process (juce::AudioBuffer<float>& buffer,
                      const juce::AudioBuffer<float>* sidechain = nullptr);

        void setChorus (bool on, float rate, float depth, float mix);
        void setDelay  (bool on, float timeSec, float feedback, float mix);
        void setReverb (bool on, float roomSize, float damping, float mix);
        void setDrive  (bool on, float drive, float tone, float mix);

        void setEQ     (bool on,
                        bool hpOn, bool lpOn,
                        float lowFreq,  float lowQ,  float lowGain,
                        float lmidFreq, float lmidQ, float lmidGain,
                        float hmidFreq, float hmidQ, float hmidGain,
                        float highFreq, float highQ, float highGain);
        void setPhaser (bool on, float rate, float depth, float feedback, float mix);

        void setVintage (bool on, float amount,
                         float bits, float srFactor,
                         float noise);

        // FASE 11
        void setCompressor (bool on,
                            float thresholdDb, float ratio,
                            float attackMs, float releaseMs,
                            float kneeDb, float makeupDb,
                            bool sidechainOn, float scAmount);

    private:
        double sr       = 44100.0;
        int    maxBlock = 512;

        // ---- Drive ----
        bool  driveOn      = false;
        float driveAmount  = 1.0f;
        float driveTone    = 0.5f;
        float driveMix     = 0.0f;
        juce::dsp::IIR::Filter<float> toneL, toneR;
        std::vector<float> driveBufferScratch;

        // ---- Chorus ----
        bool  chorusOn     = false;
        float chorusRate   = 0.5f;
        float chorusDepth  = 0.25f;
        float chorusMix    = 0.5f;
        juce::dsp::Chorus<float> chorus;

        // ---- Delay ----
        bool  delayOn      = false;
        float delayTimeSec = 0.3f;
        float delayFeedback= 0.4f;
        float delayMix     = 0.3f;
        juce::dsp::DelayLine<float> delayL { 192000 };
        juce::dsp::DelayLine<float> delayR { 192000 };

        // ---- Reverb ----
        bool  reverbOn     = false;
        float reverbMix    = 0.3f;
        juce::Reverb reverb;

        // ---- EQ ----
        using EQChain = juce::dsp::ProcessorChain<
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>>;

        bool  eqOn   = true;
        bool  eqHpOn = false;
        bool  eqLpOn = false;

        float eqLowFreq  = 100.0f,  eqLowQ  = 0.707f, eqLowGain  = 0.0f;
        float eqLmidFreq = 500.0f,  eqLmidQ = 0.707f, eqLmidGain = 0.0f;
        float eqHmidFreq = 2000.0f, eqHmidQ = 0.707f, eqHmidGain = 0.0f;
        float eqHighFreq = 8000.0f, eqHighQ = 0.707f, eqHighGain = 0.0f;

        EQChain eqL, eqR;

        // ---- Phaser ----
        bool  phaserOn     = false;
        juce::dsp::Phaser<float> phaser;

        // ---- Vintage ----
        bool  vintageOn     = false;
        float vintageAmount = 0.5f;
        float vintageBits   = 12.0f;
        float vintageSr     = 1.0f;
        float vintageNoise  = 0.15f;

        float heldL    = 0.0f;
        float heldR    = 0.0f;
        float srCounter = 1.0f;

        juce::Random vintageRng;

        // ---- FASE 11: Compressor ----
        bool  compOn         = false;
        float compThreshold  = -12.0f;
        float compRatio      = 4.0f;
        float compAttack     = 10.0f;
        float compRelease    = 100.0f;
        float compKnee       = 6.0f;
        float compMakeup     = 0.0f;
        bool  compSidechain  = false;
        float compScAmount   = 1.0f;

        // Envelope follower state
        float  compEnvelope     = 0.0f;
        float  compAttackCoef   = 0.0f;
        float  compReleaseCoef  = 0.0f;
        float  compLastAttack   = -1.0f;
        float  compLastRelease  = -1.0f;
        double compLastSr       = -1.0;

        void computeCompCoeffs();
    };
}
