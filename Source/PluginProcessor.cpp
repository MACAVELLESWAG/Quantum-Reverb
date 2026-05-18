#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
QuantumReverbAudioProcessor::QuantumReverbAudioProcessor()
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    preDelaySmoothed.reset (100);
    decaySmoothed.reset (100);
    sizeSmoothed.reset (100);
    dampingSmoothed.reset (100);
    diffusionSmoothed.reset (100);
    modDepthSmoothed.reset (100);
    modRateSmoothed.reset (100);
    mixSmoothed.reset (100);
    lowCutSmoothed.reset (100);
    highCutSmoothed.reset (100);
    outputGainSmoothed.reset (100);
}

QuantumReverbAudioProcessor::~QuantumReverbAudioProcessor()
{
}

//==============================================================================
const juce::String QuantumReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool QuantumReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool QuantumReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool QuantumReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double QuantumReverbAudioProcessor::getTailLengthSeconds() const
{
    return 30.0;
}

int QuantumReverbAudioProcessor::getNumPrograms()
{
    return 1;
}

int QuantumReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void QuantumReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String QuantumReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void QuantumReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}
//==============================================================================
void QuantumReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    reverbEngine.prepare (sampleRate, samplesPerBlock);
    
    auto sr = sampleRate;
    preDelaySmoothed.reset (sr, 0.05);
    decaySmoothed.reset (sr, 0.1);
    sizeSmoothed.reset (sr, 0.05);
    dampingSmoothed.reset (sr, 0.05);
    diffusionSmoothed.reset (sr, 0.05);
    modDepthSmoothed.reset (sr, 0.05);
    modRateSmoothed.reset (sr, 0.05);
    mixSmoothed.reset (sr, 0.02);
    lowCutSmoothed.reset (sr, 0.05);
    highCutSmoothed.reset (sr, 0.05);
    outputGainSmoothed.reset (sr, 0.02);
}

void QuantumReverbAudioProcessor::releaseResources()
{
    reverbEngine.reset();
}

//==============================================================================
bool QuantumReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void QuantumReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (auto* p = apvts.getRawParameterValue("preDelay"))  preDelaySmoothed.setTargetValue (p->load());
    if (auto* p = apvts.getRawParameterValue("decay"))     decaySmoothed.setTargetValue (p->load());
    if (auto* p = apvts.getRawParameterValue("size"))      sizeSmoothed.setTargetValue (p->load());
    if (auto* p = apvts.getRawParameterValue("damping"))   dampingSmoothed.setTargetValue (p->load());
    if (auto* p = apvts.getRawParameterValue("diffusion")) diffusionSmoothed.setTargetValue (p->load());
    if (auto* p = apvts.getRawParameterValue("modDepth"))  modDepthSmoothed.setTargetValue (p->load());
    if (auto* p = apvts.getRawParameterValue("modRate"))   modRateSmoothed.setTargetValue (p->load());
    if (auto* p = apvts.getRawParameterValue("mix"))       mixSmoothed.setTargetValue (p->load());
    if (auto* p = apvts.getRawParameterValue("lowCut"))    lowCutSmoothed.setTargetValue (p->load());
    if (auto* p = apvts.getRawParameterValue("highCut"))   highCutSmoothed.setTargetValue (p->load());
    if (auto* p = apvts.getRawParameterValue("outputGain"))outputGainSmoothed.setTargetValue (juce::Decibels::decibelsToGain (p->load()));

    reverbEngine.setPreDelayMs (preDelaySmoothed.getNextValue());
    reverbEngine.setDecaySeconds (decaySmoothed.getNextValue());
    reverbEngine.setSize (sizeSmoothed.getNextValue());
    reverbEngine.setDamping (dampingSmoothed.getNextValue());
    reverbEngine.setDiffusion (diffusionSmoothed.getNextValue());
    reverbEngine.setModDepth (modDepthSmoothed.getNextValue());
    reverbEngine.setModRateHz (modRateSmoothed.getNextValue());
    reverbEngine.setMix (mixSmoothed.getNextValue());
    reverbEngine.setLowCutHz (lowCutSmoothed.getNextValue());
    reverbEngine.setHighCutHz (highCutSmoothed.getNextValue());
    
    if (auto* p = apvts.getRawParameterValue("freeze"))
        reverbEngine.setFreeze (p->load() > 0.5f);
    if (auto* p = apvts.getRawParameterValue("shimmer"))
        reverbEngine.setShimmer (p->load() > 0.5f);

    float inLevel = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        inLevel = juce::jmax (inLevel, buffer.getRMSLevel (ch, 0, buffer.getNumSamples()));
    inputLevel.store (inLevel);

    reverbEngine.process (buffer);

    float outLevel = 0.0f;
    for (int ch = 0; ch < totalNumOutputChannels; ++ch)
        outLevel = juce::jmax (outLevel, buffer.getRMSLevel (ch, 0, buffer.getNumSamples()));
    outputLevel.store (outLevel);

    outputGain.setGainLinear (outputGainSmoothed.getNextValue());
    juce::dsp::AudioBlock<float> block (buffer);
    outputGain.process (juce::dsp::ProcessContextReplacing<float> (block));
}

void QuantumReverbAudioProcessor::triggerImpulse() noexcept
{
    reverbEngine.triggerImpulse();
}

//==============================================================================
bool QuantumReverbAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* QuantumReverbAudioProcessor::createEditor()
{
    return new QuantumReverbAudioProcessorEditor (*this);
}

//==============================================================================
void QuantumReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void QuantumReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout QuantumReverbAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("preDelay", "Pre Delay", 
        juce::NormalisableRange<float> (0.0f, 200.0f, 0.1f), 25.0f, "ms"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("decay", "Decay", 
        juce::NormalisableRange<float> (0.3f, 25.0f, 0.01f, 0.4f), 4.5f, "s"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("size", "Size", 
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.75f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("damping", "Damping", 
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.35f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("diffusion", "Diffusion", 
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.65f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("modDepth", "Mod Depth", 
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.22f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("modRate", "Mod Rate", 
        juce::NormalisableRange<float> (0.1f, 6.0f, 0.01f), 1.2f, "Hz"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("mix", "Mix", 
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.55f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lowCut", "Low Cut", 
        juce::NormalisableRange<float> (20.0f, 500.0f, 1.0f, 0.3f), 80.0f, "Hz"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("highCut", "High Cut", 
        juce::NormalisableRange<float> (2000.0f, 20000.0f, 1.0f, 0.3f), 8500.0f, "Hz"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("outputGain", "Output", 
        juce::NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f, "dB"));

    params.push_back (std::make_unique<juce::AudioParameterBool> ("freeze", "Freeze", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("shimmer", "Shimmer", false));

    return { params.begin(), params.end() };
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new QuantumReverbAudioProcessor();
}