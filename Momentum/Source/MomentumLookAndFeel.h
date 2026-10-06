/*
    MomentumLookAndFeel.h

    House visual identity (theme.md) for the Momentum plugin.

    The neutral palette, control shapes, spacing grid and interaction feel are shared
    across every plugin in the range. theme.md allows each plugin to re-tint exactly one
    accent for its own identity; Momentum re-tints the primary accent from the house
    burnt orange (#E8532A) to amber (#FFB020). Everything else is untouched.
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/** Colour tokens. Never hard-code a colour anywhere else in the project. */
namespace MomentumTheme
{
    const juce::Colour background   { 0xff0e1116 }; // near-black, slight blue undertone
    const juce::Colour panel        { 0xff171b22 }; // raised sections, module backgrounds
    const juce::Colour edge         { 0xff2a303a }; // 1px inner strokes, subtle separators
    const juce::Colour accent       { 0xffffb020 }; // PRIMARY - Momentum's amber re-tint
    const juce::Colour accentAlt    { 0xff4fb6c4 }; // secondary, muted teal
    const juce::Colour textPrimary  { 0xffe6e8ec };
    const juce::Colour textMuted    { 0xff8a929e };
    const juce::Colour success      { 0xff7bc96f };
    const juce::Colour warning      { 0xfff2c14e };
    const juce::Colour danger       { 0xffe8532a }; // house orange, kept for clip only
    const juce::Colour shadow       { 0x8c000000 }; // rgba(0,0,0,0.55)

    /** 8px base grid. Fine detail may use 4. */
    constexpr int grid = 8;

    /** Corner radii: 6px main window, 4px inner panels, 2px small controls. */
    constexpr float radiusWindow  = 6.0f;
    constexpr float radiusPanel   = 4.0f;
    constexpr float radiusControl = 2.0f;

    /** Returns the first installed face from the house preference list. */
    juce::String uiTypefaceName();
    juce::String monoTypefaceName();

    /** 11px uppercase +0.08em - section and control labels. */
    juce::Font labelFont (float height = 11.0f);
    /** 14px semibold - section headers and the plugin name. */
    juce::Font headerFont (float height = 14.0f);
    /** Tabular mono for every numeric readout. */
    juce::Font valueFont (float height = 13.0f);

    /** Uppercases and letter-spaces a label the way the spec asks for. */
    juce::String labelText (const juce::String& source);
}

//==============================================================================
/** LookAndFeel implementing the house control shapes. */
class MomentumLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MomentumLookAndFeel();
    ~MomentumLookAndFeel() override = default;

    //== Knobs ==================================================================
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    //== Sliders / faders =======================================================
    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    int getSliderThumbRadius (juce::Slider&) override;

    //== Buttons / toggles ======================================================
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    //== Combo box ==============================================================
    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

    //== Popup menu =============================================================
    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;
    juce::Font getPopupMenuFont() override;

    //== Tooltip - dark pill, muted text ========================================
    void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;
    juce::Rectangle<int> getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos,
                                           juce::Rectangle<int> parentArea) override;

    //== Scrollbar ==============================================================
    void drawScrollbar (juce::Graphics&, juce::ScrollBar&, int x, int y, int width, int height,
                        bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                        bool isMouseOver, bool isMouseDown) override;

    //== Shared drawing helpers used by the editor ==============================
    /** 1px horizontal separator in edge colour - never boxes inside boxes. */
    static void drawSeparator (juce::Graphics&, juce::Rectangle<int> row);
    /** Uppercase section header with a 2px x 12px accent bar to its left. */
    static void drawSectionHeader (juce::Graphics&, juce::Rectangle<int> area, const juce::String& text);
    /** The knob-body gradient, reused by slider thumbs so they match. */
    static juce::ColourGradient bodyGradient (juce::Rectangle<float> bounds);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MomentumLookAndFeel)
};
