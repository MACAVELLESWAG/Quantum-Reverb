#include "PluginEditor.h"

//==============================================================================
// New Premium Oscillator Hardware LookAndFeel
QuantumLookAndFeel::QuantumLookAndFeel()
{
    setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xFF4A90E2));
    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xFF2A2A2A));
}

void QuantumLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                           juce::Slider& slider)
{
    auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Outer subtle glow
    g.setColour (juce::Colour (0x3000A0FF));
    g.drawEllipse (rx - 2, ry - 2, rw + 4, rw + 4, 2.0f);

    // Main dark body
    g.setColour (juce::Colour (0xFF1F252B));
    g.fillEllipse (rx, ry, rw, rw);

    // Silver ring
    g.setColour (juce::Colour (0xFF8A8A8A));
    g.drawEllipse (rx + 3, ry + 3, rw - 6, rw - 6, 2.0f);

    // Indicator
    juce::Path p;
    const float pointerLength = radius * 0.72f;
    const float pointerThickness = 2.8f;
    p.addRectangle (-pointerThickness * 0.5f, -radius * 0.88f, pointerThickness, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));

    g.setColour (juce::Colour (0xFF00D4FF));
    g.fillPath (p);

    // Center dot
    g.setColour (juce::Colour (0xFF111111));
    g.fillEllipse (centreX - 4, centreY - 4, 8, 8);
    g.setColour (juce::Colour (0xFF00D4FF));
    g.fillEllipse (centreX - 2, centreY - 2, 4, 4);
}

void QuantumLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
    bool isOn = button.getToggleState();

    if (isOn)
    {
        g.setColour (juce::Colour (0xFF1A3A4A));
        g.fillRoundedRectangle (bounds, 5.0f);
        g.setColour (juce::Colour (0xFF00D4FF));
        g.drawRoundedRectangle (bounds.expanded(2), 6.0f, 1.5f);
    }
    else
    {
        g.setColour (juce::Colour (0xFF2A2F36));
        g.fillRoundedRectangle (bounds, 5.0f);
        g.setColour (juce::Colour (0xFF555555));
        g.drawRoundedRectangle (bounds, 5.0f, 1.0f);
    }

    g.setColour (isOn ? juce::Colour (0xFF00D4FF) : juce::Colours::lightgrey);
    g.setFont (juce::Font (11.5f, juce::Font::bold));
    g.drawText (button.getButtonText(), bounds, juce::Justification::centred, true);
}

juce::Font QuantumLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font (10.5f, juce::Font::bold);
}

//==============================================================================
// ReverbVisualiser (kept similar but slightly refined)
ReverbVisualiser::ReverbVisualiser (QuantumReverbAudioProcessor& proc)
    : processor (proc)
{
    setOpaque (false);
    startTimerHz (48);
    particles.reserve (160);
    for (int i = 0; i < 20; ++i) spawnParticles (1, 0.25f);
}

ReverbVisualiser::~ReverbVisualiser() { stopTimer(); }

void ReverbVisualiser::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto centre = bounds.getCentre();

    g.setColour (juce::Colour (0xFF0F1216));
    g.fillRoundedRectangle (bounds, 10.0f);

    g.setColour (juce::Colour (0x4000D4FF));
    g.drawRoundedRectangle (bounds.reduced(2), 10.0f, 2.0f);

    // Subtle grid
    g.setColour (juce::Colour (0x12FFFFFF));
    for (int i = 1; i < 5; ++i)
    {
        float y = bounds.getY() + bounds.getHeight() * (i / 5.0f);
        g.drawHorizontalLine ((int)y, bounds.getX() + 12, bounds.getRight() - 12);
    }

    // Animated waveform
    g.setColour (juce::Colour (0xFF00D4FF));
    juce::Path wavePath;
    const float waveY = centre.y - 18;
    const float amp = 16.0f + smoothedLevel * 38.0f;

    wavePath.startNewSubPath (bounds.getX() + 25, waveY);
    for (int i = 0; i < 110; ++i)
    {
        float x = bounds.getX() + 25 + (i / 109.0f) * (bounds.getWidth() - 50);
        float phase = (animationFrame * 0.07f) + (i * 0.11f);
        float y = waveY + std::sin(phase) * amp * (0.55f + 0.45f * std::sin(i * 0.025f));
        wavePath.lineTo (x, y);
    }

    g.setColour (juce::Colour (0x6000D4FF));
    g.strokePath (wavePath, juce::PathStrokeType(3.5f));
    g.setColour (juce::Colour (0xFF00D4FF));
    g.strokePath (wavePath, juce::PathStrokeType(1.8f));

    // Particles
    for (const auto& p : particles)
    {
        if (p.life <= 0.02f) continue;
        float alpha = juce::jlimit(0.0f, 1.0f, p.life);
        g.setColour (p.col.withAlpha(alpha * 0.85f));
        g.fillEllipse (p.x - p.size*0.5f, p.y - p.size*0.5f, p.size, p.size);
    }

    g.setColour (juce::Colour (0xFF00D4FF));
    g.setFont (juce::Font (16.5f, juce::Font::bold));
    g.drawText ("QUANTUM REVERB", bounds.removeFromTop(34).reduced(15,0), juce::Justification::centred, false);

    g.setFont (juce::Font (9.5f));
    g.setColour (juce::Colours::lightgrey.withAlpha(0.55f));
    g.drawText ("Click to excite  •  Premium Algorithmic Space", bounds.removeFromBottom(18), juce::Justification::centred);
}

void ReverbVisualiser::resized() {}
void ReverbVisualiser::mouseDown (const juce::MouseEvent&)
{
    processor.triggerImpulse();
    spawnParticles (32, 0.9f);
}
void ReverbVisualiser::mouseDrag (const juce::MouseEvent&) {}

void ReverbVisualiser::timerCallback()
{
    updateFromProcessor();
    updateParticles();
    animationFrame++;
    if (rng.nextFloat() < 0.32f) spawnParticles (1 + (int)(smoothedLevel*2.5f), 0.2f);
    repaint();
}

void ReverbVisualiser::updateFromProcessor()
{
    currentOutputLevel = processor.getOutputLevel();
    smoothedLevel = smoothedLevel * 0.82f + currentOutputLevel * 1.6f * 0.18f;
    smoothedLevel = juce::jlimit(0.0f, 1.0f, smoothedLevel);
}

void ReverbVisualiser::spawnParticles (int count, float intensity)
{
    auto b = getLocalBounds().toFloat();
    for (int i = 0; i < count; ++i)
    {
        Particle p;
        p.x = b.getCentreX() + rng.nextFloat()*36 - 18;
        p.y = b.getCentreY() + rng.nextFloat()*26 - 13;
        float vel = 0.7f + intensity * 2.0f;
        p.vx = (rng.nextFloat()-0.5f) * vel;
        p.vy = (rng.nextFloat()-0.5f) * vel * 0.65f - 0.25f;
        p.life = 0.6f + rng.nextFloat()*0.55f;
        p.size = 1.1f + intensity * 1.6f;
        p.col = (rng.nextFloat() > 0.5f) ? juce::Colour(0xFF00D4FF) : juce::Colour(0xFF4A90E2);
        particles.push_back(p);
    }
    while (particles.size() > 150) particles.erase(particles.begin());
}

void ReverbVisualiser::updateParticles()
{
    for (auto& p : particles)
    {
        if (p.life <= 0) continue;
        p.x += p.vx; p.y += p.vy;
        p.vy += 0.012f;
        p.vx *= 0.982f; p.vy *= 0.982f;
        p.life -= 0.016f;
        if (p.life < 0.35f) p.size *= 0.96f;
    }
    particles.erase (std::remove_if(particles.begin(), particles.end(), [](const Particle& p){ return p.life <= 0.015f; }), particles.end());
}

void ReverbVisualiser::setMode (int newMode) { vizMode = newMode; repaint(); }

//==============================================================================
// Main Editor - Premium Oscillator Layout
QuantumReverbAudioProcessorEditor::QuantumReverbAudioProcessorEditor (QuantumReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), visualiser (p)
{
    setLookAndFeel (&customLookAndFeel);
    setSize (980, 620);
    setResizable (false, false);

    titleLabel.setText ("QUANTUM REVERB", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (24.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF00D4FF));
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    addAndMakeVisible (visualiser);

    // Knobs - improved spacing
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

    for (auto* s : { &preDelaySlider, &decaySlider, &sizeSlider, &dampingSlider, &diffusionSlider,
                     &modDepthSlider, &modRateSlider, &mixSlider, &lowCutSlider, &highCutSlider, &outputGainSlider })
    {
        s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 48, 17);
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
    g.fillAll (juce::Colour (0xFF1C2128));

    // Subtle top gradient
    juce::ColourGradient grad (juce::Colour (0xFF252C35), 0, 0, juce::Colour (0xFF1C2128), 0, 80, false);
    g.setGradientFill (grad);
    g.fillRect (0, 0, getWidth(), 80);

    g.setColour (juce::Colour (0xFF00D4FF));
    g.drawRoundedRectangle (getLocalBounds().reduced(6).toFloat(), 14.0f, 2.0f);
}

void QuantumReverbAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (28);

    titleLabel.setBounds (bounds.removeFromTop (38));

    visualiser.setBounds (bounds.removeFromTop (265).reduced (35, 8));

    auto knobArea = bounds;
    auto topRow = knobArea.removeFromTop (92);
    auto midRow = knobArea.removeFromTop (92);
    auto bottomRow = knobArea.removeFromTop (92);

    int knobW = 74;
    int spacing = 16;
    int startX = topRow.getX() + 18;

    preDelaySlider.setBounds (startX, topRow.getY(), knobW, 82);
    decaySlider.setBounds (startX + knobW + spacing, topRow.getY(), knobW, 82);
    sizeSlider.setBounds (startX + (knobW + spacing)*2, topRow.getY(), knobW, 82);
    dampingSlider.setBounds (startX + (knobW + spacing)*3, topRow.getY(), knobW, 82);

    diffusionSlider.setBounds (bounds.getX() + 8, midRow.getY() + 4, knobW, 82);
    modDepthSlider.setBounds (bounds.getX() + 8, midRow.getY() + 92, knobW, 82);

    modRateSlider.setBounds (bounds.getRight() - knobW - 8, midRow.getY() + 4, knobW, 82);
    mixSlider.setBounds (bounds.getRight() - knobW - 8, midRow.getY() + 92, knobW, 82);

    lowCutSlider.setBounds (startX, bottomRow.getY(), knobW, 82);
    highCutSlider.setBounds (startX + knobW + spacing, bottomRow.getY(), knobW, 82);
    outputGainSlider.setBounds (startX + (knobW + spacing)*2, bottomRow.getY(), knobW, 82);

    auto buttonArea = bottomRow.removeFromRight (210);
    freezeButton.setBounds (buttonArea.getX(), buttonArea.getY() + 6, 92, 30);
    shimmerButton.setBounds (buttonArea.getX() + 100, buttonArea.getY() + 6, 92, 30);

    exciteButton.setBounds (buttonArea.getX() + 18, buttonArea.getY() + 44, 168, 26);
    exciteButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF1F2A35));
    exciteButton.setColour (juce::TextButton::textColourOnId, juce::Colour (0xFF00D4FF));

    int btnW = 56;
    vizModeButtons[0].setBounds (visualiser.getX() + 12, visualiser.getBottom() - 30, btnW, 22);
    vizModeButtons[1].setBounds (visualiser.getX() + 12 + btnW + 5, visualiser.getBottom() - 30, btnW, 22);
    vizModeButtons[2].setBounds (visualiser.getX() + 12 + (btnW + 5)*2, visualiser.getBottom() - 30, btnW, 22);
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