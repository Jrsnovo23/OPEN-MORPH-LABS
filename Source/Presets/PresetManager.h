#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace presets
{
    struct PresetInfo
    {
        juce::String name;
        juce::String category;
        juce::String author;
        bool         isFactory = false;
        juce::File   file;      // vacío si es factory
    };

    class PresetManager
    {
    public:
        explicit PresetManager (juce::AudioProcessorValueTreeState& apvts);

        const juce::Array<PresetInfo>& getAllPresets() const { return allPresets; }

        void loadByIndex (int index);
        void loadFromFile (const juce::File& f);

        void next();
        void prev();

        bool saveUserPreset (const juce::String& name, const juce::String& category);

        juce::String getCurrentName() const;
        juce::String getCurrentCategory() const;
        int          getCurrentIndex() const { return currentIndex; }

        void refresh();

        juce::File getUserPresetDirectory() const;

        // ===== SISTEMA DE FAVORITOS =====
        bool isFavorite (const juce::String& presetName) const;
        void toggleFavorite (const juce::String& presetName);

        bool isFavoritesOnly() const { return favoritesOnly; }
        void setFavoritesOnly (bool v) { favoritesOnly = v; }

        // FASE 7: obtener índices favoritos para el menú popup.
        juce::Array<int> getFavoriteIndices() const;

    private:
        void applyPreset (const juce::var& preset);
        juce::var captureCurrentState() const;
        void loadFactoryPresets();

        void loadFavorites();
        void saveFavorites();
        juce::File getFavoritesFile() const;

        juce::AudioProcessorValueTreeState& apvts;
        juce::Array<PresetInfo> allPresets;
        int currentIndex = 0;

        // ===== FAVORITOS =====
        juce::StringArray favorites;
        bool favoritesOnly = false;
    };
}
