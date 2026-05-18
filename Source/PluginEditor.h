#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Custom LookAndFeel for premium hardware aesthetic (red/gold glowing theme)
class QuantumLookAndFeel : public juce::LookAndFeel_V4
{
public:
    QuantumLookAndFeel();
    
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font getLabelFont (juce::Label& label) override;

private:
    juce::Colour redMetal { 0xFF9B1E1E };
    juce::Colour darkBg { 0xFF1A1A1F };
    juce::Colour goldGlow { 0xFFFFD700 };
    juce::Colour cyanGlow { 0xFF00F0FF };
};

//==============================================================================
// The interactive centerpiece: Beautiful audio-reactive visualizer
class ReverbVisualiser : public juce::Component,
                         private juce::Timer
{
public:
    ReverbVisualiser (QuantumReverbAudioProcessor& proc);
    ~ReverbVisualiser() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

    void setMode (int newMode);

private:
    void timerCallback() override;
    void updateFromProcessor();
    void spawnParticles (int count, float intensity);
    void updateParticles();

    QuantumReverbAudioProcessor& processor;

    struct Particle
    {
        float x = 0, y = 0;
        float vx = 0, vy = 0;
        float life = 0;
        float size = 1.5f;
        juce::Colour col { juce::Colours::cyan };
    };

    std::vector<Particle> particles;
    juce::Random rng;

    float currentInputLevel = 0.0f;
    float currentOutputLevel = 0.0f;
    float smoothedLevel = 0.0f;

    int vizMode = 0;
    int animationFrame = 0;

    juce::Path waveformPath;
    juce::ColourGradient glowGradient;
};

//==============================================================================
// Main Editor
class QuantumReverbAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    QuantumReverbAudioProcessorEditor (QuantumReverbAudioProcessor&);
    ~QuantumReverbAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    QuantumReverbAudioProcessor& audioProcessor;

    QuantumLookAndFeel customLookAndFeel;

    ReverbVisualiser visualiser;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preDelayAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sizeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> diffusionAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modDepthAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modRateAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowCutAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highCutAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttach;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> freezeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> shimmerAttach;

    juce::Slider preDelaySlider, decaySlider, sizeSlider, dampingSlider;
    juce::Slider diffusionSlider, modDepthSlider, modRateSlider, mixSlider;
    juce::Slider lowCutSlider, highCutSlider, outputGainSlider;

    juce::ToggleButton freezeButton, shimmerButton, exciteButton;

    juce::Label titleLabel;

    juce::TextButton vizModeButtons[3];

    void setupSlider (juce::Slider& slider, const juce::String& name, const juce::String& suffix);
    void setupButton (juce::ToggleButton& button, const juce::String& text);
    void updateVizMode (int mode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuantumReverbAudioProcessorEditor)
};