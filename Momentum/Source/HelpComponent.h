/*
    HelpComponent.h

    The in-plugin help page required by include.md: what every control does, how to work
    with the plugin, a description of the GUI, and the version number.
*/

#pragma once

#include "MomentumLookAndFeel.h"

#include <functional>
#include <vector>

//==============================================================================
/** The scrolling body of the help page. */
class HelpContent : public juce::Component
{
public:
    HelpContent();

    /** Lays the text out for the given width and returns the height it needs. */
    int layoutForWidth (int width);

    void paint (juce::Graphics&) override;

private:
    struct Block
    {
        enum class Kind { section, heading, body, bullet };

        Kind kind;
        juce::String text;
        juce::TextLayout layout;
        int y { 0 };
        int height { 0 };
    };

    static juce::Font fontFor (Block::Kind);
    static juce::Colour colourFor (Block::Kind);
    static int indentFor (Block::Kind);
    static int spacingAfter (Block::Kind);

    std::vector<Block> blocks;
    int laidOutWidth { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelpContent)
};

//==============================================================================
/** Full-window overlay holding the help page. */
class HelpComponent : public juce::Component
{
public:
    HelpComponent();

    std::function<void()> onClose;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Label title;
    juce::TextButton closeButton { "Close" };
    juce::Viewport viewport;
    HelpContent content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelpComponent)
};
