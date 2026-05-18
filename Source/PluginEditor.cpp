#include "PluginEditor.h"

//==============================================================================
// QuantumLookAndFeel Implementation
QuantumLookAndFeel::QuantumLookAndFeel()
{
    setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xFFFFD700));
    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xFF3A3A3A));
    setColour (juce::ToggleButton::textColourId, juce::Colours::white);
}

void QuantumLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                           juce::Slider& slider)
{
    auto radius = (float) juce::jmin (width / 2, height / 2) - 6.0f;
    auto centreX = (float) x + (float) width * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour (juce::Colour (0x40FFD700));
    g.drawEllipse (rx - 3, ry - 3, rw + 6, rw + 6, 3.0f);

    g.setColour (juce::Colour (0xFF2A2A2A));
    g.fillEllipse (rx, ry, rw, rw);

    juce::ColourGradient gradient (juce::Colour (0xFF4A4A4A), rx, ry,
                                     juce::Colour (0xFF1A1A1A), rx + rw, ry + rw, false);
    g.setGradientFill (gradient);
    g.fillEllipse (rx + 2, ry + 2, rw - 4, rw - 4);

    g.setColour (juce::Colour (0xFF111111));
    g.drawEllipse (rx + 6, ry + 6, rw - 12, rw - 12, 2.0f);

    juce::Path p;
    const float pointerLength = radius * 0.65f;
    const float pointerThickness = 3.5f;
    p.addRectangle (-pointerThickness * 0.5f, -radius * 0.9f, pointerThickness, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));

    g.setColour (juce::Colour (0x80FFD700));
    g.fillPath (p);
    g.setColour (juce::Colour (0xFFFFD700));
    g.fillPath (p);

    g.setColour (juce::Colours::white);
    g.fillEllipse (centreX - 3, centreY - 3, 6, 6);
}

void QuantumLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);

    bool isOn = button.getToggleState();

    if (isOn)
    {
        g.setColour (juce::Colour (0xFF1A3A1A));
        g.fillRoundedRectangle (bounds, 6.0f);
        
        g.setColour (juce::Colour (0x40FFD700));
        g.drawRoundedRectangle (bounds.expanded (3), 8.0f, 2.0f);
        g.setColour (juce::Colour (0xFFFFD700));
        g.drawRoundedRectangle (bounds, 6.0f, 1.5f);
    }
    else
    {
        g.setColour (juce::Colour (0xFF2A2A2A));
        g.fillRoundedRectangle (bounds, 6.0f);
        g.setColour (juce::Colour (0xFF555555));
        g.drawRoundedRectangle (bounds, 6.0f, 1.0f);
    }

    g.setColour (isOn ? juce::Colour (0xFFFFD700) : juce::Colours::lightgrey);
    g.setFont (juce::Font (12.0f, juce::Font::bold));
    g.drawText (button.getButtonText(), bounds, juce::Justification::centred, true);
}

juce::Font QuantumLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font (11.0f, juce::Font::bold);
}

//==============================================================================
// ReverbVisualiser Implementation
ReverbVisualiser::ReverbVisualiser (QuantumReverbAudioProcessor& proc)
    : processor (proc)
{
    setOpaque (false);
    startTimerHz (45);
    particles.reserve (180);
    
    for (int i = 0; i < 25; ++i)
        spawnParticles (1, 0.3f);
}

ReverbVisualiser::~ReverbVisualiser()
{
    stopTimer();
}

void ReverbVisualiser::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto centre = bounds.getCentre();

    g.setColour (juce::Colour (0xFF0A0A12));
    g.fillRoundedRectangle (bounds, 12.0f);

    g.setColour (juce::Colour (0x30FFD700));
    g.drawRoundedRectangle (bounds.reduced (2), 12.0f, 2.5f);
    g.setColour (juce::Colour (0x2000F0FF));
    g.drawRoundedRectangle (bounds.reduced (5), 10.0f, 1.5f);

    g.setColour (juce::Colour (0x15FFFFFF));
    for (int i = 1; i < 6; ++i)
    {
        float y = bounds.getY() + bounds.getHeight() * (i / 6.0f);
        g.drawHorizontalLine ((int)y, bounds.getX() + 15, bounds.getRight() - 15);
        
        float x = bounds.getX() + bounds.getWidth() * (i / 6.0f);
        g.drawVerticalLine ((int)x, bounds.getY() + 15, bounds.getBottom() - 15);
    }

    g.setColour (juce::Colour (0xFFFFD700));
    
    juce::Path wavePath;
    const float waveY = centre.y - 20;
    const float amp = 18.0f + smoothedLevel * 45.0f;
    
    wavePath.startNewSubPath (bounds.getX() + 30, waveY);
    
    for (int i = 0; i < 120; ++i)
    {
        float x = bounds.getX() + 30 + (i / 119.0f) * (bounds.getWidth() - 60);
        float phase = (animationFrame * 0.08f) + (i * 0.12f);
        float y = waveY + std::sin (phase) * amp * (0.6f + 0.4f * std::sin (i * 0.03f));
        wavePath.lineTo (x, y);
    }
    
    g.setColour (juce::Colour (0x60FFD700));
    g.strokePath (wavePath, juce::PathStrokeType (4.5f));
    g.setColour (juce::Colour (0xFFFFD700));
    g.strokePath (wavePath, juce::PathStrokeType (2.0f));

    for (const auto& p : particles)
    {
        if (p.life <= 0.01f) continue;
        
        float alpha = juce::jlimit (0.0f, 1.0f, p.life);
        juce::Colour c = p.col.withAlpha (alpha * 0.9f);
        
        g.setColour (c.withAlpha (alpha * 0.25f));
        g.fillEllipse (p.x - p.size * 2.2f, p.y - p.size * 2.2f, p.size * 4.4f, p.size * 4.4f);
        
        g.setColour (c);
        g.fillEllipse (p.x - p.size * 0.5f, p.y - p.size * 0.5f, p.size, p.size);
    }

    g.setColour (juce::Colour (0xFFFFD700));
    g.setFont (juce::Font (18.0f, juce::Font::bold));
    g.drawText ("QUANTUM REVERB", bounds.removeFromTop(38).reduced(20, 0), juce::Justification::centred, false);

    g.setFont (juce::Font (10.0f));
    g.setColour (juce::Colours::lightgrey.withAlpha (0.6f));
    g.drawText ("Click to Excite  •  Interactive Space", bounds.removeFromBottom(22), juce::Justification::centred);

    juce::String modeText = (vizMode == 0) ? "PARTICLES + WAVE" : (vizMode == 1 ? "ENERGY FIELD" : "DEEP SPACE");
    g.drawText (modeText, bounds.removeFromBottom(18), juce::Justification::centred);
}

void ReverbVisualiser::resized()
{
}

void ReverbVisualiser::mouseDown (const juce::MouseEvent& e)
{
    processor.triggerImpulse();
    
    float intensity = 0.6f + currentOutputLevel * 1.2f;
    spawnParticles (28 + (int)(intensity * 12), intensity);
}

void ReverbVisualiser::mouseDrag (const juce::MouseEvent& e)
{
}

void ReverbVisualiser::timerCallback()
{
    updateFromProcessor();
    updateParticles();
    animationFrame = (animationFrame + 1) % 10000;
    
    if (rng.nextFloat() < 0.35f)
        spawnParticles (1 + (int)(smoothedLevel * 3), 0.25f);
    
    repaint();
}

void ReverbVisualiser::updateFromProcessor()
{
    currentInputLevel = processor.getInputLevel();
    currentOutputLevel = processor.getOutputLevel();
    
    smoothedLevel = smoothedLevel * 0.85f + (currentOutputLevel * 1.4f) * 0.15f;
    smoothedLevel = juce::jlimit (0.0f, 1.0f, smoothedLevel);
}

void ReverbVisualiser::spawnParticles (int count, float intensity)
{
    auto b = getLocalBounds().toFloat();
    
    for (int i = 0; i < count; ++i)
    {
        Particle p;
        p.x = b.getCentreX() + rng.nextFloat() * 40.0f - 20.0f;
        p.y = b.getCentreY() + rng.nextFloat() * 30.0f - 15.0f;
        
        float vel = 0.8f + intensity * 2.2f;
        p.vx = (rng.nextFloat() - 0.5f) * vel;
        p.vy = (rng.nextFloat() - 0.5f) * vel * 0.7f - 0.3f;
        
        p.life = 0.65f + rng.nextFloat() * 0.6f;
        p.size = 1.2f + intensity * 1.8f;
        
        if (rng.nextFloat() > 0.5f)
            p.col = juce::Colour (0xFFFFD700);
        else
            p.col = juce::Colour (0xFF00E0FF);
        
        particles.push_back (p);
    }
    
    while (particles.size() > 160)
        particles.erase (particles.begin());
}

void ReverbVisualiser::updateParticles()
{
    for (auto& p : particles)
    {
        if (p.life <= 0) continue;
        
        p.x += p.vx;
        p.y += p.vy;
        
        p.vy += 0.015f;
        p.vx *= 0.985f;
        p.vy *= 0.985f;
        
        p.life -= 0.018f;
        
        if (p.life < 0.3f)
            p.size *= 0.96f;
    }
    
    particles.erase (std::remove_if (particles.begin(), particles.end(), 
        [](const Particle& p){ return p.life <= 0.01f; }), particles.end());
}

void ReverbVisualiser::setMode (int newMode)
{
    vizMode = newMode;
    repaint();
}

//==============================================================================
// QuantumReverbAudioProcessorEditor Implementation
QuantumReverbAudioProcessorEditor::QuantumReverbAudioProcessorEditor (QuantumReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), visualiser (p)
{
    setLookAndFeel (&customLookAndFeel);
    
    setSize (920, 560);
    setResizable (false, false);

    titleLabel.setText ("QUANTUM REVERB", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (26.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xFFFFD700));
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    addAndMakeVisible (visualiser);

    setupSlider (preDelaySlider, "PRE DELAY", "ms");
    setupSlider (decaySlider, "DECAY", "s");
    setupSlider (sizeSlider, "SIZE", "");
    setupSlider (dampingSlider, "DAMPING", "");

    preDelayAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "preDelay", preDelaySlider);
    decayAttach    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "decay", decaySlider);
    sizeAttach     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "size", sizeSlider);
    dampingAttach  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "damping", dampingSlider);

    setupSlider (diffusionSlider, "DIFFUSION", "");
    setupSlider (modDepthSlider, "MOD DEPTH", "");
    diffusionAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "diffusion", diffusionSlider);
    modDepthAttach  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "modDepth", modDepthSlider);

    setupSlider (modRateSlider, "MOD RATE", "Hz");
    setupSlider (mixSlider, "MIX", "");
    modRateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "modRate", modRateSlider);
    mixAttach     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "mix", mixSlider);

    setupSlider (lowCutSlider, "LOW CUT", "Hz");
    setupSlider (highCutSlider, "HIGH CUT", "Hz");
    setupSlider (outputGainSlider, "OUTPUT", "dB");
    lowCutAttach   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "lowCut", lowCutSlider);
    highCutAttach  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "highCut", highCutSlider);
    outputGainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "outputGain", outputGainSlider);

    setupButton (freezeButton, "FREEZE");
    setupButton (shimmerButton, "SHIMMER");
    freezeAttach  = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.getAPVTS(), "freeze", freezeButton);
    shimmerAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.getAPVTS(), "shimmer", shimmerButton);

    exciteButton.setButtonText ("EXCITE");
    exciteButton.setClickingTogglesState (false);
    exciteButton.onClick = [this] { audioProcessor.triggerImpulse(); visualiser.repaint(); };
    addAndMakeVisible (exciteButton);

    vizModeButtons[0].setButtonText ("WAVE");
    vizModeButtons[1].setButtonText ("FIELD");
    vizModeButtons[2].setButtonText ("SPACE");
    for (int i = 0; i < 3; ++i)
    {
        vizModeButtons[i].setClickingTogglesState (true);
        vizModeButtons[i].setRadioGroupId (1001);
        vizModeButtons[i].onClick = [this, i] { updateVizMode(i); };
        addAndMakeVisible (vizModeButtons[i]);
    }
    vizModeButtons[0].setToggleState (true, juce::dontSendNotification);

    for (auto* s : { &preDelaySlider, &decaySlider, &sizeSlider, &dampingSlider,
                     &diffusionSlider, &modDepthSlider, &modRateSlider, &mixSlider,
                     &lowCutSlider, &highCutSlider, &outputGainSlider })
    {
        s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 18);
        s->setColour (juce::Slider::textBoxTextColourId, juce::Colours::lightgrey);
        s->setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }
}

QuantumReverbAudioProcessorEditor::~QuantumReverbAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void QuantumReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF2C0F0F));

    g.setColour (juce::Colour (0x15FFFFFF));
    for (int y = 20; y < getHeight() - 20; y += 3)
        g.drawHorizontalLine (y, 8, getWidth() - 8);

    g.setColour (juce::Colour (0xFFFFD700));
    g.drawRoundedRectangle (getLocalBounds().reduced(4).toFloat(), 18.0f, 2.5f);
    
    g.setColour (juce::Colour (0x40FFD700));
    g.drawRoundedRectangle (getLocalBounds().reduced(9).toFloat(), 22.0f, 4.0f);

    g.setColour (juce::Colour (0xFF1C1C22));
    g.fillRoundedRectangle (getLocalBounds().reduced(25).toFloat(), 14.0f);
}

void QuantumReverbAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (30);

    titleLabel.setBounds (bounds.removeFromTop (42));

    visualiser.setBounds (bounds.removeFromTop (280).reduced (40, 10));

    auto knobArea = bounds;
    auto topRow = knobArea.removeFromTop (95);
    auto midRow = knobArea.removeFromTop (95);
    auto bottomRow = knobArea.removeFromTop (95);

    int knobW = 78;
    int spacing = 18;
    int startX = topRow.getX() + 25;
    
    preDelaySlider.setBounds (startX, topRow.getY(), knobW, 85);
    decaySlider.setBounds (startX + knobW + spacing, topRow.getY(), knobW, 85);
    sizeSlider.setBounds (startX + (knobW + spacing) * 2, topRow.getY(), knobW, 85);
    dampingSlider.setBounds (startX + (knobW + spacing) * 3, topRow.getY(), knobW, 85);

    diffusionSlider.setBounds (bounds.getX() + 10, midRow.getY() + 5, knobW, 85);
    modDepthSlider.setBounds (bounds.getX() + 10, midRow.getY() + 95, knobW, 85);

    modRateSlider.setBounds (bounds.getRight() - knobW - 10, midRow.getY() + 5, knobW, 85);
    mixSlider.setBounds (bounds.getRight() - knobW - 10, midRow.getY() + 95, knobW, 85);

    lowCutSlider.setBounds (startX, bottomRow.getY(), knobW, 85);
    highCutSlider.setBounds (startX + knobW + spacing, bottomRow.getY(), knobW, 85);
    outputGainSlider.setBounds (startX + (knobW + spacing) * 2, bottomRow.getY(), knobW, 85);

    auto buttonArea = bottomRow.removeFromRight (220);
    freezeButton.setBounds (buttonArea.getX(), buttonArea.getY() + 8, 95, 32);
    shimmerButton.setBounds (buttonArea.getX() + 105, buttonArea.getY() + 8, 95, 32);
    
    exciteButton.setBounds (buttonArea.getX() + 20, buttonArea.getY() + 48, 160, 28);
    exciteButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF3A2A00));
    exciteButton.setColour (juce::TextButton::textColourOnId, juce::Colour (0xFFFFD700));

    int btnW = 58;
    vizModeButtons[0].setBounds (visualiser.getX() + 15, visualiser.getBottom() - 32, btnW, 24);
    vizModeButtons[1].setBounds (visualiser.getX() + 15 + btnW + 6, visualiser.getBottom() - 32, btnW, 24);
    vizModeButtons[2].setBounds (visualiser.getX() + 15 + (btnW + 6) * 2, visualiser.getBottom() - 32, btnW, 24);
}

void QuantumReverbAudioProcessorEditor::setupSlider (juce::Slider& slider, const juce::String& name, const juce::String& suffix)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextValueSuffix (suffix);
    slider.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (slider);
}

void QuantumReverbAudioProcessorEditor::setupButton (juce::ToggleButton& button, const juce::String& text)
{
    button.setButtonText (text);
    button.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (button);
}

void QuantumReverbAudioProcessorEditor::updateVizMode (int mode)
{
    visualiser.setMode (mode);
}