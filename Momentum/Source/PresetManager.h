/*
    PresetManager.h

    Factory preset bank plus user preset save / save-as / open, as required by include.md.

    User presets are plain XML files with a .mmtp extension so they can be copied between
    machines, mailed, or checked into a project folder.
*/

#pragma once

#include "PluginProcessor.h"

#include <array>
#include <vector>

//==============================================================================
struct MomentumPreset
{
    juce::String name;
    juce::String description;
    std::array<float, MomentumParams::numParameters> values {};
};

//==============================================================================
class PresetManager
{
public:
    explicit PresetManager (MomentumAudioProcessor&);

    //== Factory bank ===========================================================
    static const std::vector<MomentumPreset>& getFactoryPresets();

    //== User presets ===========================================================
    /** %APPDATA%/Sheldon Davidson/Momentum/Presets - created on demand. */
    static juce::File getUserPresetDirectory();
    static juce::String getFileExtension()          { return ".mmtp"; }
    static juce::String getFileWildcard()           { return "*.mmtp"; }

    /** Rescans the user preset folder. Cheap, but not free - call it when the menu opens. */
    void refreshUserPresets();
    const juce::Array<juce::File>& getUserPresetFiles() const noexcept { return userPresetFiles; }

    //== Loading and saving =====================================================
    void loadFactoryPreset (int index);
    bool loadFromFile (const juce::File&);
    bool saveToFile (const juce::File&, const juce::String& presetName);

    /** The file the current preset came from, if any. Empty for factory presets. */
    juce::File getCurrentPresetFile() const noexcept { return currentPresetFile; }
    void clearCurrentPresetFile()                    { currentPresetFile = juce::File(); }

private:
    static const juce::Identifier presetTreeType;

    MomentumAudioProcessor& processor;
    juce::Array<juce::File> userPresetFiles;
    juce::File currentPresetFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
