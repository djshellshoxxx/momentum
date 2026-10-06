#include "MomentumControls.h"

//==============================================================================
MomentumSlider::MomentumSlider()
{
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    setMouseDragSensitivity (baseDragSensitivity);
    setVelocityBasedMode (false);
    setWantsKeyboardFocus (false);
}

float MomentumSlider::targetProportion()
{
    const auto range = getRange();

    if (range.getLength() <= 0.0)
        return 0.0f;

    return (float) valueToProportionOfLength (getValue());
}

void MomentumSlider::snapAnimationToValue()
{
    animatedProportion = targetProportion();
    animationPrimed = true;
    repaint();
}

bool MomentumSlider::advanceAnimation (float secondsElapsed)
{
    const float target = targetProportion();

    if (! animationPrimed)
    {
        animatedProportion = target;
        animationPrimed = true;
        return true;
    }

    const float delta = target - animatedProportion;

    if (std::abs (delta) < 0.0005f)
    {
        if (animatedProportion != target)
        {
            animatedProportion = target;
            return true;
        }

        return false;
    }

    // theme.md: value changes animate over 80ms, ease-out. A one-pole with tau = 80/3 ms
    // lands within a pixel of the target in roughly that time at any sensible frame rate.
    constexpr float tau = 0.080f / 3.0f;
    const float alpha = 1.0f - std::exp (-juce::jmax (0.0f, secondsElapsed) / tau);

    animatedProportion += delta * juce::jlimit (0.0f, 1.0f, alpha);
    return true;
}

//==============================================================================
void MomentumSlider::mouseDown (const juce::MouseEvent& event)
{
    if (event.mods.isPopupMenu())
    {
        if (onContextMenu != nullptr)
            onContextMenu (*this);

        return;
    }

    // theme.md: Shift is coarse, Ctrl/Cmd is ultra-fine, plain drag is fine.
    if (event.mods.isCtrlDown() || event.mods.isCommandDown())
        setMouseDragSensitivity (baseDragSensitivity * 6);
    else if (event.mods.isShiftDown())
        setMouseDragSensitivity (juce::jmax (20, baseDragSensitivity / 4));
    else
        setMouseDragSensitivity (baseDragSensitivity);

    Slider::mouseDown (event);
}

void MomentumSlider::mouseUp (const juce::MouseEvent& event)
{
    Slider::mouseUp (event);
    setMouseDragSensitivity (baseDragSensitivity);
}

void MomentumSlider::mouseEnter (const juce::MouseEvent& event)
{
    Slider::mouseEnter (event);
    repaint();
}

void MomentumSlider::mouseExit (const juce::MouseEvent& event)
{
    Slider::mouseExit (event);
    repaint();
}

void MomentumSlider::paint (juce::Graphics& g)
{
    if (! animationPrimed)
        snapAnimationToValue();

    const auto bounds = getLocalBounds();

    if (isRotary())
    {
        const auto rotary = getRotaryParameters();

        getLookAndFeel().drawRotarySlider (g, bounds.getX(), bounds.getY(),
                                           bounds.getWidth(), bounds.getHeight(),
                                           juce::jlimit (0.0f, 1.0f, animatedProportion),
                                           rotary.startAngleRadians, rotary.endAngleRadians, *this);
        return;
    }

    if (isHorizontal())
    {
        constexpr float thumbWidth = 16.0f;
        const float left  = (float) bounds.getX() + thumbWidth * 0.5f;
        const float right = (float) bounds.getRight() - thumbWidth * 0.5f;
        const float pos = juce::jmap (juce::jlimit (0.0f, 1.0f, animatedProportion), left, right);

        getLookAndFeel().drawLinearSlider (g, bounds.getX(), bounds.getY(),
                                           bounds.getWidth(), bounds.getHeight(),
                                           pos, left, right, getSliderStyle(), *this);
        return;
    }

    Slider::paint (g);
}

//==============================================================================
GainReductionMeter::GainReductionMeter()
{
    setInterceptsMouseClicks (false, false);
}

void GainReductionMeter::update (float gainReductionDb, float secondsElapsed)
{
    const float incoming = juce::jlimit (0.0f, rangeDb, gainReductionDb);
    const float dt = juce::jmax (0.0f, secondsElapsed);

    // The processor already applies Recovery smoothing, so the bar only needs enough
    // smoothing to stop it flickering between frames.
    displayedDb += (incoming - displayedDb) * 0.55f;

    if (displayedDb >= peakDb)
    {
        peakDb = displayedDb;
        holdRemaining = holdTimeS;
    }
    else if (holdRemaining > 0.0f)
    {
        holdRemaining -= dt;
    }
    else
    {
        peakDb = juce::jmax (displayedDb, peakDb - fallDbPerS * dt);
    }

    repaint();
}

void GainReductionMeter::paint (juce::Graphics& g)
{
    using namespace MomentumTheme;

    const auto bounds = getLocalBounds().toFloat();

    //-- track -----------------------------------------------------------------
    g.setColour (edge.withAlpha (0.60f));
    g.fillRoundedRectangle (bounds, radiusControl);

    //-- fill: grows DOWNWARD from the top as gain reduction increases ---------
    const float proportion = juce::jlimit (0.0f, 1.0f, displayedDb / rangeDb);

    if (proportion > 0.001f)
    {
        const auto fill = bounds.withHeight (bounds.getHeight() * proportion);

        // theme.md asks for a graded meter; the amber of start.md sits at the nominal
        // middle, with teal below it and the house orange reserved for the deep end.
        juce::ColourGradient gradient (accentAlt, bounds.getCentreX(), bounds.getY(),
                                       danger,    bounds.getCentreX(), bounds.getBottom(), false);
        gradient.addColour (0.35, accent);
        gradient.addColour (0.70, warning);

        g.setGradientFill (gradient);
        g.fillRoundedRectangle (fill, radiusControl);
    }

    //-- 1px peak hold line ----------------------------------------------------
    if (peakDb > 0.05f)
    {
        const float y = bounds.getY() + bounds.getHeight() * juce::jlimit (0.0f, 1.0f, peakDb / rangeDb);
        g.setColour (textPrimary.withAlpha (0.85f));
        g.fillRect (bounds.getX(), juce::jmin (y, bounds.getBottom() - 1.0f), bounds.getWidth(), 1.0f);
    }

    g.setColour (edge);
    g.drawRoundedRectangle (bounds.reduced (0.5f), radiusControl, 1.0f);
}
