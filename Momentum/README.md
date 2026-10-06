# Momentum

**A dynamic-derivative compressor.** VST3 + Standalone, Windows x64.
Version 1.0.0 — © Sheldon Davidson

---

## What it does

Ordinary compressors react to **how loud** a signal is. Momentum reacts to **how fast the
loudness is changing**.

Two RMS envelope followers run in parallel on the mono sum of the input:

- a **fast** follower, whose window is set by **Speed**
- a **slow** follower, fixed at 300 ms

The difference between them in dB — call it *Delta* — is a measure of how quickly the signal
is getting louder. Whatever part of Delta exceeds **Sensitivity** is multiplied by **Amount**
and applied as gain reduction. Attack is instantaneous by design; only the release is
smoothed, by **Recovery**.

The musical consequence: a slow crescendo produces a small Delta and passes through
untouched, no matter how loud it eventually gets. A sudden jump — a shouted syllable, a
stray hard snare hit, a pick attack — produces a large Delta and gets clamped. You can hold
down surprises without flattening the performance.

---

## Parameters

| Control | Range | Default | What it does |
|---|---|---|---|
| **Sensitivity** | 0 – 24 dB | 6.0 dB | How large a sudden jump has to be before anything happens. Higher = more gets through. |
| **Amount** | 0 – 100 % | 60 % | How much of the excess Delta is turned into gain reduction. |
| **Speed** | 1 – 50 ms | 8.0 ms | The fast follower's window. Short catches consonants and transients; long catches phrases. |
| **Recovery** | 20 – 1000 ms | 150 ms | How quickly gain reduction lets go once the jump has passed. |
| **Makeup** | −12 – +12 dB | 0.0 dB | Output trim, applied after the detector. |

The gain-reduction meter to the right of the knob row reads 0 to −18 dB, fills downward, and
holds a peak line.

---

## Building

Requires CMake 3.22+, Git, and MSVC (Visual Studio 2022 or the 2022 Build Tools).
JUCE 7.0.9 is fetched automatically by CMake — no manual setup.

```sh
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

If CMake 4.x rejects the configure step on a policy/minimum-version error, the top of
`CMakeLists.txt` already sets `CMAKE_POLICY_VERSION_MINIMUM` to 3.5 for you. Should you need
it explicitly, add `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` to the configure line.

### Editions

The build above is the **Pro** edition, which is the default. A **Lite** edition is selected
at configure time:

```sh
cmake -B build-lite -G "Visual Studio 17 2022" -A x64 -DMOMENTUM_EDITION=Lite
cmake --build build-lite --config Release
```

The two carry different plugin codes, bundle IDs and product names, so both can be installed
at the same time without clobbering each other.

| | Pro | Lite |
|---|---|---|
| Compressor, all five controls, full range | yes | **yes** |
| Factory presets | 20 | 6 (curated, see below) |
| Save / Save As / Open `.mmtp` | yes | — |
| MIDI Learn and CC mapping | yes | — |
| A/B compare | yes | — |
| Reset, Random, tooltips, help, options | yes | yes |
| Plugin code / bundle ID | `Mmnt` / `…momentum` | `MmnL` / `…momentumlite` |

The DSP is deliberately not gated: Lite is the whole compressor with a smaller workflow
around it. Every gate is one `constexpr` in `Source/MomentumEdition.h`.

Lite's six factory presets are a deliberate spread rather than the first six in the bank,
so each one demonstrates a different job:

| Preset | Shows off |
|---|---|
| Init — Neutral | The starting point |
| Vocal — Shout Tamer | Sudden shouted syllables |
| Drums — Spike Control | Stray hard hits, without pumping |
| Bass — Pick Attack Tamer | Pick and slap attacks |
| Mix Bus — Transparent Safety | A quiet safety net |
| Podcast — Plosive Clamp | Speech, pops and desk knocks |

`getFactoryPresets()` asserts in debug builds that those indices still land on those names,
so reordering the bank cannot silently change what Lite ships with.

### Building on a small machine

Link-time optimisation is the peak-memory moment of a JUCE build, and on a 4 GB machine — or
one sharing the box with another compile — it is the step the OS is most likely to kill. If a
build dies with no error message and a non-zero exit code, that is what happened. Drop LTO:

```sh
cmake -B build-lite -G "Visual Studio 17 2022" -A x64 -DMOMENTUM_EDITION=Lite -DMOMENTUM_DISABLE_LTO=ON
```

It is off by default, so ordinary builds are unaffected. It also helps to keep MSBuild
single-threaded, which the two shells disagree about how to spell — in PowerShell the
stop-parsing token is needed:

```powershell
cmake --build build-lite --config Release -- --% -m:1 -p:CL_MPCount=1
```

**Do not ship a binary built with `MOMENTUM_DISABLE_LTO=ON`.** It is a development escape
hatch, not a release configuration.

### Where the build lands

| Artefact | Path |
|---|---|
| VST3 | `build/Momentum_artefacts/Release/VST3/Momentum.vst3` |
| Standalone | `build/Momentum_artefacts/Release/Standalone/Momentum.exe` |

`COPY_PLUGIN_AFTER_BUILD` is on, so the VST3 is also installed automatically to:

```
C:\Program Files\Common Files\VST3\Momentum.vst3
```

Writing there needs an elevated shell. If the copy step fails, the build itself still
succeeds and you can copy the bundle by hand.

> **Note on the standalone:** it takes a few seconds to appear on first launch while JUCE
> enumerates audio and MIDI devices. It also opens with input muted — JUCE does this to
> avoid a feedback loop — so click **Settings...** in the yellow bar to choose a device and
> unmute.

---

## Interface

720 × 344, non-resizable.

- **Header** — the Momentum wordmark, a **FILE** menu, the preset selector, **A**/**B**
  compare slots with a copy-across button, and the options gear.
- **DETECTOR** — Sensitivity, Amount, Speed and Recovery, with the gain-reduction meter.
- **OUTPUT** — the Makeup trim.
- **Footer** — Reset, Random, Help, and the version number.

### Features

- **Presets.** 20 factory presets with descriptive names, organised by source
  (Vocal, Drums, Bass, Guitar, Piano, Orchestra, Strings, Mix Bus, Mastering,
  Podcast, Live, Synth, FX, Broadcast).
  User presets save to plain-XML `.mmtp` files you can copy between machines.
- **A/B compare.** Two slots, with a button to copy the active one into the other.
- **Reset** puts every control back to its default. **Random** re-randomises from the
  defaults on every press, within musically useful bounds.
- **Tooltips** appear after 400 ms of hover, and can be switched off in Options.
- **Right-click any control** for value entry, reset to default, MIDI Learn, and clear
  mapping. Learned CC numbers are shown on the interface and persist with the session.
- **Help page** explains every control, the signal flow, and the interface.
- **Options page** toggles tooltips and, in the standalone, selects audio and MIDI devices.

---

## Deliberate deviations from the spec

The three spec documents (`../start.md`, `../include.md`, `../theme.md`) disagree in four
places. Each was resolved deliberately:

1. **Colour palette.** `start.md` asks for charcoal `#151515` with amber `#ffb020`.
   `theme.md` mandates the house palette (`#0E1116` base, `#171B22` panels, `#2A303A` edges)
   and explicitly permits each plugin to re-tint one accent.
   → House neutrals from `theme.md`, primary accent re-tinted to Momentum's amber `#FFB020`.
   Secondary accent stays teal `#4FB6C4`.

2. **Window size.** `start.md` specifies 520 × 320. `include.md` additionally requires a menu
   bar, preset selector, help page, options page, reset button and random button, which do
   not fit in that footprint.
   → 720 × 344, non-resizable. The same layout language, with enough room for the required
   controls.

3. **MIDI input.** `start.md` says `NEEDS_MIDI_INPUT FALSE`. `include.md` requires right-click
   MIDI mapping on every control, and VST3 only exposes `IMidiMapping` when the plugin
   accepts MIDI.
   → `NEEDS_MIDI_INPUT TRUE`. This adds an empty MIDI bus and nothing else; the audio path is
   byte-for-byte unchanged.

4. **Export audio.** `include.md` scopes this to instruments, "not for effects".
   → Intentionally omitted. Momentum is an effect.

---

## Layout of the source

| File | Contents |
|---|---|
| `Source/MomentumEdition.h` | Compile-time Pro/Lite feature gates |
| `Source/PluginProcessor.*` | APVTS, the parameter table, the dual-RMS delta detector, MIDI learn, state |
| `Source/PluginEditor.*` | The interface, menus, context menus, A/B, meter ballistics |
| `Source/MomentumLookAndFeel.*` | Palette tokens and every custom-drawn control |
| `Source/MomentumControls.*` | The slider and meter subclasses the editor is built from |
| `Source/PresetManager.*` | Factory bank and `.mmtp` user preset I/O |
| `Source/HelpComponent.*` | The help page |
| `Source/OptionsComponent.*` | The options page |
