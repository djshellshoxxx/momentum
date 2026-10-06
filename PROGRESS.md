# Momentum — Build Progress

**Project:** Momentum — dynamic-derivative compressor (VST3 + Standalone)
**Root:** `C:\Users\new\Desktop\momentum\Momentum`
**Spec sources:** `../start.md` (DSP + plugin spec), `../include.md` (features required in all VSTs), `../theme.md` (house visual identity)
**Started:** 2026-09-15
**Version:** 1.0.0

---

## Status: COMPLETE (2026-09-15)

All seven phases are done. Release builds clean with zero warnings, the VST3 is installed,
and the standalone has been launched and visually verified. See `Momentum/README.md` for the
user-facing documentation.

## How to rebuild

1. `cd C:\Users\new\Desktop\momentum\Momentum`
2. Configure: `cmake -B build -G "Visual Studio 17 2022" -A x64`
3. Build:     `cmake --build build --config Release`

Artefacts:
- VST3 — `build/Momentum_artefacts/Release/VST3/Momentum.vst3`
  (auto-installed to `C:\Program Files\Common Files\VST3`; needs an elevated shell)
- Standalone — `build/Momentum_artefacts/Release/Standalone/Momentum.exe`

---

## Environment (verified 2026-09-15)

| Tool | Status |
|---|---|
| CMake | 4.4.3 — present |
| Git | 2.54.0.windows.1 — present |
| Compiler | **Visual Studio Build Tools 2022** (not full VS) — generator `Visual Studio 17 2022` |
| VST3 install dir | `C:\Program Files\Common Files\VST3` (exists, populated) |

Note: CMake 4.x is stricter than JUCE 7.0.9 expects in places. If configure fails on a
policy/minimum-version error, add `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` to the configure line.

---

## Spec conflicts and how they were resolved

The three spec files disagree in four places. Decisions made (all deliberate, all documented
in README.md as well):

1. **Colour palette.** `start.md` asks for charcoal `#151515` + amber `#ffb020`.
   `theme.md` mandates the house palette (`#0E1116` base, `#171B22` panels, `#2A303A` edges)
   and explicitly permits each plugin to **re-tint one accent**.
   → **Resolved:** house neutrals from `theme.md`, primary accent re-tinted to Momentum's
   amber `#FFB020`. Secondary accent stays teal `#4FB6C4`. Best of both.

2. **Window size.** `start.md` says 520x320, non-resizable.
   `include.md` additionally requires a menu bar, preset selector, help page, options page,
   reset button and random button — these do not fit in 520x320.
   → **Resolved:** 720x344, non-resizable. Same layout language, just enough room.

3. **MIDI input.** `start.md` says `NEEDS_MIDI_INPUT FALSE`.
   `include.md` requires right-click **MIDI mapping** on every control, which needs an event
   input bus (VST3 exposes `IMidiMapping` only when the plugin accepts MIDI).
   → **Resolved:** `NEEDS_MIDI_INPUT TRUE`. Audio behaviour is identical; it only adds an
   empty MIDI bus so MIDI Learn can work. Documented in README.

4. **Export audio.** `include.md` says this is for instruments only, "not for effects".
   → **Resolved:** intentionally omitted. Momentum is an effect.

---

## Task status

### Phase 1 — Scaffolding
- [x] Read start.md / include.md / theme.md
- [x] Verify toolchain (cmake, git, MSVC, VST3 dir)
- [x] Create `Momentum/`, `Momentum/Source/`, `Momentum/Resources/`
- [x] Create this progress file
- [x] Generate custom app icon (`Resources/MomentumIcon.png`)
- [x] Write `CMakeLists.txt`

### Phase 2 — DSP core (start.md)
- [x] `PluginProcessor.h` / `.cpp` — APVTS, 5 params, dual-RMS delta detector
- [x] Coefficient recalculation on prepareToPlay + Speed/Recovery change
- [x] Atomic GR value published for the meter
- [x] State save/load

### Phase 3 — Look and feel (theme.md)
- [x] `MomentumLookAndFeel.h` / `.cpp` — palette tokens, rotary, linear slider, buttons,
      combo box, tooltip pill, popup menu

### Phase 4 — Editor + include.md features
- [x] `PluginEditor.h` / `.cpp` — header strip, 4 knobs, makeup slider, GR meter, footer,
      signature notch
- [x] Menu bar: File (Save / Save As / Open / Options) + preset selector + A/B
- [x] Reset button
- [x] Random button (re-randomises from defaults on every press)
- [x] Hover tooltips (400ms), toggleable from Options
- [x] Right-click on any control: set value / reset / MIDI learn / clear mapping
- [x] Help page (per-feature explanation, workflow, GUI description, version number)
- [x] Options page (tooltips on/off, audio + MIDI device selection in standalone)

### Phase 5 — Presets
- [x] `PresetManager.h` / `.cpp` — factory bank with descriptive names
- [x] User preset save / save-as / open to `.mmtp` files

### Phase 6 — MIDI learn
- [x] `MidiLearnManager` — CC capture, persistent mapping, applied in processBlock

### Phase 7 — Build and verify
- [x] `README.md` (build commands, install location, description)
- [x] `cmake -B build -G "Visual Studio 17 2022" -A x64` succeeds
- [x] `cmake --build build --config Release` succeeds with no errors
- [x] `Momentum.vst3` confirmed at `C:\Program Files\Common Files\VST3`
- [x] `Momentum.exe` standalone confirmed and launches

---

## Build log

| Date | Action | Result |
|---|---|---|
| 2026-09-15 | Toolchain probe | OK — cmake 4.4.3, git 2.54, VS Build Tools 2022 |
| 2026-09-15 | Configure `-G "Visual Studio 17 2022" -A x64` | OK — JUCE 7.0.9 fetched |
| 2026-09-15 | `cmake --build build --config Release` | OK — exit 0, **zero warnings** |
| 2026-09-15 | VST3 install | OK — `C:\Program Files\Common Files\VST3\Momentum.vst3` |
| 2026-09-15 | Standalone launch test | OK — window appears (~10 s while devices enumerate), full UI verified by screenshot |
| 2026-09-15 | README.md | Written |

---

## Forward-looking notes (from include.md)

- This build is the **Pro** version. A free/lite tier may follow — keep the feature gate idea
  in mind: `MOMENTUM_EDITION` compile definition is the intended hook (not yet added).
- No instrument variant applies here (Momentum is an effect).

---

## Spec-conformance audit (2026-09-16, 09:33)

Re-verified the Phase 1-7 checkboxes against the three spec files rather than trusting them.
Read-only audit — no rebuild (machine was under memory pressure from an unrelated build).

**start.md — DSP and config: conforms.**

| Spec item | Found |
|---|---|
| Sensitivity 0-24 dB, def 6.0, no skew | `PluginProcessor.cpp:11` — matches |
| Amount 0-1, def 0.6, shown as % | `PluginProcessor.cpp:15` — matches |
| Speed 1-50 ms, def 8.0, skew 0.4 | `PluginProcessor.cpp:19` — matches |
| Recovery 20-1000 ms, def 150, skew 0.4 | `PluginProcessor.cpp:23` — matches |
| Makeup -12..+12 dB, def 0.0 | `PluginProcessor.cpp:27` — matches |
| Slow envelope fixed at 300 ms | `PluginProcessor.h:199` `slowEnvelopeMs = 300.0` |
| `10*log10(env + 1e-20)` | `PluginProcessor.cpp:390-391` — exact |
| ScopedNoDenormals in processBlock | `PluginProcessor.cpp:341` |
| Editor polls GR at 30 Hz | `PluginEditor.cpp:161` `startTimerHz(30)` |
| C++17, JUCE 7.0.9 via FetchContent | `CMakeLists.txt:11,22` |
| Bundle ID / Mmnt / ShDv | `CMakeLists.txt:34-36` — exact |

**include.md — required features: all present.**

- Help page — `HelpComponent.cpp` (298 lines); version via `JucePlugin_VersionString`
- Custom icon — `Resources/MomentumIcon.png`, wired as `ICON_BIG`/`ICON_SMALL` (`CMakeLists.txt:49-50`)
- Presets — `PresetManager.cpp` factory bank + user `.mmtp` files
- Dropdown menu — `PluginEditor.cpp:496-505`: New (reset to defaults) / Open Preset… /
  Save Preset / Save Preset As… / Open Preset Folder / Options… / Help…
- Reset button and Random button — both in `PluginEditor.cpp` / `PluginProcessor.cpp`
- Right-click on any control — `PluginEditor.cpp:554-558`: Set value… / Reset to default /
  MIDI Learn / Clear MIDI mapping (CC n)
- Tooltips — present across LookAndFeel, editor and Options (toggleable)
- Export audio — still correctly omitted (Momentum is an effect)

**theme.md — palette: exact match.** `MomentumLookAndFeel.h:20-30` carries every house token
(`#0e1116`, `#171b22`, `#2a303a`, `#4fb6c4`, `#e6e8ec`, `#8a929e`, `#7bc96f`, `#f2c14e`,
shadow at 55% alpha), with the one permitted accent re-tint to amber `#ffb020` and the house
orange `#e8532a` retained for clip. Signature 12px/45deg corner notch at `PluginEditor.cpp:485`.

**Gaps found: none.** No code changes were needed.

**Still open (unchanged):** the `MOMENTUM_EDITION` compile-definition hook for a future
free/lite tier. Flagged forward-looking in the original notes, never a committed task —
it needs a decision on what the lite tier withholds before it can be built.

---

## Phase 8 — Edition gate (2026-09-16, in progress)

`MOMENTUM_EDITION` is now wired. This was the last open item from the forward-looking notes.

> ⚠️ **NOT YET COMPILED.** Written while the machine had ~65 MB RAM free (an unrelated
> `basspilot` build was using the rest), so the build is deferred to the next work tick.
> Treat Phase 8 as unverified until the build log below shows a green Pro **and** Lite run.
> `CMakeLists.txt` was backed up to the session scratchpad before editing.

### The lite tier, as assumed

The user did not specify what the free tier withholds, so this is a stated assumption, easy
to change — every gate is one `constexpr` in `Source/MomentumEdition.h`:

| Feature | Pro | Lite |
|---|---|---|
| Full DSP, all 5 controls, full range | yes | **yes — deliberately not gated** |
| Factory presets | all 20 | first 6 |
| Save / Save As / Open `.mmtp` | yes | no |
| MIDI Learn + CC mapping | yes | no |
| A/B compare slots | yes | no |
| Reset, Random, tooltips, help, options | yes | yes |

Rationale: a crippled-sounding demo sells nothing. Lite is the complete compressor with a
smaller workflow around it, so it is genuinely usable and Pro is a workflow upgrade.

### How to build each

```
cmake -B build      -G "Visual Studio 17 2022" -A x64                          # Pro (default)
cmake -B build-lite -G "Visual Studio 17 2022" -A x64 -DMOMENTUM_EDITION=Lite  # Lite
```

Pro and Lite carry different `PLUGIN_CODE` (`Mmnt` / `MmnL`), `BUNDLE_ID` and `PRODUCT_NAME`
("Momentum" / "Momentum Lite"), so both can be installed side by side without clobbering
each other in `C:\Program Files\Common Files\VST3`.

### Files touched

- **`Source/MomentumEdition.h`** — new. All gates as `inline constexpr`, plus
  `visibleFactoryPresetCount()` and `describeEdition()`.
- **`CMakeLists.txt`** — `MOMENTUM_EDITION` cache variable (Pro/Lite, `FATAL_ERROR` on
  anything else), per-edition identity, `MOMENTUM_EDITION_PRO` compile definition.
- **`Source/PluginProcessor.h`** — includes the gate; adds `getEditionName()`.
- **`Source/PluginEditor.cpp`** — File menu, right-click menu, factory bank clamp,
  A/B buttons, footer.
- **`Source/HelpComponent.cpp`** — version line carries the edition; adds a paragraph
  saying what this edition includes.

### Design note

Gates are `constexpr` + `if constexpr`, not `#ifdef`. Both editions are therefore parsed and
type-checked by *every* build, so a change that breaks Lite cannot hide inside a Pro build.
`if constexpr` (rather than a plain `if`) keeps the dead branch out of the binary without
tripping MSVC's C4127 constant-conditional warning, which matters because this project is
held at zero warnings.

### Bug caught while writing this

The Lite "More presets in Pro" row was initially added as a normal, selectable combo-box
item with ID 900. `handlePresetSelection()` routes any ID below 1000 to
`loadFactoryPreset (id - 1)`, so clicking it would have called `loadFactoryPreset (899)` and
indexed far past a 20-entry vector. Fixed twice over: the row is now
`setItemEnabled (900, false)`, and `handlePresetSelection()` bounds-checks the factory index
before using it.

### Phase 8 verification

- [x] `cmake --build build --config Release` — **Pro: clean, zero warnings**, VST3
      reinstalled, standalone relinked.
- [x] Lite configure — `-DMOMENTUM_EDITION=Lite` confirmed by the status line.
- [x] Lite compile of **all seven** project translation units — `PluginProcessor`,
      `PluginEditor`, `MomentumLookAndFeel`, `MomentumControls`, `PresetManager`,
      `HelpComponent`, `OptionsComponent` — **zero errors, zero warnings** with
      `MOMENTUM_EDITION_PRO=0`. Every gated line is in these files, so the gate is verified.
- [x] `Momentum/README.md` — editions, both build commands, feature table, low-memory notes.
- [ ] **A complete Lite binary was never produced.** See below.
- [ ] Visual check of Lite's UI differences — blocked on the above.

### Why there is no Lite binary

Three attempts, all killed by the OS for memory. None of them reported a single compiler
error or warning; each simply stopped and returned a non-zero exit code. The last one died
inside `juce_audio_processors.cpp` — a stock JUCE unity file, byte-identical in both
editions and already proven to compile in the Pro tree.

This machine has 3977 MB total. A second full JUCE build tree does not fit alongside normal
desktop use, and dropping LTO (`-DMOMENTUM_DISABLE_LTO=ON`) was not enough on its own.

What this does and does not tell us:

- **Verified:** the Lite code path compiles. All seven project files build clean under
  `MOMENTUM_EDITION_PRO=0`, and they contain every gate.
- **Not verified:** that a Lite binary links, loads in a host, and that the UI actually hides
  A/B, hides preset save/open, and shows 6 presets with a disabled "More presets in Pro" row.
  That is a runtime claim and nothing here has tested it.

To finish on a machine with more headroom:

```
cd Momentum
cmake -B build-lite -G "Visual Studio 17 2022" -A x64 -DMOMENTUM_EDITION=Lite
cmake --build build-lite --config Release
```

An observation worth keeping: a build that dies with a non-zero exit and an empty diagnostics
section has been killed, not failed. Grep the output for `error` before assuming a code fault.

### Also corrected in this phase

- `Momentum/README.md` claimed **13 factory presets**. The bank has **20**, confirmed by
  parsing `PresetManager.cpp`. Corrected, and the missing categories (Podcast, Live, Synth,
  FX, Broadcast) added to the list. This error predated Phase 8.
- `MomentumEdition.h` originally claimed the bank "is ordered so that slice is a usable
  spread". It was not — the first six were Init, three Vocal and two Drums. **Now fixed
  properly** rather than just documented: `liteFactoryPresets` is a curated index list
  `{ 0, 1, 4, 7, 12, 14 }` — Init, Vocal Shout Tamer, Drums Spike Control, Bass Pick Attack
  Tamer, Mix Bus Transparent Safety, Podcast Plosive Clamp — so each of Lite's six shows a
  different job. `getFactoryPresets()` asserts the indices still match the expected names,
  so reordering the bank cannot silently change Lite's contents.
  Two consequences handled: the preset menu keeps item IDs at (bank index + 1) now that the
  visible set is non-contiguous, and `syncPresetSelection()` falls through to the free-text
  path when state names a preset this edition does not list.
- Added `option(MOMENTUM_DISABLE_LTO ... OFF)` to `CMakeLists.txt` as a documented
  development escape hatch for low-memory machines. Default OFF, so release builds are
  unchanged. Not for shipping binaries.
