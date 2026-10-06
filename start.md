Build a complete, working VST3 audio plugin called "Momentum" using JUCE 7.0.9 and CMake. Do everything end-to-end in one go: create the project structure, write all source files in full, configure the build, and produce a plugin that compiles cleanly on Windows (MSVC, Visual Studio 2022) and loads in FL Studio and any VST3 host. Do not ask me any clarifying questions. Do not stub anything. Do not leave TODOs.

CONCEPT
Momentum is a stereo compressor that reacts to the RATE OF CHANGE of input loudness, not absolute loudness. Two RMS envelope followers run in parallel on the input: a FAST one (short window) and a SLOW one (long window). The difference between them in dB — call it Delta — represents how quickly the signal is getting louder. When Delta exceeds Sensitivity (in dB), the compressor applies gain reduction proportional to (Delta - Sensitivity) * Amount. Slow crescendos produce small Delta and pass through untouched. Sudden jumps produce large Delta and get clamped. The result: musical dynamics preserved, surprises controlled.

SIGNAL FLOW (per sample, stereo linked)
1. Read input L/R. Compute mono sum = (L + R) * 0.5 for detection.
2. Square it, feed into two one-pole smoothers:
   - fastEnv: time constant from Speed knob, range 1 ms to 50 ms
   - slowEnv: fixed at 300 ms
3. Convert both to dB: fastDb = 10 * log10(fastEnv + 1e-20), slowDb = 10 * log10(slowEnv + 1e-20).
4. Delta = fastDb - slowDb (positive when signal is rising faster than the slow follower).
5. Over = max(0, Delta - Sensitivity).
6. Target gain reduction in dB = Over * Amount (where Amount is 0..1, so at Amount=1 a 6 dB Over produces 6 dB GR; at Amount=0.5, 3 dB GR).
7. Smooth the GR with a one-pole using Recovery time constant (release-like behavior). Attack is instantaneous — that's the point of the plugin.
8. Convert GR dB to linear gain: gainLin = pow(10, -currentGR / 20).
9. Apply gainLin to both L and R (stereo linked).
10. Apply Makeup gain (dB → linear) to output.

PARAMETERS (AudioProcessorValueTreeState, all with proper ranges, defaults, and skew where noted)
- Sensitivity: 0.0 dB to 24.0 dB, default 6.0 dB. This is the Delta threshold in dB.
- Amount: 0.0 to 1.0, default 0.6, displayed as 0-100%.
- Speed: 1.0 ms to 50.0 ms, default 8.0 ms, skew 0.4. Fast envelope time constant.
- Recovery: 20.0 ms to 1000.0 ms, default 150 ms, skew 0.4. Release smoothing on gain reduction.
- Makeup: -12.0 dB to +12.0 dB, default 0.0 dB.

GUI (520 x 320 px, resizable off)
- Dark charcoal background (#151515), single accent color warm amber (#ffb020), text near-white (#ececec).
- Custom LookAndFeel_V4 subclass. Rotary knobs drawn as filled arcs (amber fill, dark track #2a2a2a) with value shown in the knob center in a clean sans-serif at 14pt bold. Knob labels below in 11pt regular, uppercase, letter-spaced.
- Layout: title "MOMENTUM" top-left in 20pt bold amber, subtitle "dynamic-derivative compressor" beside it in 10pt gray (#888).
- Row of four knobs: SENSITIVITY, AMOUNT, SPEED, RECOVERY.
- Below the knobs: a horizontal MAKEUP slider (full width, 24px tall).
- Right side of the header: a vertical gain reduction meter (10px wide, 80px tall) that fills DOWNWARD from the top in amber as GR increases. Range 0 to -18 dB. Small "GR" label below it in 10pt.
- No logos, no version strings on the GUI, no menu bar.
- Use SharedResourcePointer for the LookAndFeel so it's set once.

TECHNICAL REQUIREMENTS
- JUCE 7.0.9 fetched via CMake FetchContent from https://github.com/juce-framework/JUCE.git tag 7.0.9.
- Plugin format: VST3 only. Plugin category: Fx/Dynamics. Manufacturer: "Sheldon Davidson". Plugin code: "Mmnt". Manufacturer code: "ShDv". Bundle ID: com.sheldondavidson.momentum.
- Sample-rate aware: recompute all envelope coefficients in prepareToPlay and whenever Speed or Recovery change.
- Denormals disabled with ScopedNoDenormals in processBlock.
- Thread-safe parameter reads using std::atomic<float>* pointers from APVTS.
- Editor polls the current GR value from the processor at 30 Hz via a Timer and repaints the meter.
- State save/load via APVTS state and getStateInformation/setStateInformation.
- Zero heap allocations in processBlock.
- No use of deprecated JUCE APIs.
- Support mono and stereo. In mono, apply the same detection and gain to the single channel.

PROJECT LAYOUT
Momentum/
  CMakeLists.txt
  Source/
    PluginProcessor.h
    PluginProcessor.cpp
    PluginEditor.h
    PluginEditor.cpp
    MomentumLookAndFeel.h
    MomentumLookAndFeel.cpp
  README.md

DELIVERABLES
1. Every file above, complete and compilable. No omissions, no "…", no placeholder comments.
2. The CMakeLists.txt must be self-contained: set C++17, fetch JUCE, call juce_add_plugin with all required flags (VST3, IS_SYNTH FALSE, NEEDS_MIDI_INPUT FALSE, COPY_PLUGIN_AFTER_BUILD TRUE on Windows so it lands in the user's VST3 folder), link juce_audio_utils and juce_dsp, and define JUCE_WEB_BROWSER=0, JUCE_USE_CURL=0, JUCE_VST3_CAN_REPLACE_VST2=0.
3. README.md with three sections only: (a) exact build commands for Windows using CMake + Visual Studio 2022 generator, (b) install location the built VST3 gets copied to, (c) one-paragraph description of what the plugin does and typical use cases (taming vocal shouts, controlling drum spikes without pumping, letting orchestral swells breathe).
4. After writing all files, run the build from the command line (cmake -B build -G "Visual Studio 17 2022" -A x64, then cmake --build build --config Release) and report any errors. If the build fails, fix the errors and rebuild until it succeeds. Do not hand back a broken project.

Make a standalone as well as the vst3.

Start now. Create the folder, write every file in full, run the build, iterate until it compiles clean, and confirm the .vst3 exists at the copy destination.