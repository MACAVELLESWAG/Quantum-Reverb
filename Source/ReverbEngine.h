#pragma once

#include <JuceHeader.h>

/**
 * @brief High-quality, real-time safe algorithmic reverb engine.
 * 
 * Features:
 * - Pre-delay
 * - Input diffusion (allpass chain)
 * - Modulated parallel comb filters (Schroeder-inspired with FDN elements)
 * - Output diffusion
 * - Per-comb damping filters
 * - LFO modulation on comb delay times for lush movement
 * - Freeze mode (infinite sustain)
 * - Shimmer mode (simple ethereal high-frequency feedback lift)
 * 
 * All processing is allocation-free after prepare().
 */
class ReverbEngine
{
public:
    ReverbEngine();
    ~ReverbEngine() = default;

    void prepare (double sampleRate, int samplesPerBlock);
    void reset();

    void process (juce::AudioBuffer<float>& buffer);

    // Parameter setters (called from smoothed values in processor)
    void setPreDelayMs (float ms);
    void setDecaySeconds (float seconds);
    void setSize (float size01);
    void setDamping (float damping01);
    void setDiffusion (float diffusion01);
    void setModDepth (float depth01);
    void setModRateHz (float rateHz);
    void setMix (float wet01);
    void setLowCutHz (float hz);
    void setHighCutHz (float hz);
    void setFreeze (bool freeze);
    void setShimmer (bool shimmer);

    void triggerImpulse();

    // For visualizer feedback (optional advanced)
    float getCurrentDecayEstimate() const noexcept { return currentDecay; }

private:
    double sampleRate = 44100.0;
    int blockSize = 512;

    static constexpr int numCombs = 4;
    static constexpr int numAllpasses = 2;

    juce::dsp::DelayLine<float> preDelay { 44100 * 2 };

    juce::dsp::DelayLine<float> inputAllpasses[numAllpasses];
    float inputAllpassCoeffs[numAllpasses] = { 0.7f, 0.5f };

    struct CombFilter
    {
        juce::dsp::DelayLine<float> delayLine { 44100 };
        juce::dsp::IIR::Filter<float> dampingFilter;
        float feedback = 0.7f;
        float baseDelaySamples = 0.0f;
        float currentDelaySamples = 0.0f;
    };
    CombFilter combs[numCombs];

    juce::dsp::DelayLine<float> outputAllpasses[numAllpasses];
    float outputAllpassCoeffs[numAllpasses] = { 0.6f, 0.4f };

    juce::dsp::IIR::Filter<float> lowCutFilter;
    juce::dsp::IIR::Filter<float> highCutFilter;

    juce::dsp::Oscillator<float> lfo;

    float preDelaySamples = 0.0f;
    float decaySeconds = 3.0f;
    float size = 0.7f;
    float damping = 0.4f;
    float diffusion = 0.6f;
    float modDepth = 0.15f;
    float modRate = 0.8f;
    float wetMix = 0.5f;
    float dryMix = 0.5f;
    float lowCutHz = 40.0f;
    float highCutHz = 12000.0f;
    bool freezeMode = false;
    bool shimmerMode = false;

    float currentDecay = 3.0f;

    int impulseCountdown = 0;
    static constexpr int impulseLength = 64;

    void updateParameters();
    void updateCombDelaysAndFeedback();
};