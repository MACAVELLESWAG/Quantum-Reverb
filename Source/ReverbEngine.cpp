#include "ReverbEngine.h"

ReverbEngine::ReverbEngine()
{
}

void ReverbEngine::prepare (double newSampleRate, int newBlockSize)
{
    sampleRate = newSampleRate;
    blockSize = newBlockSize;

    preDelay.prepare ({ sampleRate, (juce::uint32) blockSize, 2 });
    preDelay.setMaximumDelayInSamples (int(sampleRate * 0.6));

    for (int i = 0; i < numAllpasses; ++i)
    {
        inputAllpasses[i].prepare ({ sampleRate, (juce::uint32) blockSize, 2 });
        inputAllpasses[i].setMaximumDelayInSamples (int(sampleRate * 0.1));
    }

    const float baseDelaysMs[numCombs] = { 29.7f, 37.1f, 41.1f, 43.7f };
    
    for (int i = 0; i < numCombs; ++i)
    {
        combs[i].delayLine.prepare ({ sampleRate, (juce::uint32) blockSize, 2 });
        combs[i].delayLine.setMaximumDelayInSamples (int(sampleRate * 0.2));
        combs[i].baseDelaySamples = baseDelaysMs[i] * 0.001f * (float)sampleRate;
        
        combs[i].dampingFilter.prepare ({ sampleRate });
        combs[i].dampingFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 8000.0f);
    }

    for (int i = 0; i < numAllpasses; ++i)
    {
        outputAllpasses[i].prepare ({ sampleRate, (juce::uint32) blockSize, 2 });
        outputAllpasses[i].setMaximumDelayInSamples (int(sampleRate * 0.05));
    }

    lowCutFilter.prepare ({ sampleRate });
    highCutFilter.prepare ({ sampleRate });
    
    lowCutFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, lowCutHz);
    highCutFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, highCutHz);

    lfo.prepare ({ sampleRate });
    lfo.initialise ([](float x) { return std::sin (x); }, 128);

    reset();
}

void ReverbEngine::reset()
{
    preDelay.reset();
    for (auto& ap : inputAllpasses) ap.reset();
    for (auto& c : combs) { c.delayLine.reset(); c.dampingFilter.reset(); }
    for (auto& ap : outputAllpasses) ap.reset();
    lowCutFilter.reset();
    highCutFilter.reset();
    lfo.reset();
    
    impulseCountdown = 0;
}

void ReverbEngine::setPreDelayMs (float ms)
{
    preDelaySamples = juce::jlimit (0.0f, 500.0f, ms) * 0.001f * (float)sampleRate;
}

void ReverbEngine::setDecaySeconds (float seconds)
{
    decaySeconds = juce::jlimit (0.2f, 60.0f, seconds);
    updateCombDelaysAndFeedback();
}

void ReverbEngine::setSize (float size01)
{
    size = juce::jlimit (0.0f, 1.0f, size01);
    updateCombDelaysAndFeedback();
}

void ReverbEngine::setDamping (float damping01)
{
    damping = juce::jlimit (0.0f, 1.0f, damping01);
    updateCombDelaysAndFeedback();
}

void ReverbEngine::setDiffusion (float diffusion01)
{
    diffusion = juce::jlimit (0.0f, 1.0f, diffusion01);
}

void ReverbEngine::setModDepth (float depth01)
{
    modDepth = juce::jlimit (0.0f, 1.0f, depth01);
}

void ReverbEngine::setModRateHz (float rateHz)
{
    modRate = juce::jlimit (0.05f, 8.0f, rateHz);
    lfo.setFrequency (modRate);
}

void ReverbEngine::setMix (float wet01)
{
    wetMix = juce::jlimit (0.0f, 1.0f, wet01);
    dryMix = 1.0f - wetMix;
}

void ReverbEngine::setLowCutHz (float hz)
{
    lowCutHz = juce::jlimit (20.0f, 2000.0f, hz);
    lowCutFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, lowCutHz);
}

void ReverbEngine::setHighCutHz (float hz)
{
    highCutHz = juce::jlimit (2000.0f, 20000.0f, hz);
    highCutFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, highCutHz);
}

void ReverbEngine::setFreeze (bool freeze)
{
    freezeMode = freeze;
    updateCombDelaysAndFeedback();
}

void ReverbEngine::setShimmer (bool shimmer)
{
    shimmerMode = shimmer;
}

void ReverbEngine::triggerImpulse()
{
    impulseCountdown = impulseLength;
}

void ReverbEngine::updateCombDelaysAndFeedback()
{
    const float baseFeedback = std::pow (0.001f, 1.0f / (decaySeconds * (float)sampleRate / 44100.0f));
    
    for (int i = 0; i < numCombs; ++i)
    {
        float scaledBase = combs[i].baseDelaySamples * (0.6f + size * 1.4f);
        combs[i].baseDelaySamples = scaledBase;
        
        float fb = freezeMode ? 0.999f : baseFeedback * (0.85f + damping * 0.1f);
        combs[i].feedback = juce::jlimit(0.3f, 0.999f, fb);
        
        float dampFreq = 2000.0f + (1.0f - damping) * 14000.0f;
        combs[i].dampingFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, dampFreq);
    }
    
    currentDecay = decaySeconds;
}

void ReverbEngine::updateParameters()
{
}

void ReverbEngine::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    lfo.setFrequency (modRate);

    juce::AudioBuffer<float> wetBuffer (numChannels, numSamples);
    wetBuffer.clear();

    for (int ch = 0; ch < numChannels; ++ch)
        wetBuffer.copyFrom (ch, 0, buffer, ch, 0, numSamples);

    if (preDelaySamples > 1.0f)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = wetBuffer.getWritePointer (ch);
            for (int i = 0; i < numSamples; ++i)
            {
                preDelay.pushSample (ch, data[i]);
                data[i] = preDelay.popSample (ch, preDelaySamples);
            }
        }
    }

    const float diffAmt = diffusion * 0.8f;
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = wetBuffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float x = data[i];
            for (int ap = 0; ap < numAllpasses; ++ap)
            {
                float delayed = inputAllpasses[ap].popSample (ch);
                float y = x + inputAllpassCoeffs[ap] * delayed;
                inputAllpasses[ap].pushSample (ch, x - inputAllpassCoeffs[ap] * y);
                x = y;
            }
            data[i] = x * (0.5f + diffAmt * 0.5f);
        }
    }

    if (impulseCountdown > 0)
    {
        const float impulseAmp = 0.8f;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = wetBuffer.getWritePointer (ch);
            for (int i = 0; i < numSamples && impulseCountdown > 0; ++i)
            {
                if (impulseCountdown > impulseLength / 2)
                    data[i] += impulseAmp * (impulseCountdown % 2 == 0 ? 1.0f : -1.0f);
                --impulseCountdown;
            }
        }
    }

    juce::AudioBuffer<float> combSum (numChannels, numSamples);
    combSum.clear();

    const float modAmount = modDepth * 8.0f;

    for (int c = 0; c < numCombs; ++c)
    {
        auto& comb = combs[c];
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* inData = wetBuffer.getReadPointer (ch);
            auto* outData = combSum.getWritePointer (ch);

            for (int i = 0; i < numSamples; ++i)
            {
                float lfoVal = lfo.processSample (0.0f);
                float modulatedDelay = comb.baseDelaySamples + (lfoVal * modAmount * (c + 1) * 0.3f);
                modulatedDelay = juce::jmax (10.0f, modulatedDelay);

                float delayed = comb.delayLine.popSample (ch, modulatedDelay);
                
                float damped = comb.dampingFilter.processSample (delayed);
                
                float feedbackSample = damped * comb.feedback;
                
                if (shimmerMode)
                    feedbackSample *= 1.15f;

                float inputWithFeedback = inData[i] * 0.6f + feedbackSample;
                
                comb.delayLine.pushSample (ch, inputWithFeedback);
                
                outData[i] += delayed * (1.0f / numCombs);
            }
        }
    }

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = combSum.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float x = data[i];
            for (int ap = 0; ap < numAllpasses; ++ap)
            {
                float delayed = outputAllpasses[ap].popSample (ch);
                float y = x + outputAllpassCoeffs[ap] * delayed;
                outputAllpasses[ap].pushSample (ch, x - outputAllpassCoeffs[ap] * y);
                x = y;
            }
            data[i] = x;
        }
    }

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* wetData = combSum.getWritePointer (ch);
        auto* dryData = buffer.getWritePointer (ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float wet = wetData[i];
            wet = lowCutFilter.processSample (wet);
            wet = highCutFilter.processSample (wet);

            float mixed = dryData[i] * dryMix + wet * wetMix;
            dryData[i] = mixed;
        }
    }
}