#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <array>
#include <vector>

class StepSequencer
{
public:
    static constexpr int numSteps = 16;

    struct Step
    {
        std::atomic<bool>  active      { false };
        std::atomic<int>   pitch       { 0 };      // -24..+24 semitonos
        std::atomic<float> velocity    { 0.8f };   // 0..1
        std::atomic<float> gate        { 1.0f };   // 0.1..2.0 (proporción del step)
        std::atomic<float> probability { 1.0f };   // 0..1
    };

    struct Event
    {
        enum Type { NoteOn, NoteOff } type;
        int   sampleOffset;
        int   midiNote;
        float velocity;
    };

    void prepare (double sampleRate) { currentSampleRate = sampleRate; }
    void reset();

    void process (double bpm,
                  double ppqPosition,
                  bool isPlaying,
                  int numSamples,
                  std::vector<Event>& outEvents);

    // Operaciones one-shot (llamadas desde la UI)
    void randomizeAll();
    void clearAll();
    void shiftLeft();
    void shiftRight();

    // Parámetros globales (atómicos, seguros entre hilos)
    std::atomic<bool>  enabled   { false };
    std::atomic<int>   rateIndex { 3 };      // 0=1/1 ... 4=1/16, 5=1/8T, 6=1/16T, 7=1/4.
    std::atomic<float> swing     { 0.0f };   // 0..1
    std::atomic<int>   length    { 16 };     // 1..16
    std::atomic<int>   direction { 0 };      // 0=Fwd, 1=Rev, 2=PingPong, 3=Random
    std::atomic<int>   baseNote  { 60 };     // nota raíz

    std::array<Step, numSteps> steps;

    // Para la UI: paso actual (0..15) o -1 si parado.
    std::atomic<int> currentStep { -1 };

    juce::ValueTree toValueTree() const;
    void fromValueTree (const juce::ValueTree& v);

private:
    int computeSeqPos (int absoluteStep) const;

    double currentSampleRate = 44100.0;

    int    pendingNote          = -1;
    int    samplesUntilNoteOff  = -1;
};
