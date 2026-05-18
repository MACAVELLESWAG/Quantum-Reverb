#pragma once

#include <JuceHeader.h>
#include "ReverbEngine.h"

// Forward declare for editor
class QuantumReverbAudioProcessorEditor;

//==============================================================================
class QuantumReverbAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    QuantumReverbAudioProcessor();
    ~QuantumReverbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Public for editor access
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    
    // Real-time safe level access for visualizer
    float getInputLevel() const noexcept { return inputLevel.load(); }
    float getOutputLevel() const noexcept { return outputLevel.load(); }
    
    // Trigger impulse from GUI (interactive)
    void triggerImpulse() noexcept;

    // Reverb engine access (for advanced viz if needed)
    ReverbEngine& getReverbEngine() { return reverbEngine; }

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    ReverbEngine reverbEngine;
    
    // Smoothed parameters for real-time safety and zip-free automation
    juce::SmoothedValue<float> preDelaySmoothed;
    juce::SmoothedValue<float> decaySmoothed;
    juce::SmoothedValue<float> sizeSmoothed;
    juce::SmoothedValue<float> dampingSmoothed;
    juce::SmoothedValue<float> diffusionSmoothed;
    juce::SmoothedValue<float> modDepthSmoothed;
    juce::SmoothedValue<float> modRateSmoothed;
    juce::SmoothedValue<float> mixSmoothed;
    juce::SmoothedValue<float> lowCutSmoothed;
    juce::SmoothedValue<float> highCutSmoothed;
    juce::SmoothedValue<float> outputGainSmoothed;

    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };
    
    juce::dsp::Gain<float> inputGain;
    juce::dsp::Gain<float> outputGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuantumReverbAudioProcessor)
};