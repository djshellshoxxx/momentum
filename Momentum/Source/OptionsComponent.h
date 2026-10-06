/*
    OptionsComponent.h

    The Options page reached from File > Options or the gear icon, as required by include.md:
    tooltips on/off, MIDI mapping management, and - when Momentum is running as a standalone
    application - audio card and MIDI input selection.

    Inside a plugin host the device selection is the host's job, so that section is replaced
    by a short note rather than shown disabled.
*/

#pragma once

#include "MomentumLookAndFeel.h"

#include <juce_audio_utils/juce_audio_utils.h>

#include <functional>

class MomentumAudioProcessor;

//==============================================================================
class OptionsComponent : public juce::Component
{
public:
    explicit OptionsComponent (MomentumAudioProcessor&);
    ~OptionsComponent() override;

    std::function<void()> onClose;
    std::function<void()> onTooltipsChanged;
    std::function<void()> onMidiMappingsCleared;

    /** Re-reads everything from the processor. Call each time the page is shown. */
    void refresh();

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    /** Builds the audio/MIDI device selector when running standalone; null otherwise. */
    void createDeviceSelectorIfStandalone();

    MomentumAudioProcessor& processor;

    juce::Label title;
    juce::TextButton closeButton { "Close" };

    juce::ToggleButton tooltipsToggle { "Show tooltips on hover" };
    juce::Label tooltipsHint;

    juce::TextButton clearMidiButton { "Clear all MIDI mappings" };
    juce::Label midiHint;

    juce::Label deviceHeading;
    juce::Label deviceHint;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> deviceSelector;
    juce::Viewport deviceViewport;

    juce::Label footer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OptionsComponent)
};
