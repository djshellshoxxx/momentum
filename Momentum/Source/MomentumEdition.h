/*
    MomentumEdition.h

    Compile-time edition gate. include.md notes that this build "may become the pro version,
    and a free version might be made in future builds" - this header is that hook.

    Select the edition at configure time:

        cmake -B build -G "Visual Studio 17 2022" -A x64 -DMOMENTUM_EDITION=Lite

    Pro is the default, so an ordinary build is byte-for-byte the plugin that shipped before
    this header existed.

    Everything here is constexpr rather than #ifdef on purpose. Both editions are then parsed
    and type-checked by every build, so a change that breaks Lite cannot slip through a Pro
    build unnoticed. Call sites use `if constexpr (...)` to keep the dead branch out of the
    binary without tripping MSVC's C4127 constant-conditional warning.

    The DSP is deliberately NOT gated. Lite is the whole compressor with a smaller workflow
    around it - that is what makes it worth installing, and what makes Pro worth buying.
*/

#pragma once

#include <juce_core/juce_core.h>

#include <algorithm>
#include <iterator>

#ifndef MOMENTUM_EDITION_PRO
 #define MOMENTUM_EDITION_PRO 1
#endif

namespace MomentumEdition
{
    /** True for the paid build. */
    inline constexpr bool isPro = (MOMENTUM_EDITION_PRO != 0);

    /** Shown in the footer, the help page and the standalone window title. */
    inline constexpr const char* name = isPro ? "Pro" : "Lite";

    //== Feature gates ==========================================================
    /** Saving, Save As and opening .mmtp files. Lite loads factory presets only. */
    inline constexpr bool allowUserPresets = isPro;

    /** Right-click MIDI Learn and CC mapping. */
    inline constexpr bool allowMidiLearn = isPro;

    /** A/B compare slots in the header. */
    inline constexpr bool allowABCompare = isPro;

    /** Which factory presets Lite exposes, as indices into the 20-entry bank.

        A curated spread, not a prefix: taking the first six would have handed Lite users
        Init plus three Vocal and two Drums presets, which says very little about what the
        compressor is for. These six each demonstrate a different job - a starting point, a
        vocal, a drum transient, a bass pick attack, a gentle mix-bus safety net, and speech.

        Indices are into PresetManager::getFactoryPresets(). If the bank is reordered, fix
        this list: getFactoryPresets() asserts in debug builds that each index still lands on
        the name in liteFactoryPresetNames below. */
    inline constexpr int liteFactoryPresets[] = { 0, 1, 4, 7, 12, 14 };

    /** The names those indices are expected to refer to, for the startup sanity check. */
    inline constexpr const char* liteFactoryPresetNames[] =
    {
        "Init - Neutral",
        "Vocal - Shout Tamer",
        "Drums - Spike Control",
        "Bass - Pick Attack Tamer",
        "Mix Bus - Transparent Safety",
        "Podcast - Plosive Clamp"
    };

    /** How many factory presets this edition exposes, given the full bank size. */
    inline int visibleFactoryPresetCount (int bankSize) noexcept
    {
        if constexpr (isPro)
            return bankSize;
        else
            return juce::jmin (bankSize, (int) std::size (liteFactoryPresets));
    }

    /** Whether a given bank entry is visible in this edition. */
    inline bool isFactoryPresetVisible (int bankIndex) noexcept
    {
        if constexpr (isPro)
            return bankIndex >= 0;
        else
            return std::find (std::begin (liteFactoryPresets),
                              std::end   (liteFactoryPresets),
                              bankIndex) != std::end (liteFactoryPresets);
    }

    /** One line for the help page explaining what this edition does and does not include. */
    inline juce::String describeEdition()
    {
        if (isPro)
            return "This is the Pro edition. Every feature described on this page is available.";

        return "This is the Lite edition. The compressor itself is complete and unrestricted - "
               "the full detector, all five controls and the whole dynamic range are here. "
               "Pro adds saving and loading your own presets, MIDI Learn, A/B compare, and the "
               "complete factory bank.";
    }
}
