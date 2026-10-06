/*
    PluginEditor.h

    Momentum's interface. Laid out on theme.md's 8px grid, using the house palette with the
    primary accent re-tinted to amber, and carrying every item include.md requires of a
    plugin in this range: a File menu, presets, A/B compare, reset, random, tooltips,
    right-click value entry and MIDI learn, a help page and an options page.
*/

#pragma once

#include "PluginProcessor.h"
#include "PresetManager.h"
#include "MomentumLookAndFeel.h"
#include "MomentumControls.h"
#include "HelpComponent.h"
#include "OptionsComponent.h"

#include <array>
#include <memory>

//==============================================================================
/** Small vector-drawn icon button for the header strip. */
class HeaderIconButton : public juce::Button
{
public:
    enum class Icon { gear, swap };

    HeaderIconButton (Icon iconToDraw, const juce::String& componentName);

    void paintButton (juce::Graphics&, bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override;

private:
    Icon icon;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderIconButton)
};

//==============================================================================
class MomentumAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit MomentumAudioProcessorEditor (MomentumAudioProcessor&);
    ~MomentumAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    //== Window geometry ========================================================
    static constexpr int windowWidth  = 720;
    static constexpr int windowHeight = 344;

private:
    //== Timer ==================================================================
    void timerCallback() override;

    //== Building blocks ========================================================
    void setUpKnob (MomentumSlider&, juce::Label&, int parameterIndex);
    void setUpMakeupSlider();
    void setUpHeader();
    void setUpUtilityButtons();
    void applyTooltips();

    //== Actions ================================================================
    void showFileMenu();
    void showControlContextMenu (MomentumSlider&);
    void showValueEntryFor (MomentumSlider&);

    void refreshPresetMenu();
    void syncPresetSelection();
    void handlePresetSelection (int menuItemID);

    void doSavePreset (bool forceAskForName);
    void doOpenPreset();

    void showOverlay (juce::Component* overlayToShow);
    void hideOverlay();

    void switchToSlot (int slot);
    void copyActiveSlotToOther();
    void snapAllAnimations();

    //== Layout helpers =========================================================
    juce::Rectangle<int> knobColumn (int index) const;

    //== References =============================================================
    MomentumAudioProcessor& processor;
    juce::SharedResourcePointer<MomentumLookAndFeel> lookAndFeel;
    PresetManager presetManager;

    //== Header =================================================================
    juce::TextButton fileButton { "File" };
    juce::ComboBox presetBox;
    juce::TextButton slotAButton { "A" };
    juce::TextButton slotBButton { "B" };
    HeaderIconButton copySlotButton { HeaderIconButton::Icon::swap, "Copy slot" };
    HeaderIconButton optionsButton  { HeaderIconButton::Icon::gear, "Options" };

    //== Detector ===============================================================
    static constexpr int numKnobs = 4;

    std::array<MomentumSlider, numKnobs> knobs;
    std::array<juce::Label, numKnobs> knobLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, numKnobs> knobAttachments;

    GainReductionMeter meter;

    //== Output =================================================================
    MomentumSlider makeupSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> makeupAttachment;

    //== Utility ================================================================
    juce::TextButton resetButton  { "Reset" };
    juce::TextButton randomButton { "Random" };
    juce::TextButton helpButton   { "Help" };

    //== Overlays ===============================================================
    HelpComponent helpPage;
    OptionsComponent optionsPage;
    juce::Component* visibleOverlay { nullptr };

    //== Transient UI ===========================================================
    std::unique_ptr<juce::TooltipWindow> tooltipWindow;
    std::unique_ptr<juce::FileChooser> fileChooser;
    std::unique_ptr<juce::AlertWindow> valueEntryWindow;

    //== A/B state ==============================================================
    std::array<std::array<float, MomentumParams::numParameters>, 2> abSlots;
    int activeSlot { 0 };

    //== Bookkeeping ============================================================
    juce::uint32 lastTimerTicksMs { 0 };
    int lastSeenMidiRevision { -1 };
    juce::String lastSeenPresetName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MomentumAudioProcessorEditor)
};
