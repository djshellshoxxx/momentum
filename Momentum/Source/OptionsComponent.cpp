#include "OptionsComponent.h"
#include "PluginProcessor.h"

#if JucePlugin_Build_Standalone
 #include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
 #define MOMENTUM_CAN_REACH_STANDALONE_HOLDER 1
#else
 #define MOMENTUM_CAN_REACH_STANDALONE_HOLDER 0
#endif

//==============================================================================
namespace
{
    void styleHint (juce::Label& label, const juce::String& text)
    {
        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::Font (MomentumTheme::uiTypefaceName(), 11.5f, juce::Font::plain));
        label.setColour (juce::Label::textColourId, MomentumTheme::textMuted);
        label.setJustificationType (juce::Justification::topLeft);
        label.setMinimumHorizontalScale (1.0f);
    }
}

//==============================================================================
OptionsComponent::OptionsComponent (MomentumAudioProcessor& p)
    : processor (p)
{
    title.setText ("Options", juce::dontSendNotification);
    title.setFont (MomentumTheme::headerFont (14.0f));
    title.setColour (juce::Label::textColourId, MomentumTheme::textPrimary);
    addAndMakeVisible (title);

    closeButton.onClick = [this] { if (onClose != nullptr) onClose(); };
    addAndMakeVisible (closeButton);

    tooltipsToggle.onClick = [this]
    {
        processor.setTooltipsEnabled (tooltipsToggle.getToggleState());

        if (onTooltipsChanged != nullptr)
            onTooltipsChanged();
    };
    addAndMakeVisible (tooltipsToggle);

    styleHint (tooltipsHint, "Hover help appears after a short delay over every knob, fader "
                             "and button. Turn it off once you know your way around.");
    addAndMakeVisible (tooltipsHint);

    clearMidiButton.onClick = [this]
    {
        processor.getMidiLearn().clearAllMappings();

        if (onMidiMappingsCleared != nullptr)
            onMidiMappingsCleared();
    };
    addAndMakeVisible (clearMidiButton);

    styleHint (midiHint, "Right-click any control and choose MIDI Learn to bind a controller. "
                         "This removes every binding at once.");
    addAndMakeVisible (midiHint);

    deviceHeading.setText ("Audio and MIDI", juce::dontSendNotification);
    deviceHeading.setFont (MomentumTheme::labelFont (11.0f));
    deviceHeading.setColour (juce::Label::textColourId, MomentumTheme::textPrimary);
    addAndMakeVisible (deviceHeading);

    addAndMakeVisible (deviceHint);

    createDeviceSelectorIfStandalone();

    footer.setText ("Momentum " + MomentumAudioProcessor::getVersionString(), juce::dontSendNotification);
    footer.setFont (juce::Font (MomentumTheme::monoTypefaceName(), 9.0f, juce::Font::plain));
    footer.setColour (juce::Label::textColourId, MomentumTheme::textMuted);
    footer.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (footer);

    refresh();
}

OptionsComponent::~OptionsComponent()
{
    // The selector holds a reference to the standalone holder's device manager, so it has
    // to go before this component does.
    deviceViewport.setViewedComponent (nullptr, false);
    deviceSelector.reset();
}

void OptionsComponent::createDeviceSelectorIfStandalone()
{
   #if MOMENTUM_CAN_REACH_STANDALONE_HOLDER
    if (auto* holder = juce::StandalonePluginHolder::getInstance())
    {
        deviceSelector = std::make_unique<juce::AudioDeviceSelectorComponent> (
                             holder->deviceManager,
                             0, 256,   // input channels: min, max
                             0, 256,   // output channels: min, max
                             true,     // show MIDI input options
                             false,    // show MIDI output selector
                             true,     // show channels as stereo pairs
                             false);   // hide the advanced options button

        deviceViewport.setViewedComponent (deviceSelector.get(), false);
        deviceViewport.setScrollBarsShown (true, false);
        addAndMakeVisible (deviceViewport);

        styleHint (deviceHint, "Choose the audio device, sample rate, buffer size and MIDI "
                               "inputs used by the standalone application.");
        return;
    }
   #endif

    styleHint (deviceHint, "Momentum is running inside a host, so the audio device, sample rate, "
                           "buffer size and MIDI routing are controlled by the host rather than "
                           "by the plugin.");
}

//==============================================================================
void OptionsComponent::refresh()
{
    tooltipsToggle.setToggleState (processor.getTooltipsEnabled(), juce::dontSendNotification);

    int mappingCount = 0;

    for (int i = 0; i < MomentumParams::numParameters; ++i)
        if (processor.getMidiLearn().getControllerFor (i) >= 0)
            ++mappingCount;

    clearMidiButton.setEnabled (mappingCount > 0);
    clearMidiButton.setButtonText (mappingCount > 0
                                     ? "Clear all MIDI mappings (" + juce::String (mappingCount) + ")"
                                     : "No MIDI mappings");
}

//==============================================================================
void OptionsComponent::paint (juce::Graphics& g)
{
    using namespace MomentumTheme;

    const auto bounds = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (background.withAlpha (0.96f));
    g.fillRoundedRectangle (bounds, radiusWindow);

    g.setColour (edge);
    g.drawRoundedRectangle (bounds, radiusWindow, 1.0f);

    MomentumLookAndFeel::drawSeparator (g, juce::Rectangle<int> (16, 40, getWidth() - 32, 1));

    // 2px accent bar beside the device section heading.
    g.setColour (accent);
    g.fillRect ((float) deviceHeading.getX() - 10.0f, (float) deviceHeading.getBounds().getCentreY() - 6.0f,
                2.0f, 12.0f);

    MomentumLookAndFeel::drawSeparator (g, juce::Rectangle<int> (16, deviceHeading.getY() - 12,
                                                                 getWidth() - 32, 1));
}

void OptionsComponent::resized()
{
    auto area = getLocalBounds().reduced (16, 12);

    auto headerRow = area.removeFromTop (28);
    closeButton.setBounds (headerRow.removeFromRight (72).withSizeKeepingCentre (72, 24));
    title.setBounds (headerRow);

    area.removeFromTop (16);

    auto footerRow = area.removeFromBottom (14);
    footer.setBounds (footerRow);
    area.removeFromBottom (8);

    tooltipsToggle.setBounds (area.removeFromTop (24).withTrimmedLeft (10));
    area.removeFromTop (2);
    tooltipsHint.setBounds (area.removeFromTop (30).withTrimmedLeft (10));

    area.removeFromTop (12);

    clearMidiButton.setBounds (area.removeFromTop (28).withTrimmedLeft (10).withWidth (220));
    area.removeFromTop (4);
    midiHint.setBounds (area.removeFromTop (30).withTrimmedLeft (10));

    area.removeFromTop (20);

    deviceHeading.setBounds (area.removeFromTop (16).withTrimmedLeft (10));
    area.removeFromTop (4);

    if (deviceSelector != nullptr)
    {
        deviceHint.setBounds (area.removeFromTop (18).withTrimmedLeft (10));
        area.removeFromTop (4);

        deviceViewport.setBounds (area);

        const int selectorWidth = juce::jmax (200, deviceViewport.getMaximumVisibleWidth());
        deviceSelector->setSize (selectorWidth, juce::jmax (area.getHeight(), 340));
    }
    else
    {
        deviceHint.setBounds (area.removeFromTop (44).withTrimmedLeft (10));
    }
}
