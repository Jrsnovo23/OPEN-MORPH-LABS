#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <array>
#include <vector>

class Arpeggiator
{
public:
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
                  const juce::MidiKeyboardState& keyboardState,
                  std::vector<Event>& outEvents);

    // Parámetros globales (atómicos, seguros entre hilos)
    std::atomic<bool>  enabled   { false };
    std::atomic<int>   mode      { 0 };       // 0=Up, 1=Down, 2=UpDown, 3=Random
    std::atomic<int>   octaves   { 1 };       // 1..4
    std::atomic<int>   rateIndex { 4 };       // mismo sistema que StepSequencer
    std::atomic<float> gate      { 0.5f };    // 0.1..1.0
    std::atomic<bool>  latch     { false };

    // Para la UI: índice del paso actual (0..N-1) o -1
    std::atomic<int> currentStepIndex { -1 };

    juce::ValueTree toValueTree() const;
    void fromValueTree (const juce::ValueTree& v);

private:
    std::vector<int> buildNoteList (const juce::MidiKeyboardState& state) const;
    int getSequenceLength (int numBaseNotes) const;
    int getNoteAtIndex (const std::vector<int>& baseNotes, int seqIndex) const;

    double currentSampleRate = 44100.0;

    // Estado interno
    std::array<bool, 128> latchedNotes {};
    int  currentNote           = -1;
    int  samplesUntilNoteOff   = -1;
    int  currentSequenceIndex  = 0;
    int  upDownDirection       = 1;

    // Para detectar si el usuario acaba de tocar o de soltar todas las notas
    bool anyNoteHeldPrevBlock = false;
};
