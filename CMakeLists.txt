cmake_minimum_required(VERSION 3.22)

project(PPGWave3Clone VERSION 0.4.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "" FORCE)
endif()

add_subdirectory(JUCE)

juce_add_plugin(PPGWave3Clone
    COMPANY_NAME                "OpenWave Audio"
    PLUGIN_MANUFACTURER_CODE    Opwv
    PLUGIN_CODE                 Pw33
    FORMATS                     VST3 AU Standalone
    PRODUCT_NAME                "PPG Wave 3 Clone"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES             "Instrument" "Synth"
    AU_MAIN_TYPE                kAudioUnitType_MusicDevice
    COPY_PLUGIN_AFTER_BUILD     FALSE
)

target_sources(PPGWave3Clone
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Params/ParameterLayout.cpp
        Source/DSP/Wavetable.cpp
        Source/DSP/Oscillator.cpp
        Source/DSP/Filter.cpp
        Source/DSP/LFO.cpp
        Source/DSP/Effects.cpp
        Source/Synth/SynthVoice.cpp
        Source/UI/PPGLookAndFeel.cpp
        Source/UI/Visualizers.cpp
        Source/Presets/PresetManager.cpp
)

target_include_directories(PPGWave3Clone PRIVATE Source)

target_compile_definitions(PPGWave3Clone
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
)

target_link_libraries(PPGWave3Clone
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

if (APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    target_compile_options(PPGWave3Clone PRIVATE -mno-avx -mno-avx2)
endif()
