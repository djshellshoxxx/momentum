/*
    MomentumControls.h

    The two custom widgets the editor is built from:

      MomentumSlider      - a Slider that honours the interaction rules in theme.md
                            (80ms eased value animation, modifier-scaled drag, double-click
                            reset) and the right-click behaviour required by include.md.

      GainReductionMeter  - the vertical GR meter: fills downward from the top, peak hold,
                            numeric readout.
*/

#pragma once

#include "MomentumLookAndFeel.h"

#include <functional>

//==============================================================================
class MomentumSlider : public juce::Slider
{
public:
    MomentumSlider();

    /** Index into the MomentumParams table. Lets the editor identify which control was hit. */
    void setParameterIndex (int index) noexcept     { parameterIndex = index; }
    int  getParameterIndex() const noexcept         { return parameterIndex; }

    /** The editor supplies the menu because it owns the processor and the preset state. */
    std::function<void (MomentumSlider&)> onContextMenu;

    /** Advances the 80ms ease towards the real value. Returns true when a repaint is needed. */
    bool advanceAnimation (float secondsElapsed);

    /** Skips the animation, e.g. after a preset load. */
    void snapAnimationToValue();

    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void paint (juce::Graphics&) override;

private:
    float targetProportion();

    int parameterIndex { -1 };
    float animatedProportion { 0.0f };
    bool animationPrimed { false };

    static constexpr int baseDragSensitivity = 160;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MomentumSlider)
};

//==============================================================================
class GainReductionMeter : public juce::Component,
                           public juce::SettableTooltipClient
{
public:
    GainReductionMeter();

    /** Feed the processor's block-peak gain reduction, in positive dB. */
    void update (float gainReductionDb, float secondsElapsed);

    void paint (juce::Graphics&) override;

    /** Held peak in positive dB, for the numeric readout the editor draws beside the bar. */
    float getPeakDb() const noexcept    { return peakDb; }

private:
    static constexpr float rangeDb    = 18.0f;  // meter bottom, per start.md
    static constexpr float holdTimeS  = 1.5f;   // theme.md peak hold
    static constexpr float fallDbPerS = 20.0f;  // theme.md peak fall rate

    float displayedDb { 0.0f };
    float peakDb { 0.0f };
    float holdRemaining { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainReductionMeter)
};
