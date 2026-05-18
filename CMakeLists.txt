cmake_minimum_required(VERSION 3.22)

project(QuantumReverb VERSION 1.0.0)

# JUCE setup - user should set JUCE_DIR or have JUCE in path
# Recommended: JUCE 7.0.0 or newer
find_package(JUCE CONFIG REQUIRED)

juce_add_plugin(QuantumReverb
    COMPANY_NAME "vibe-weaver"
    BUNDLE_ID "com.vibeweaver.quantumreverb"
    PLUGIN_MANUFACTURER_CODE "VibW"
    PLUGIN_CODE "Qrev"
    FORMATS VST3 AU
    PRODUCT_NAME "Quantum Reverb"
    DESCRIPTION "Premium aesthetic reverb with interactive visualizer"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(QuantumReverb
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(QuantumReverb
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(QuantumReverb
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)