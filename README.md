# QUANTUM REVERB

**Premium Aesthetic Reverb Plugin** built with JUCE Framework by vibe-weaver.

## Features

- **Stunning Hardware-Inspired GUI**: Brushed red metallic chassis with glowing yellow accents, directly inspired by high-end studio gear.
- **Interactive Audio Visualizer** as the true centerpiece:
  - Reactive glowing waveform
  - Particle system simulating reflections (spawn on audio + click to excite)
  - Click anywhere on the visualizer to send an impulse into the reverb (perfect for sound design and instant gratification)
  - Multiple visual modes
- **High-Quality Algorithmic Reverb Engine**:
  - Pre-delay
  - Input & output diffusion
  - Modulated parallel comb filters with per-comb damping
  - Lush modulation
  - Freeze mode for infinite sustain
  - Shimmer mode for ethereal tails
  - Low/High cut filters
- **Thoughtful Layout**: Knobs arranged like premium hardware around the central display.
- **Real-time Safe**: No allocations in audio thread, smoothed parameters, proper DSP architecture.
- Formats: VST3 + AU

## Build Instructions

1. Make sure you have **JUCE 7 or 8** installed (https://juce.com)
2. Open terminal in this folder:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release -j 8
   ```
3. The plugins will be copied to your system plugin folder (or check build folder).

## Parameters (all automatable)

- **Pre Delay** — Initial delay before reverb tail
- **Decay** — Length of the reverb tail
- **Size** — Perceived space size (affects internal delays)
- **Damping** — High-frequency absorption
- **Diffusion** — Density of early reflections
- **Mod Depth / Mod Rate** — Movement and lushness in the tail
- **Mix** — Wet/Dry balance
- **Low Cut / High Cut** — Output filtering
- **Freeze** — Infinite reverb
- **Shimmer** — Ethereal high-frequency lift
- **EXCITE** button + Visualizer Click — Instantly excites the reverb

## Philosophy

This plugin was crafted following strict vibe-weaver principles:
- Beauty is non-negotiable
- Real-time safety is sacred
- The visualizer must feel alive and invite interaction
- Every control must feel premium and intentional

Enjoy creating beautiful, immersive spaces.

— vibe-weaver

---

*Pro tip: Click the visualizer while playing audio for magical moments.*