#include "HelpComponent.h"
#include "PluginProcessor.h"

//==============================================================================
namespace
{
    struct RawBlock
    {
        int kind;   // 0 section, 1 heading, 2 body, 3 bullet
        const char* text;
    };

    const RawBlock helpText[] =
    {
        { 0, "What Momentum does" },
        { 2, "Momentum is a compressor that listens to how FAST your signal is getting louder, "
             "not to how loud it is. A normal compressor asks \"are you above the threshold?\". "
             "Momentum asks \"did you just jump?\"." },
        { 2, "Two loudness followers run side by side on the input. A fast one tracks the last "
             "few milliseconds; a slow one is fixed at 300 ms and represents where the music "
             "has been sitting. The gap between them, in dB, is called Delta, and it is a direct "
             "measure of how sudden a move is. A crescendo that builds over two bars keeps Delta "
             "near zero and is left completely alone. A shout, a stick that lands twice as hard, "
             "or a door slamming in the live room produces a large Delta and gets held down." },
        { 2, "The practical result: your performance keeps its shape, and only the surprises "
             "get controlled." },

        { 0, "The controls" },

        { 1, "SENSITIVITY  (0 to 24 dB, default 6 dB)" },
        { 2, "The Delta threshold. Momentum ignores everything until the fast follower is this "
             "many dB ahead of the slow one. Turn it down to catch more of the performance; turn "
             "it up until only genuinely unexpected events are caught. On a mix bus, 12 to 16 dB "
             "is a safety net. On a shouty vocal, 3 to 5 dB is an active leveller." },

        { 1, "AMOUNT  (0 to 100%, default 60%)" },
        { 2, "How much of the excess is converted into gain reduction. At 100%, 6 dB of excess "
             "Delta becomes 6 dB of gain reduction, which is effectively a hard clamp on sudden "
             "moves. At 50% the same event becomes 3 dB, which reads as a firm hand rather than "
             "a wall. This is the control to reach for when the effect is right but too strong." },

        { 1, "SPEED  (1 to 50 ms, default 8 ms)" },
        { 2, "The window of the fast follower. Short settings hear individual transients - stick "
             "hits, pick attacks, consonants. Longer settings hear whole syllables and phrases, "
             "so the plugin responds to a loud WORD rather than a loud letter. If Momentum is "
             "reacting to things you do not hear as events, lengthen Speed." },

        { 1, "RECOVERY  (20 to 1000 ms, default 150 ms)" },
        { 2, "How quickly gain reduction lets go once the sudden moment has passed. Attack is "
             "always instantaneous - that is the entire point of the design, and there is no "
             "attack control. Short recovery keeps the plugin invisible between events. Long "
             "recovery lets one loud hit duck the following bar, which is the classic pumping "
             "sound if you want it." },

        { 1, "MAKEUP  (-12 to +12 dB, default 0 dB)" },
        { 2, "Plain output trim, applied after compression. Use it to match loudness when you "
             "bypass and un-bypass, so you are judging the compression instead of the level." },

        { 0, "How to work with it" },
        { 3, "Start from a factory preset close to your source, or from Init." },
        { 3, "Set Amount to 100% temporarily. This exaggerates what the detector is doing and "
             "makes the next step much easier to hear." },
        { 3, "Sweep Sensitivity down until the gain reduction meter only moves on the events you "
             "actually want held. Ignore everything else at this stage." },
        { 3, "Adjust Speed so the meter reacts to events, not to texture. If it is twitching "
             "constantly, Speed is too short." },
        { 3, "Set Recovery by ear: short if you want the plugin to disappear between events, "
             "longer if you want an audible duck." },
        { 3, "Bring Amount back down until the control feels natural rather than clamped." },
        { 3, "Finally set Makeup so bypassed and processed are the same subjective loudness." },

        { 0, "The interface" },
        { 2, "Along the top strip you will find the plugin name, the preset selector, the A/B "
             "compare buttons, the File menu and the gear icon for Options." },
        { 2, "The DETECTOR section holds the four main knobs: Sensitivity, Amount, Speed and "
             "Recovery. Each knob shows its current value in the centre, and the amber arc around "
             "the outside shows where it sits in its range. The small dot under the value is dim "
             "while the control is at its default and lights up once you move it." },
        { 2, "To the right of the knobs is the gain reduction meter. It fills downward from the "
             "top as Momentum works, over a range of 0 to -18 dB, with a thin peak-hold line and "
             "a numeric readout above it." },
        { 2, "The OUTPUT section is the Makeup fader, with tick marks every 6 dB and a centre "
             "detent at 0 dB." },
        { 2, "Along the bottom are RESET, which returns every control to its default, RANDOM, "
             "which generates a fresh set of musically plausible settings on every press, and "
             "HELP, which opens this page." },

        { 0, "Mouse and keyboard" },
        { 3, "Drag a knob vertically to change it." },
        { 3, "Hold Shift while dragging for coarse movement." },
        { 3, "Hold Ctrl while dragging for ultra-fine movement." },
        { 3, "Double-click any control to return it to its default." },
        { 3, "Right-click any control for: type in an exact value, reset to default, MIDI Learn, "
             "and clear the MIDI mapping." },
        { 3, "Hover over anything for a tooltip. Tooltips can be switched off in Options." },

        { 0, "Presets" },
        { 2, "The selector in the top strip lists the factory bank first, then any presets you "
             "have saved yourself. Use File > Save Preset As to add your own; they are written as "
             ".mmtp files and can be copied between machines. File > Open Preset loads a .mmtp "
             "from anywhere on disk, and File > Open Preset Folder shows you where they live." },
        { 2, "A and B are two complete snapshots of every control. Set up a sound, press B, "
             "change things, then click between A and B to compare. Copy A to B duplicates the "
             "current slot into the other one so you can start a variation from where you are." },

        { 0, "MIDI control" },
        { 2, "Right-click a control and choose MIDI Learn, then move a knob or fader on your "
             "controller. That CC now drives the parameter, and the binding is saved with the "
             "session. A CC can only drive one parameter at a time, so learning it onto a second "
             "control releases it from the first. Clear MIDI Mapping removes a single binding; "
             "Options has a button that clears them all." },

        { 0, "Options" },
        { 2, "The gear icon opens Options, where you can switch tooltips on or off and clear all "
             "MIDI mappings. When Momentum is running as a standalone application, Options also "
             "contains the audio device, sample rate, buffer size and MIDI input selection. Inside "
             "a plugin host those are handled by the host itself." },

        { 0, "A note on exporting audio" },
        { 2, "Momentum is an effect, not an instrument, so it has no audio export of its own. "
             "Render or bounce from your host as usual." }
    };
}

//==============================================================================
HelpContent::HelpContent()
{
    blocks.reserve (juce::numElementsInArray (helpText) + 2);

    for (const auto& raw : helpText)
    {
        Block block;
        block.kind = static_cast<Block::Kind> (raw.kind);
        block.text = juce::String::fromUTF8 (raw.text);
        blocks.push_back (std::move (block));
    }

    // include.md requires the version number to be reachable from the help page.
    Block versionHeading;
    versionHeading.kind = Block::Kind::section;
    versionHeading.text = "Version";
    blocks.push_back (std::move (versionHeading));

    Block version;
    version.kind = Block::Kind::body;
    version.text = "Momentum " + MomentumAudioProcessor::getVersionString()
                     + " " + MomentumAudioProcessor::getEditionName()
                     + "  -  dynamic-derivative compressor  -  Sheldon Davidson.";
    blocks.push_back (std::move (version));

    Block edition;
    edition.kind = Block::Kind::body;
    edition.text = MomentumEdition::describeEdition();
    blocks.push_back (std::move (edition));
}

juce::Font HelpContent::fontFor (Block::Kind kind)
{
    switch (kind)
    {
        case Block::Kind::section:  return MomentumTheme::headerFont (13.0f).withExtraKerningFactor (0.08f);
        case Block::Kind::heading:  return MomentumTheme::headerFont (12.0f);
        case Block::Kind::bullet:   return juce::Font (MomentumTheme::uiTypefaceName(), 12.5f, juce::Font::plain);
        case Block::Kind::body:
        default:                    return juce::Font (MomentumTheme::uiTypefaceName(), 12.5f, juce::Font::plain);
    }
}

juce::Colour HelpContent::colourFor (Block::Kind kind)
{
    switch (kind)
    {
        case Block::Kind::section:  return MomentumTheme::accent;
        case Block::Kind::heading:  return MomentumTheme::textPrimary;
        case Block::Kind::bullet:   return MomentumTheme::textMuted;
        case Block::Kind::body:
        default:                    return MomentumTheme::textMuted;
    }
}

int HelpContent::indentFor (Block::Kind kind)
{
    return kind == Block::Kind::bullet ? 16 : 0;
}

int HelpContent::spacingAfter (Block::Kind kind)
{
    switch (kind)
    {
        case Block::Kind::section:  return 8;
        case Block::Kind::heading:  return 4;
        case Block::Kind::bullet:   return 6;
        case Block::Kind::body:
        default:                    return 12;
    }
}

int HelpContent::layoutForWidth (int width)
{
    laidOutWidth = width;

    int y = 0;
    bool first = true;

    for (auto& block : blocks)
    {
        if (block.kind == Block::Kind::section && ! first)
            y += 12;

        first = false;

        const int indent = indentFor (block.kind);
        const auto text = block.kind == Block::Kind::bullet
                            ? juce::String (juce::CharPointer_UTF8 ("\xe2\x80\xa2  ")) + block.text
                            : (block.kind == Block::Kind::section ? block.text.toUpperCase() : block.text);

        juce::AttributedString attributed;
        attributed.setWordWrap (juce::AttributedString::byWord);
        attributed.setLineSpacing (2.0f);
        attributed.append (text, fontFor (block.kind), colourFor (block.kind));

        block.layout.createLayout (attributed, (float) juce::jmax (40, width - indent));
        block.y = y;
        block.height = (int) std::ceil (block.layout.getHeight());

        y += block.height + spacingAfter (block.kind);
    }

    return y;
}

void HelpContent::paint (juce::Graphics& g)
{
    if (laidOutWidth != getWidth())
        layoutForWidth (getWidth());

    for (const auto& block : blocks)
    {
        const int indent = indentFor (block.kind);

        if (block.kind == Block::Kind::section)
        {
            // 2px accent bar to the left of every section header, as in the main window.
            g.setColour (MomentumTheme::accent);
            g.fillRect (0.0f, (float) block.y + 2.0f, 2.0f, 12.0f);
            block.layout.draw (g, juce::Rectangle<float> (10.0f, (float) block.y,
                                                          (float) getWidth() - 10.0f,
                                                          (float) block.height));
            continue;
        }

        block.layout.draw (g, juce::Rectangle<float> ((float) indent, (float) block.y,
                                                      (float) (getWidth() - indent),
                                                      (float) block.height));
    }
}

//==============================================================================
HelpComponent::HelpComponent()
{
    title.setText ("Help", juce::dontSendNotification);
    title.setFont (MomentumTheme::headerFont (14.0f));
    title.setColour (juce::Label::textColourId, MomentumTheme::textPrimary);
    addAndMakeVisible (title);

    closeButton.onClick = [this] { if (onClose != nullptr) onClose(); };
    addAndMakeVisible (closeButton);

    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);
}

void HelpComponent::paint (juce::Graphics& g)
{
    using namespace MomentumTheme;

    const auto bounds = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (background.withAlpha (0.96f));
    g.fillRoundedRectangle (bounds, radiusWindow);

    g.setColour (edge);
    g.drawRoundedRectangle (bounds, radiusWindow, 1.0f);

    MomentumLookAndFeel::drawSeparator (g, juce::Rectangle<int> (16, 40, getWidth() - 32, 1));
}

void HelpComponent::resized()
{
    auto area = getLocalBounds().reduced (16, 12);

    auto headerRow = area.removeFromTop (28);
    closeButton.setBounds (headerRow.removeFromRight (72).withSizeKeepingCentre (72, 24));
    title.setBounds (headerRow);

    area.removeFromTop (12);

    viewport.setBounds (area);

    const int contentWidth = juce::jmax (100, viewport.getMaximumVisibleWidth() - 8);
    const int contentHeight = content.layoutForWidth (contentWidth);
    content.setSize (contentWidth, contentHeight);
}
