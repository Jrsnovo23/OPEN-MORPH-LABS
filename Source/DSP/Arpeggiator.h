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

    enum class Mode
    {
        Up = 0,
        Down,
        UpDown,
        DownUp,
        Random,
        AsPlayed,
        Chord
    };

    void prepare (double sampleRate) { currentSampleRate = sampleRate; }
    void reset();

    void process (double bpm,
                  double ppqPosition,
                  bool isPlaying,
                  int numSamples,
                  const juce::MidiKeyboardState& keyboardState,
                  std::vector<Event>& outEvents);

    std::atomic<bool>  enabled   { false };
    std::atomic<int>   mode      { 0 };       // 0..6
    std::atomic<int>   octaves   { 1 };       // 1..4
    std::atomic<int>   rateIndex { 4 };       // 0..12 (1/1 ... 1/64)
    std::atomic<float> gate      { 0.5f };    // 0.1..1.0
    std::atomic<float> swing     { 0.0f };    // 0..1
    std::atomic<bool>  latch     { false };

    std::atomic<int> currentStepIndex { -1 };

    juce::ValueTree toValueTree() const;
    void fromValueTree (const juce::ValueTree& v);

private:
    std::vector<int> buildNoteList (const juce::MidiKeyboardState& state);
    int getSequenceLength (int numBaseNotes) const;
    int getNoteAtIndex (const std::vector<int>& baseNotes, int seqIndex) const;

    double currentSampleRate = 44100.0;

    std::array<bool, 128> latchedNotes {};
    std::array<int, 128>  playOrder {};
    int  playOrderCount       = 0;
    int  currentNote           = -1;
    int  samplesUntilNoteOff   = -1;
    int  currentSequenceIndex  = 0;
    int  upDownDirection       = 1;

    // Notas del acorde activas (modo CHORD)
    std::vector<int> chordNotesHeld;

    bool anyNoteHeldPrevBlock  = false;
};
