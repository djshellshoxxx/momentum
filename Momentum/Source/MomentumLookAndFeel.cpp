#include "MomentumLookAndFeel.h"

//==============================================================================
namespace MomentumTheme
{
    static juce::String pickTypeface (const juce::StringArray& preferences, const juce::String& fallback)
    {
        static const juce::StringArray installed = juce::Font::findAllTypefaceNames();

        for (const auto& name : preferences)
            if (installed.contains (name))
                return name;

        return fallback;
    }

    juce::String uiTypefaceName()
    {
        static const juce::String name = pickTypeface ({ "Inter", "Space Grotesk",
                                                         "Segoe UI Variable Text", "Segoe UI" },
                                                       juce::Font::getDefaultSansSerifFontName());
        return name;
    }

    juce::String monoTypefaceName()
    {
        static const juce::String name = pickTypeface ({ "JetBrains Mono", "IBM Plex Mono",
                                                         "Cascadia Mono", "Consolas" },
                                                       juce::Font::getDefaultMonospacedFontName());
        return name;
    }

    juce::Font labelFont (float height)
    {
        return juce::Font (uiTypefaceName(), height, juce::Font::plain).withExtraKerningFactor (0.08f);
    }

    juce::Font headerFont (float height)
    {
        // JUCE has no semibold flag; bold at these sizes reads as the weight 600 the spec asks for.
        return juce::Font (uiTypefaceName(), height, juce::Font::bold);
    }

    juce::Font valueFont (float height)
    {
        return juce::Font (monoTypefaceName(), height, juce::Font::plain);
    }

    juce::String labelText (const juce::String& source)
    {
        return source.toUpperCase();
    }
}

//==============================================================================
MomentumLookAndFeel::MomentumLookAndFeel()
{
    using namespace MomentumTheme;

    setColour (juce::ResizableWindow::backgroundColourId, background);
    setColour (juce::DocumentWindow::textColourId,        textPrimary);

    setColour (juce::Label::textColourId,                 textPrimary);
    setColour (juce::Label::backgroundColourId,           juce::Colours::transparentBlack);
    setColour (juce::Label::outlineColourId,              juce::Colours::transparentBlack);

    setColour (juce::Slider::textBoxTextColourId,         textPrimary);
    setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId,    accent.withAlpha (0.30f));
    setColour (juce::Slider::rotarySliderFillColourId,    accent);
    setColour (juce::Slider::rotarySliderOutlineColourId, edge);
    setColour (juce::Slider::thumbColourId,               accent);
    setColour (juce::Slider::trackColourId,               accent);
    setColour (juce::Slider::backgroundColourId,          edge);

    setColour (juce::TextButton::buttonColourId,          panel);
    setColour (juce::TextButton::buttonOnColourId,        accent.withAlpha (0.15f));
    setColour (juce::TextButton::textColourOffId,         textMuted);
    setColour (juce::TextButton::textColourOnId,          accent);

    setColour (juce::ToggleButton::textColourId,          textPrimary);
    setColour (juce::ToggleButton::tickColourId,          accent);
    setColour (juce::ToggleButton::tickDisabledColourId,  edge);

    setColour (juce::ComboBox::backgroundColourId,        panel);
    setColour (juce::ComboBox::textColourId,              textPrimary);
    setColour (juce::ComboBox::outlineColourId,           edge);
    setColour (juce::ComboBox::arrowColourId,             textMuted);
    setColour (juce::ComboBox::buttonColourId,            panel);
    setColour (juce::ComboBox::focusedOutlineColourId,    accent);

    setColour (juce::PopupMenu::backgroundColourId,            panel);
    setColour (juce::PopupMenu::textColourId,                  textPrimary);
    setColour (juce::PopupMenu::headerTextColourId,            textMuted);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, accent.withAlpha (0.18f));
    setColour (juce::PopupMenu::highlightedTextColourId,       accent);

    setColour (juce::TooltipWindow::backgroundColourId,   juce::Colour (0xf0101419));
    setColour (juce::TooltipWindow::textColourId,         textMuted);
    setColour (juce::TooltipWindow::outlineColourId,      edge);

    setColour (juce::TextEditor::backgroundColourId,      background);
    setColour (juce::TextEditor::textColourId,            textPrimary);
    setColour (juce::TextEditor::outlineColourId,         edge);
    setColour (juce::TextEditor::focusedOutlineColourId,  accent);
    setColour (juce::TextEditor::highlightColourId,       accent.withAlpha (0.30f));
    setColour (juce::TextEditor::highlightedTextColourId, textPrimary);
    setColour (juce::CaretComponent::caretColourId,       accent);

    setColour (juce::ScrollBar::thumbColourId,            edge.brighter (0.30f));
    setColour (juce::ScrollBar::trackColourId,            juce::Colours::transparentBlack);

    setColour (juce::AlertWindow::backgroundColourId,     panel);
    setColour (juce::AlertWindow::textColourId,           textPrimary);
    setColour (juce::AlertWindow::outlineColourId,        edge);

    setColour (juce::ListBox::backgroundColourId,         background);
    setColour (juce::ListBox::textColourId,               textPrimary);

    setColour (juce::GroupComponent::outlineColourId,     edge);
    setColour (juce::GroupComponent::textColourId,        textMuted);

    setColour (juce::ProgressBar::backgroundColourId,     edge);
    setColour (juce::ProgressBar::foregroundColourId,     accent);
}

//==============================================================================
juce::ColourGradient MomentumLookAndFeel::bodyGradient (juce::Rectangle<float> bounds)
{
    // theme.md: knob body runs from #232833 at the top to #14181F at the bottom.
    return juce::ColourGradient (juce::Colour (0xff232833), bounds.getCentreX(), bounds.getY(),
                                 juce::Colour (0xff14181f), bounds.getCentreX(), bounds.getBottom(),
                                 false);
}

void MomentumLookAndFeel::drawSeparator (juce::Graphics& g, juce::Rectangle<int> row)
{
    g.setColour (MomentumTheme::edge);
    g.fillRect (row.getX(), row.getCentreY(), row.getWidth(), 1);
}

void MomentumLookAndFeel::drawSectionHeader (juce::Graphics& g, juce::Rectangle<int> area,
                                             const juce::String& text)
{
    using namespace MomentumTheme;

    // 2px wide, 12px tall accent bar to the left of the header text.
    const auto bar = juce::Rectangle<float> ((float) area.getX(), (float) area.getCentreY() - 6.0f,
                                             2.0f, 12.0f);
    g.setColour (accent);
    g.fillRect (bar);

    g.setColour (textPrimary);
    g.setFont (headerFont (12.0f).withExtraKerningFactor (0.08f));
    g.drawText (labelText (text), area.withTrimmedLeft (10), juce::Justification::centredLeft, false);
}

//==============================================================================
void MomentumLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPosProportional, float rotaryStartAngle,
                                            float rotaryEndAngle, juce::Slider& slider)
{
    using namespace MomentumTheme;

    const auto area   = juce::Rectangle<int> (x, y, width, height).toFloat();
    const auto centre = area.getCentre();

    constexpr float arcThickness = 3.0f;
    constexpr float arcGap       = 4.0f;
    constexpr float outerMargin  = 6.0f;

    const float outerRadius = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f - outerMargin;
    const float arcRadius   = outerRadius - arcThickness * 0.5f;
    const float bodyRadius  = juce::jmax (8.0f, arcRadius - arcThickness * 0.5f - arcGap);

    const bool enabled = slider.isEnabled();
    const bool hovered = slider.isMouseOverOrDragging();

    auto accentColour = slider.findColour (juce::Slider::rotarySliderFillColourId);

    if (! enabled)
        accentColour = accentColour.withSaturation (0.15f).withAlpha (0.45f);
    else if (hovered)
        accentColour = accentColour.brighter (0.08f); // hover brightens ~8%

    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    //-- value arc, 270 degree sweep drawn OUTSIDE the body --------------------
    {
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                             rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (edge.withAlpha (0.60f));
        g.strokePath (track, juce::PathStrokeType (arcThickness, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

        if (sliderPosProportional > 0.0005f)
        {
            juce::Path fill;
            fill.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                rotaryStartAngle, angle, true);
            g.setColour (accentColour);
            g.strokePath (fill, juce::PathStrokeType (arcThickness, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
        }
    }

    //-- body ------------------------------------------------------------------
    const auto body = juce::Rectangle<float> (bodyRadius * 2.0f, bodyRadius * 2.0f).withCentre (centre);

    g.setColour (shadow);
    g.fillEllipse (body.translated (0.0f, 2.0f).expanded (1.0f));

    g.setGradientFill (bodyGradient (body));
    g.fillEllipse (body);

    g.setColour (edge);
    g.drawEllipse (body.reduced (0.5f), 1.0f);

    //-- indicator: single 2px accent line from centre to rim, rounded cap ------
    {
        const float inner = bodyRadius * 0.42f;
        const float outer = bodyRadius - 4.0f;
        const juce::Point<float> p1 (centre.x + inner * std::sin (angle), centre.y - inner * std::cos (angle));
        const juce::Point<float> p2 (centre.x + outer * std::sin (angle), centre.y - outer * std::cos (angle));

        g.setColour (accentColour);
        g.drawLine (juce::Line<float> (p1, p2), 2.0f);
    }

    //-- centre readout (start.md: value inside the knob, 14pt bold) -----------
    {
        const bool atDefault = slider.isDoubleClickReturnEnabled()
                                 && std::abs (slider.getValue() - slider.getDoubleClickReturnValue()) < 1.0e-6;

        g.setColour (enabled ? (atDefault ? textPrimary : accentColour) : textMuted);
        g.setFont (juce::Font (monoTypefaceName(), 14.0f, juce::Font::bold));
        g.drawText (slider.getTextFromValue (slider.getValue()),
                    body.reduced (bodyRadius * 0.20f).translated (0.0f, -2.0f),
                    juce::Justification::centred, false);

        //-- 4px centre dot, muted while the control sits at its default --------
        const auto dot = juce::Rectangle<float> (4.0f, 4.0f)
                             .withCentre ({ centre.x, centre.y + bodyRadius * 0.52f });
        g.setColour (atDefault ? edge.brighter (0.25f) : accentColour);
        g.fillEllipse (dot);
    }
}

//==============================================================================
int MomentumLookAndFeel::getSliderThumbRadius (juce::Slider& slider)
{
    if (slider.isHorizontal() || slider.isVertical())
        return 8; // half of the 16px thumb width

    return LookAndFeel_V4::getSliderThumbRadius (slider);
}

void MomentumLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float minSliderPos, float maxSliderPos,
                                            juce::Slider::SliderStyle style, juce::Slider& slider)
{
    using namespace MomentumTheme;

    if (! slider.isHorizontal())
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos,
                                          minSliderPos, maxSliderPos, style, slider);
        return;
    }

    const auto area = juce::Rectangle<int> (x, y, width, height).toFloat();

    constexpr float trackThickness = 4.0f;
    constexpr float thumbWidth     = 16.0f;
    constexpr float thumbHeight    = 24.0f;

    const float centreY = area.getCentreY();
    const float left    = area.getX() + thumbWidth * 0.5f;
    const float right   = area.getRight() - thumbWidth * 0.5f;

    auto accentColour = slider.findColour (juce::Slider::trackColourId);

    if (! slider.isEnabled())
        accentColour = accentColour.withSaturation (0.15f).withAlpha (0.45f);
    else if (slider.isMouseOverOrDragging())
        accentColour = accentColour.brighter (0.08f);

    //-- ticks: 1px muted lines at a logical interval, outside the track -------
    {
        const double interval = (double) slider.getProperties().getWithDefault ("tickInterval", 0.0);

        if (interval > 0.0)
        {
            g.setColour (edge);

            for (double v = slider.getMinimum(); v <= slider.getMaximum() + 1.0e-9; v += interval)
            {
                const auto prop = (float) slider.valueToProportionOfLength (v);
                const float tx = juce::jmap (prop, left, right);
                g.fillRect (tx, centreY + thumbHeight * 0.5f + 3.0f, 1.0f, 4.0f);
            }
        }
    }

    //-- track -----------------------------------------------------------------
    const auto track = juce::Rectangle<float> (left, centreY - trackThickness * 0.5f,
                                               right - left, trackThickness);
    g.setColour (edge);
    g.fillRoundedRectangle (track, trackThickness * 0.5f);

    //-- fill: bipolar controls grow out from zero, not from the left edge -----
    const bool bipolar = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
    const float originX = bipolar ? juce::jmap ((float) slider.valueToProportionOfLength (0.0), left, right)
                                  : left;
    const float posX = juce::jlimit (left, right, sliderPos);

    if (std::abs (posX - originX) > 0.5f)
    {
        const auto fill = juce::Rectangle<float> (juce::jmin (originX, posX), track.getY(),
                                                  std::abs (posX - originX), trackThickness);
        g.setColour (accentColour);
        g.fillRoundedRectangle (fill, trackThickness * 0.5f);
    }

    //-- thumb: 16x24 rounded rect, knob body gradient, 1px accent stroke ------
    {
        const auto thumb = juce::Rectangle<float> (thumbWidth, thumbHeight).withCentre ({ posX, centreY });

        g.setColour (shadow);
        g.fillRoundedRectangle (thumb.translated (0.0f, 2.0f), radiusPanel);

        g.setGradientFill (bodyGradient (thumb));
        g.fillRoundedRectangle (thumb, radiusPanel);

        g.setColour (accentColour);
        g.drawRoundedRectangle (thumb.reduced (0.5f), radiusPanel, 1.0f);
    }
}

//==============================================================================
void MomentumLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                const juce::Colour&,
                                                bool shouldDrawButtonAsHighlighted,
                                                bool shouldDrawButtonAsDown)
{
    using namespace MomentumTheme;

    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = button.getToggleState();
    const auto accentColour = button.findColour (juce::TextButton::textColourOnId);

    if (on || shouldDrawButtonAsDown)
    {
        // Momentary presses show a brief accent flash - that is the "as down" state.
        g.setColour (accentColour.withAlpha (shouldDrawButtonAsDown ? 0.28f : 0.15f));
        g.fillRoundedRectangle (bounds, radiusPanel);
        g.setColour (accentColour.withAlpha (0.85f));
        g.drawRoundedRectangle (bounds, radiusPanel, 1.0f);
    }
    else
    {
        g.setColour (shouldDrawButtonAsHighlighted ? panel.brighter (0.08f) : panel);
        g.fillRoundedRectangle (bounds, radiusPanel);
        g.setColour (edge);
        g.drawRoundedRectangle (bounds, radiusPanel, 1.0f);
    }
}

void MomentumLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    using namespace MomentumTheme;

    const bool lit = button.getToggleState() || shouldDrawButtonAsDown;
    auto colour = lit ? button.findColour (juce::TextButton::textColourOnId)
                      : button.findColour (juce::TextButton::textColourOffId);

    if (shouldDrawButtonAsHighlighted && ! lit)
        colour = colour.brighter (0.30f);

    if (! button.isEnabled())
        colour = colour.withAlpha (0.40f);

    g.setColour (colour);
    g.setFont (getTextButtonFont (button, button.getHeight()));
    g.drawText (labelText (button.getButtonText()), button.getLocalBounds(),
                juce::Justification::centred, false);
}

void MomentumLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                            bool shouldDrawButtonAsHighlighted, bool)
{
    using namespace MomentumTheme;

    const auto bounds = button.getLocalBounds().toFloat();
    constexpr float boxSize = 16.0f;
    const auto box = juce::Rectangle<float> (bounds.getX() + 0.5f,
                                             bounds.getCentreY() - boxSize * 0.5f,
                                             boxSize, boxSize);

    const bool on = button.getToggleState();

    g.setColour (on ? accent.withAlpha (0.15f)
                    : (shouldDrawButtonAsHighlighted ? panel.brighter (0.08f) : panel));
    g.fillRoundedRectangle (box, radiusControl);

    g.setColour (on ? accent : edge);
    g.drawRoundedRectangle (box.reduced (0.5f), radiusControl, 1.0f);

    if (on)
    {
        juce::Path tick;
        tick.startNewSubPath (box.getX() + 4.0f,       box.getCentreY());
        tick.lineTo          (box.getCentreX() - 1.0f, box.getBottom() - 5.0f);
        tick.lineTo          (box.getRight() - 4.0f,   box.getY() + 5.0f);

        g.setColour (accent);
        g.strokePath (tick, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    g.setColour (button.isEnabled() ? button.findColour (juce::ToggleButton::textColourId) : textMuted);
    g.setFont (labelFont (11.0f));
    g.drawText (labelText (button.getButtonText()),
                bounds.withTrimmedLeft (boxSize + 10.0f), juce::Justification::centredLeft, false);
}

juce::Font MomentumLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return MomentumTheme::labelFont (11.0f);
}

//==============================================================================
void MomentumLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                        int, int, int, int, juce::ComboBox& box)
{
    using namespace MomentumTheme;

    const auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);
    const bool hovered = box.isMouseOver (true);

    g.setColour (hovered ? panel.brighter (0.08f) : panel);
    g.fillRoundedRectangle (bounds, radiusPanel);

    g.setColour (box.hasKeyboardFocus (false) ? accent.withAlpha (0.60f) : edge);
    g.drawRoundedRectangle (bounds, radiusPanel, 1.0f);

    juce::Path arrow;
    const float cx = (float) width - 13.0f;
    const float cy = (float) height * 0.5f;
    arrow.startNewSubPath (cx - 4.0f, cy - 2.0f);
    arrow.lineTo          (cx,        cy + 2.5f);
    arrow.lineTo          (cx + 4.0f, cy - 2.0f);

    g.setColour (box.isEnabled() ? (hovered ? accent : textMuted) : edge);
    g.strokePath (arrow, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
}

juce::Font MomentumLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (MomentumTheme::uiTypefaceName(), 12.0f, juce::Font::plain);
}

void MomentumLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (8, 0, box.getWidth() - 28, box.getHeight());
    label.setFont (getComboBoxFont (box));
    label.setColour (juce::Label::textColourId, box.findColour (juce::ComboBox::textColourId));
    label.setJustificationType (juce::Justification::centredLeft);
}

//==============================================================================
void MomentumLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    using namespace MomentumTheme;

    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);

    g.setColour (panel);
    g.fillRoundedRectangle (bounds, radiusPanel);
    g.setColour (edge);
    g.drawRoundedRectangle (bounds, radiusPanel, 1.0f);
}

juce::Font MomentumLookAndFeel::getPopupMenuFont()
{
    return juce::Font (MomentumTheme::uiTypefaceName(), 13.0f, juce::Font::plain);
}

//==============================================================================
void MomentumLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height)
{
    using namespace MomentumTheme;

    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);
    const float radius = juce::jmin (bounds.getHeight() * 0.5f, 10.0f); // dark pill

    g.setColour (shadow);
    g.fillRoundedRectangle (bounds.translated (0.0f, 2.0f), radius);

    g.setColour (findColour (juce::TooltipWindow::backgroundColourId));
    g.fillRoundedRectangle (bounds, radius);

    g.setColour (edge);
    g.drawRoundedRectangle (bounds, radius, 1.0f);

    juce::AttributedString s;
    s.setJustification (juce::Justification::centred);
    s.append (text, juce::Font (uiTypefaceName(), 12.0f, juce::Font::plain),
              findColour (juce::TooltipWindow::textColourId));

    juce::TextLayout layout;
    layout.createLayout (s, bounds.getWidth() - 20.0f);
    layout.draw (g, bounds.reduced (10.0f, 6.0f));
}

juce::Rectangle<int> MomentumLookAndFeel::getTooltipBounds (const juce::String& tipText,
                                                           juce::Point<int> screenPos,
                                                           juce::Rectangle<int> parentArea)
{
    juce::AttributedString s;
    s.setJustification (juce::Justification::centred);
    s.append (tipText, juce::Font (MomentumTheme::uiTypefaceName(), 12.0f, juce::Font::plain));

    juce::TextLayout layout;
    layout.createLayoutWithBalancedLineLengths (s, 320.0f);

    const int w = (int) layout.getWidth()  + 20;
    const int h = (int) layout.getHeight() + 14;

    return juce::Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12)
                                                                      : screenPos.x + 24,
                                 screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6)
                                                                      : screenPos.y + 6,
                                 w, h)
             .constrainedWithin (parentArea);
}

//==============================================================================
void MomentumLookAndFeel::drawScrollbar (juce::Graphics& g, juce::ScrollBar& bar,
                                         int x, int y, int width, int height,
                                         bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                                         bool isMouseOver, bool isMouseDown)
{
    using namespace MomentumTheme;

    juce::ignoreUnused (bar);

    if (thumbSize <= 0)
        return;

    const auto thumb = isScrollbarVertical
                         ? juce::Rectangle<int> (x + width / 2 - 2, thumbStartPosition, 4, thumbSize)
                         : juce::Rectangle<int> (thumbStartPosition, y + height / 2 - 2, thumbSize, 4);

    g.setColour (isMouseDown ? accent
                             : (isMouseOver ? edge.brighter (0.60f) : edge.brighter (0.25f)));
    g.fillRoundedRectangle (thumb.toFloat(), 2.0f);
}
