#include "PluginEditor.h"

//==============================================================================
namespace Layout
{
    constexpr int pad          = 16;

    constexpr int headerHeight = 32;
    constexpr int headerSep    = 32;

    constexpr int detectorHeaderY = 44;
    constexpr int knobY           = 66;
    constexpr int knobBox         = 96;
    constexpr int knobLabelY      = 164;
    constexpr int knobLabelHeight = 14;

    constexpr int detectorSep  = 190;

    constexpr int outputHeaderY = 200;
    constexpr int makeupY       = 220;
    constexpr int makeupHeight  = 40;

    constexpr int outputSep     = 272;

    constexpr int buttonY       = 282;
    constexpr int buttonHeight  = 28;
    constexpr int buttonWidth   = 88;

    constexpr int footerY       = 318;

    // Gain reduction meter block, to the right of the knob row.
    constexpr int meterBlockX   = 640;
    constexpr int meterBlockW   = 64;
    constexpr int meterBarW     = 10;   // start.md
    constexpr int meterBarH     = 80;   // start.md
    constexpr int meterBarY     = 84;
    constexpr int meterReadoutY = 66;
    constexpr int meterLabelY   = 168;

    constexpr int knobAreaRight = 624;
}

//==============================================================================
HeaderIconButton::HeaderIconButton (Icon iconToDraw, const juce::String& componentName)
    : juce::Button (componentName), icon (iconToDraw)
{
    setWantsKeyboardFocus (false);
}

void HeaderIconButton::paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
                                    bool shouldDrawButtonAsDown)
{
    using namespace MomentumTheme;

    const auto bounds = getLocalBounds().toFloat();
    const auto centre = bounds.getCentre();

    auto colour = shouldDrawButtonAsDown ? accent
                                         : (shouldDrawButtonAsHighlighted ? accent.withAlpha (0.85f)
                                                                          : textMuted);

    g.setColour (colour);

    if (icon == Icon::gear)
    {
        const float outer = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.34f;
        const float inner = outer * 0.52f;

        juce::Path gear;

        for (int i = 0; i < 8; ++i)
        {
            const float angle = juce::MathConstants<float>::twoPi * (float) i / 8.0f;
            const float toothLength = outer * 0.30f;

            juce::Rectangle<float> tooth (2.4f, toothLength);
            tooth.setCentre (centre.x + (outer + toothLength * 0.35f) * std::sin (angle),
                             centre.y - (outer + toothLength * 0.35f) * std::cos (angle));

            juce::Path toothPath;
            toothPath.addRoundedRectangle (tooth, 1.0f);
            toothPath.applyTransform (juce::AffineTransform::rotation (angle, tooth.getCentreX(),
                                                                       tooth.getCentreY()));
            gear.addPath (toothPath);
        }

        g.fillPath (gear);
        g.drawEllipse (juce::Rectangle<float> (outer * 2.0f, outer * 2.0f).withCentre (centre), 1.8f);
        g.drawEllipse (juce::Rectangle<float> (inner * 2.0f, inner * 2.0f).withCentre (centre), 1.6f);
    }
    else // swap: two arrows pointing in opposite directions
    {
        const float w = juce::jmin (bounds.getWidth() * 0.52f, 14.0f);
        const float h = 4.0f;

        for (int row = 0; row < 2; ++row)
        {
            const float y = centre.y + (row == 0 ? -h : h);
            const float dir = row == 0 ? 1.0f : -1.0f;

            g.drawLine (centre.x - w * 0.5f, y, centre.x + w * 0.5f, y, 1.6f);

            juce::Path head;
            const float tipX = centre.x + dir * w * 0.5f;
            head.startNewSubPath (tipX - dir * 3.5f, y - 3.0f);
            head.lineTo (tipX, y);
            head.lineTo (tipX - dir * 3.5f, y + 3.0f);

            g.strokePath (head, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
        }
    }
}

//==============================================================================
MomentumAudioProcessorEditor::MomentumAudioProcessorEditor (MomentumAudioProcessor& p)
    : juce::AudioProcessorEditor (&p),
      processor (p),
      presetManager (p),
      optionsPage (p)
{
    setLookAndFeel (&lookAndFeel.get());

    setUpHeader();

    for (int i = 0; i < numKnobs; ++i)
        setUpKnob (knobs[(size_t) i], knobLabels[(size_t) i], i);

    setUpMakeupSlider();
    setUpUtilityButtons();

    addAndMakeVisible (meter);

    helpPage.onClose = [this] { hideOverlay(); };
    helpPage.setVisible (false);
    addChildComponent (helpPage);

    optionsPage.onClose = [this] { hideOverlay(); };
    optionsPage.onTooltipsChanged = [this] { applyTooltips(); };
    optionsPage.onMidiMappingsCleared = [this] { optionsPage.refresh(); repaint(); };
    optionsPage.setVisible (false);
    addChildComponent (optionsPage);

    // Both A/B slots start from whatever the host handed us.
    const auto current = processor.captureParameterValues();
    abSlots[0] = current;
    abSlots[1] = current;
    activeSlot = 0;
    slotAButton.setToggleState (true, juce::dontSendNotification);

    applyTooltips();
    refreshPresetMenu();
    syncPresetSelection();

    setResizable (false, false);
    setSize (windowWidth, windowHeight);

    snapAllAnimations();

    lastTimerTicksMs = juce::Time::getMillisecondCounter();
    startTimerHz (30);
}

MomentumAudioProcessorEditor::~MomentumAudioProcessorEditor()
{
    stopTimer();

    if (valueEntryWindow != nullptr)
    {
        valueEntryWindow->exitModalState (0);
        valueEntryWindow->setLookAndFeel (nullptr);
        valueEntryWindow.reset();
    }

    setLookAndFeel (nullptr);
}

//==============================================================================
void MomentumAudioProcessorEditor::setUpHeader()
{
    fileButton.onClick = [this] { showFileMenu(); };
    addAndMakeVisible (fileButton);

    presetBox.setTextWhenNothingSelected ("Presets");
    presetBox.setJustificationType (juce::Justification::centredLeft);
    presetBox.onChange = [this] { handlePresetSelection (presetBox.getSelectedId()); };
    addAndMakeVisible (presetBox);

    slotAButton.setClickingTogglesState (false);
    slotBButton.setClickingTogglesState (false);
    slotAButton.onClick = [this] { switchToSlot (0); };
    slotBButton.onClick = [this] { switchToSlot (1); };
    if constexpr (MomentumEdition::allowABCompare)
    {
        addAndMakeVisible (slotAButton);
        addAndMakeVisible (slotBButton);
    }

    copySlotButton.onClick = [this] { copyActiveSlotToOther(); };

    if constexpr (MomentumEdition::allowABCompare)
        addAndMakeVisible (copySlotButton);

    optionsButton.onClick = [this]
    {
        optionsPage.refresh();
        showOverlay (&optionsPage);
    };
    addAndMakeVisible (optionsButton);
}

void MomentumAudioProcessorEditor::setUpKnob (MomentumSlider& knob, juce::Label& label, int parameterIndex)
{
    const auto& info = MomentumParams::get (parameterIndex);

    knob.setParameterIndex (parameterIndex);
    knob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);

    // 270 degree sweep, 7 o'clock to 5 o'clock.
    knob.setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);

    knob.setDoubleClickReturnValue (true, info.defaultValue);
    knob.onContextMenu = [this] (MomentumSlider& s) { showControlContextMenu (s); };

    addAndMakeVisible (knob);

    knobAttachments[(size_t) parameterIndex] =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.getAPVTS(), info.id, knob);

    label.setText (juce::String (info.name).toUpperCase(), juce::dontSendNotification);
    label.setFont (MomentumTheme::labelFont (11.0f));
    label.setColour (juce::Label::textColourId, MomentumTheme::textMuted);
    label.setJustificationType (juce::Justification::centred);
    label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);
}

void MomentumAudioProcessorEditor::setUpMakeupSlider()
{
    const auto& info = MomentumParams::get (MomentumParams::kMakeup);

    makeupSlider.setParameterIndex (MomentumParams::kMakeup);
    makeupSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    makeupSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    makeupSlider.setDoubleClickReturnValue (true, info.defaultValue);
    makeupSlider.getProperties().set ("tickInterval", 6.0); // theme.md: a tick every 6 dB
    makeupSlider.onContextMenu = [this] (MomentumSlider& s) { showControlContextMenu (s); };
    addAndMakeVisible (makeupSlider);

    makeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
                           processor.getAPVTS(), info.id, makeupSlider);
}

void MomentumAudioProcessorEditor::setUpUtilityButtons()
{
    resetButton.onClick = [this]
    {
        processor.resetToDefaults();
        snapAllAnimations();
        syncPresetSelection();
    };
    addAndMakeVisible (resetButton);

    randomButton.onClick = [this]
    {
        processor.randomiseParameters();
        snapAllAnimations();
        syncPresetSelection();
    };
    addAndMakeVisible (randomButton);

    helpButton.onClick = [this] { showOverlay (&helpPage); };
    addAndMakeVisible (helpButton);
}

//==============================================================================
void MomentumAudioProcessorEditor::applyTooltips()
{
    const bool enabled = processor.getTooltipsEnabled();

    if (enabled)
    {
        if (tooltipWindow == nullptr)
        {
            // theme.md: tooltips appear after 400ms of hover.
            tooltipWindow = std::make_unique<juce::TooltipWindow> (this, 400);
            tooltipWindow->setLookAndFeel (&lookAndFeel.get());
        }
    }
    else if (tooltipWindow != nullptr)
    {
        tooltipWindow->setLookAndFeel (nullptr);
        tooltipWindow.reset();
    }

    for (int i = 0; i < numKnobs; ++i)
        knobs[(size_t) i].setTooltip (MomentumParams::get (i).tooltip);

    makeupSlider.setTooltip (MomentumParams::get (MomentumParams::kMakeup).tooltip);

    meter.setTooltip ("Gain reduction, 0 to -18 dB. Fills downward; the thin line is the held peak.");

    fileButton.setTooltip ("Save, open and manage presets, and reach the Options page.");
    presetBox.setTooltip ("Factory and user presets. User presets are listed below the factory bank.");
    slotAButton.setTooltip ("A/B slot A. Click to load slot A, keeping your current settings in the other slot.");
    slotBButton.setTooltip ("A/B slot B. Click to load slot B, keeping your current settings in the other slot.");
    copySlotButton.setTooltip ("Copy the active A/B slot into the other one.");
    optionsButton.setTooltip ("Options: tooltips, MIDI mappings, and audio device setup when standalone.");

    resetButton.setTooltip ("Return every control to its default value.");
    randomButton.setTooltip ("Reset, then generate a completely new set of settings. Press again for another.");
    helpButton.setTooltip ("Open the help page: what each control does, and how to work with Momentum.");
}

//==============================================================================
void MomentumAudioProcessorEditor::timerCallback()
{
    const auto now = juce::Time::getMillisecondCounter();
    const float elapsed = juce::jlimit (0.001f, 0.25f, (float) (now - lastTimerTicksMs) * 0.001f);
    lastTimerTicksMs = now;

    meter.update (processor.getCurrentGainReductionDb(), elapsed);

    // The numeric readout beside the meter changes with it.
    repaint (Layout::meterBlockX, Layout::meterReadoutY, Layout::meterBlockW, 16);

    for (auto& knob : knobs)
        if (knob.advanceAnimation (elapsed))
            knob.repaint();

    if (makeupSlider.advanceAnimation (elapsed))
        makeupSlider.repaint();

    if (const int revision = processor.getMidiMapRevision(); revision != lastSeenMidiRevision)
    {
        lastSeenMidiRevision = revision;
        optionsPage.refresh();
        repaint();
    }

    if (const auto name = processor.getCurrentPresetName(); name != lastSeenPresetName)
        syncPresetSelection();
}

//==============================================================================
juce::Rectangle<int> MomentumAudioProcessorEditor::knobColumn (int index) const
{
    const int columnWidth = (Layout::knobAreaRight - Layout::pad) / numKnobs;
    return { Layout::pad + index * columnWidth, 0, columnWidth, windowHeight };
}

void MomentumAudioProcessorEditor::resized()
{
    //-- header ----------------------------------------------------------------
    optionsButton .setBounds (680, 4, 24, 24);
    copySlotButton.setBounds (640, 4, 32, 24);
    slotBButton   .setBounds (606, 4, 26, 24);
    slotAButton   .setBounds (576, 4, 26, 24);
    presetBox     .setBounds (372, 4, 196, 24);
    fileButton    .setBounds (308, 4, 56, 24);

    //-- knobs -----------------------------------------------------------------
    for (int i = 0; i < numKnobs; ++i)
    {
        const auto column = knobColumn (i);

        knobs[(size_t) i].setBounds (column.getCentreX() - Layout::knobBox / 2, Layout::knobY,
                                     Layout::knobBox, Layout::knobBox);

        knobLabels[(size_t) i].setBounds (column.getX(), Layout::knobLabelY,
                                          column.getWidth(), Layout::knobLabelHeight);
    }

    //-- meter -----------------------------------------------------------------
    meter.setBounds (Layout::meterBlockX + (Layout::meterBlockW - Layout::meterBarW) / 2,
                     Layout::meterBarY, Layout::meterBarW, Layout::meterBarH);

    //-- makeup ----------------------------------------------------------------
    makeupSlider.setBounds (Layout::pad, Layout::makeupY,
                            windowWidth - Layout::pad * 2, Layout::makeupHeight);

    //-- utility buttons -------------------------------------------------------
    resetButton .setBounds (Layout::pad,       Layout::buttonY, Layout::buttonWidth, Layout::buttonHeight);
    randomButton.setBounds (Layout::pad + 96,  Layout::buttonY, Layout::buttonWidth, Layout::buttonHeight);
    helpButton  .setBounds (Layout::pad + 192, Layout::buttonY, Layout::buttonWidth, Layout::buttonHeight);

    //-- overlays --------------------------------------------------------------
    const auto overlayBounds = getLocalBounds().reduced (8);
    helpPage.setBounds (overlayBounds);
    optionsPage.setBounds (overlayBounds);
}

//==============================================================================
void MomentumAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace MomentumTheme;

    g.fillAll (background);

    //-- header strip ----------------------------------------------------------
    g.setColour (panel);
    g.fillRect (0, 0, getWidth(), Layout::headerHeight);

    g.setColour (textPrimary);
    g.setFont (headerFont (14.0f));
    g.drawText ("MOMENTUM", Layout::pad, 0, 100, Layout::headerHeight,
                juce::Justification::centredLeft, false);

    g.setColour (textMuted);
    g.setFont (juce::Font (uiTypefaceName(), 10.0f, juce::Font::plain));
    g.drawText ("dynamic-derivative compressor", 112, 0, 190, Layout::headerHeight,
                juce::Justification::centredLeft, false);

    MomentumLookAndFeel::drawSeparator (g, { 0, Layout::headerSep, getWidth(), 1 });

    //-- section headers -------------------------------------------------------
    MomentumLookAndFeel::drawSectionHeader (g, { Layout::pad, Layout::detectorHeaderY, 260, 14 },
                                            "Detector");

    MomentumLookAndFeel::drawSectionHeader (g, { Layout::pad, Layout::outputHeaderY, 260, 14 },
                                            "Output");

    MomentumLookAndFeel::drawSeparator (g, { Layout::pad, Layout::detectorSep,
                                             getWidth() - Layout::pad * 2, 1 });
    MomentumLookAndFeel::drawSeparator (g, { Layout::pad, Layout::outputSep,
                                             getWidth() - Layout::pad * 2, 1 });

    //-- makeup value, right-aligned on the OUTPUT header row -----------------
    g.setColour (accent);
    g.setFont (valueFont (13.0f));
    g.drawText (makeupSlider.getTextFromValue (makeupSlider.getValue()),
                getWidth() - Layout::pad - 90, Layout::outputHeaderY, 90, 14,
                juce::Justification::centredRight, false);

    //-- MIDI mapping badges under any mapped knob -----------------------------
    g.setFont (juce::Font (monoTypefaceName(), 9.0f, juce::Font::plain));

    for (int i = 0; i < numKnobs; ++i)
    {
        const int cc = processor.getMidiLearn().getControllerFor (i);

        if (cc >= 0)
        {
            const auto column = knobColumn (i);
            g.setColour (accentAlt);
            g.drawText ("CC " + juce::String (cc), column.getX(), Layout::knobLabelY + 14,
                        column.getWidth(), 10, juce::Justification::centred, false);
        }
    }

    if (const int makeupCC = processor.getMidiLearn().getControllerFor (MomentumParams::kMakeup); makeupCC >= 0)
    {
        g.setColour (accentAlt);
        g.drawText ("CC " + juce::String (makeupCC), getWidth() - Layout::pad - 160,
                    Layout::outputHeaderY, 60, 14, juce::Justification::centredRight, false);
    }

    //-- gain reduction meter block -------------------------------------------
    {
        g.setColour (textPrimary);
        g.setFont (valueFont (11.0f));
        g.drawText (juce::String (-meter.getPeakDb(), 1),
                    Layout::meterBlockX, Layout::meterReadoutY, Layout::meterBlockW, 14,
                    juce::Justification::centred, false);

        g.setColour (textMuted);
        g.setFont (labelFont (10.0f));
        g.drawText ("GR", Layout::meterBlockX, Layout::meterLabelY, Layout::meterBlockW, 12,
                    juce::Justification::centred, false);
    }

    //-- hint beside the utility buttons ---------------------------------------
    g.setColour (textMuted.withAlpha (0.75f));
    g.setFont (juce::Font (uiTypefaceName(), 10.5f, juce::Font::plain));
    g.drawText ("Right-click any control for value entry and MIDI learn",
                Layout::pad + 296, Layout::buttonY, getWidth() - Layout::pad * 2 - 296,
                Layout::buttonHeight, juce::Justification::centredRight, false);

    //-- footer: version in 9px muted mono, bottom right -----------------------
    g.setColour (textMuted);
    g.setFont (juce::Font (monoTypefaceName(), 9.0f, juce::Font::plain));
    g.drawText ("v" + MomentumAudioProcessor::getVersionString()
                    + " " + MomentumAudioProcessor::getEditionName(),
                getWidth() - Layout::pad - 120, Layout::footerY, 120, 12,
                juce::Justification::centredRight, false);

    //-- signature: 2px accent notch across the top-left corner, 12px at 45deg -
    g.setColour (accent);
    g.drawLine (0.0f, 12.0f, 12.0f, 0.0f, 2.0f);
}

//==============================================================================
void MomentumAudioProcessorEditor::showFileMenu()
{
    presetManager.refreshUserPresets();

    juce::PopupMenu menu;
    menu.setLookAndFeel (&lookAndFeel.get());

    menu.addItem (1, "New (reset to defaults)");

    if constexpr (MomentumEdition::allowUserPresets)
    {
        menu.addSeparator();
        menu.addItem (2, "Open Preset...");
        menu.addItem (3, "Save Preset", presetManager.getCurrentPresetFile().existsAsFile());
        menu.addItem (4, "Save Preset As...");
        menu.addSeparator();
        menu.addItem (5, "Open Preset Folder");
    }
    else
    {
        // Lite: say why the entries are missing rather than silently dropping them.
        menu.addSeparator();
        menu.addItem (100, "Saving your own presets is a Pro feature", false, false);
    }

    menu.addSeparator();
    menu.addItem (6, "Options...");
    menu.addItem (7, "Help...");

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (fileButton)
                                                  .withMinimumWidth (200),
                        [this] (int result)
    {
        switch (result)
        {
            case 1:
                processor.resetToDefaults();
                presetManager.clearCurrentPresetFile();
                snapAllAnimations();
                syncPresetSelection();
                break;

            case 2:  doOpenPreset();          break;
            case 3:  doSavePreset (false);    break;
            case 4:  doSavePreset (true);     break;

            case 5:  PresetManager::getUserPresetDirectory().revealToUser(); break;

            case 6:
                optionsPage.refresh();
                showOverlay (&optionsPage);
                break;

            case 7:  showOverlay (&helpPage); break;

            default: break;
        }
    });
}

//==============================================================================
void MomentumAudioProcessorEditor::showControlContextMenu (MomentumSlider& slider)
{
    const int index = slider.getParameterIndex();

    if (! juce::isPositiveAndBelow (index, (int) MomentumParams::numParameters))
        return;

    auto& midiLearn = processor.getMidiLearn();
    const int mappedCC = midiLearn.getControllerFor (index);
    const bool learning = midiLearn.getLearnTarget() == index;

    juce::PopupMenu menu;
    menu.setLookAndFeel (&lookAndFeel.get());

    menu.addSectionHeader (MomentumParams::get (index).name);
    menu.addItem (1, "Set value...");
    menu.addItem (2, "Reset to default");

    if constexpr (MomentumEdition::allowMidiLearn)
    {
        menu.addSeparator();
        menu.addItem (3, learning ? "Listening... move a MIDI control" : "MIDI Learn", true, learning);
        menu.addItem (4, mappedCC >= 0 ? "Clear MIDI mapping (CC " + juce::String (mappedCC) + ")"
                                       : "Clear MIDI mapping",
                      mappedCC >= 0);
    }
    else
    {
        menu.addSeparator();
        menu.addItem (100, "MIDI Learn is a Pro feature", false, false);
    }

    auto* sliderPointer = &slider;

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (slider).withMinimumWidth (220),
                        [this, sliderPointer, index] (int result)
    {
        switch (result)
        {
            case 1:
                showValueEntryFor (*sliderPointer);
                break;

            case 2:
                sliderPointer->setValue (MomentumParams::get (index).defaultValue,
                                         juce::sendNotificationSync);
                break;

            case 3:
                processor.getMidiLearn().setLearnTarget (
                    processor.getMidiLearn().getLearnTarget() == index ? -1 : index);
                repaint();
                break;

            case 4:
                processor.getMidiLearn().clearMapping (index);
                optionsPage.refresh();
                repaint();
                break;

            default:
                break;
        }
    });
}

void MomentumAudioProcessorEditor::showValueEntryFor (MomentumSlider& slider)
{
    const int index = slider.getParameterIndex();

    if (! juce::isPositiveAndBelow (index, (int) MomentumParams::numParameters))
        return;

    const auto& info = MomentumParams::get (index);

    if (valueEntryWindow != nullptr)
        return;

    valueEntryWindow = std::make_unique<juce::AlertWindow> (
                           juce::String (info.name),
                           "Enter a value between " + juce::String (info.minValue, 1) + " and "
                             + juce::String (info.maxValue, 1) + " " + info.unit + ".",
                           juce::MessageBoxIconType::NoIcon, this);

    valueEntryWindow->setLookAndFeel (&lookAndFeel.get());
    valueEntryWindow->addTextEditor ("value", slider.getTextFromValue (slider.getValue()), "Value:");
    valueEntryWindow->addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey));
    valueEntryWindow->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    auto* sliderPointer = &slider;
    const juce::Component::SafePointer<MomentumAudioProcessorEditor> safeThis (this);

    valueEntryWindow->enterModalState (true, juce::ModalCallbackFunction::create (
        [this, sliderPointer, safeThis] (int result)
        {
            if (result == 1 && valueEntryWindow != nullptr)
            {
                const auto text = valueEntryWindow->getTextEditorContents ("value");
                sliderPointer->setValue (sliderPointer->getValueFromText (text),
                                         juce::sendNotificationSync);
            }

            // Tear the window down once the modal callback has unwound.
            juce::MessageManager::callAsync ([safeThis]
            {
                if (safeThis == nullptr || safeThis->valueEntryWindow == nullptr)
                    return;

                safeThis->valueEntryWindow->setLookAndFeel (nullptr);
                safeThis->valueEntryWindow.reset();
            });
        }), false);
}

//==============================================================================
void MomentumAudioProcessorEditor::refreshPresetMenu()
{
    presetManager.refreshUserPresets();

    presetBox.clear (juce::dontSendNotification);

    auto* rootMenu = presetBox.getRootMenu();
    rootMenu->addSectionHeader ("Factory");

    const auto& factory = PresetManager::getFactoryPresets();
    int shownFactory = 0;

    for (int i = 0; i < (int) factory.size(); ++i)
    {
        if (! MomentumEdition::isFactoryPresetVisible (i))
            continue;

        // The item ID stays (bank index + 1) even when the visible set is not a contiguous
        // run, so handlePresetSelection() and syncPresetSelection() need no special case.
        presetBox.addItem (factory[(size_t) i].name, i + 1);
        ++shownFactory;
    }

    if (shownFactory < (int) factory.size())
    {
        // Not selectable: handlePresetSelection() would otherwise route ID 900 into the
        // factory bank. Disabled here and bounds-checked there.
        presetBox.addItem ("More presets in Pro", 900);
        presetBox.setItemEnabled (900, false);
    }

    const auto& userFiles = presetManager.getUserPresetFiles();

    if (! userFiles.isEmpty())
    {
        rootMenu->addSectionHeader ("User");

        for (int i = 0; i < userFiles.size(); ++i)
            presetBox.addItem (userFiles.getReference (i).getFileNameWithoutExtension(), 1000 + i);
    }
}

void MomentumAudioProcessorEditor::syncPresetSelection()
{
    const auto name = processor.getCurrentPresetName();
    lastSeenPresetName = name;

    const auto& factory = PresetManager::getFactoryPresets();

    for (int i = 0; i < (int) factory.size(); ++i)
    {
        if (factory[(size_t) i].name == name)
        {
            // In Lite the bank entry may exist but not be in the menu - state saved by a Pro
            // instance can name a preset this edition does not list. Fall through to the
            // free-text path rather than selecting an item ID that was never added.
            if (! MomentumEdition::isFactoryPresetVisible (i))
                break;

            presetBox.setSelectedId (i + 1, juce::dontSendNotification);
            return;
        }
    }

    const auto& userFiles = presetManager.getUserPresetFiles();

    for (int i = 0; i < userFiles.size(); ++i)
    {
        if (userFiles.getReference (i).getFileNameWithoutExtension() == name)
        {
            presetBox.setSelectedId (1000 + i, juce::dontSendNotification);
            return;
        }
    }

    // Edited, randomised, or loaded from outside the preset folder.
    presetBox.setSelectedId (0, juce::dontSendNotification);
    presetBox.setText (name, juce::dontSendNotification);
}

void MomentumAudioProcessorEditor::handlePresetSelection (int menuItemID)
{
    if (menuItemID <= 0)
        return;

    if (menuItemID >= 1000)
    {
        const auto& userFiles = presetManager.getUserPresetFiles();
        const int index = menuItemID - 1000;

        if (juce::isPositiveAndBelow (index, userFiles.size()))
            presetManager.loadFromFile (userFiles.getReference (index));
    }
    else
    {
        const int index = menuItemID - 1;

        // Guards the Lite edition's non-selectable "More presets in Pro" row.
        if (! juce::isPositiveAndBelow (index, (int) PresetManager::getFactoryPresets().size()))
            return;

        presetManager.loadFactoryPreset (index);
    }

    snapAllAnimations();
    lastSeenPresetName = processor.getCurrentPresetName();
    repaint();
}

//==============================================================================
void MomentumAudioProcessorEditor::doSavePreset (bool forceAskForName)
{
    const auto existing = presetManager.getCurrentPresetFile();

    if (! forceAskForName && existing.existsAsFile())
    {
        presetManager.saveToFile (existing, existing.getFileNameWithoutExtension());
        refreshPresetMenu();
        syncPresetSelection();
        return;
    }

    const auto startFile = PresetManager::getUserPresetDirectory()
                               .getChildFile (juce::File::createLegalFileName (
                                   processor.getCurrentPresetName()) + PresetManager::getFileExtension());

    fileChooser = std::make_unique<juce::FileChooser> ("Save Momentum preset", startFile,
                                                       PresetManager::getFileWildcard());

    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode
                                | juce::FileBrowserComponent::canSelectFiles
                                | juce::FileBrowserComponent::warnAboutOverwriting,
                              [this] (const juce::FileChooser& chooser)
    {
        auto file = chooser.getResult();

        if (file == juce::File())
            return;

        if (file.getFileExtension().isEmpty())
            file = file.withFileExtension (PresetManager::getFileExtension());

        presetManager.saveToFile (file, file.getFileNameWithoutExtension());
        refreshPresetMenu();
        syncPresetSelection();
    });
}

void MomentumAudioProcessorEditor::doOpenPreset()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Open Momentum preset",
                                                       PresetManager::getUserPresetDirectory(),
                                                       PresetManager::getFileWildcard());

    fileChooser->launchAsync (juce::FileBrowserComponent::openMode
                                | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& chooser)
    {
        const auto file = chooser.getResult();

        if (! file.existsAsFile())
            return;

        if (presetManager.loadFromFile (file))
        {
            snapAllAnimations();
            refreshPresetMenu();
            syncPresetSelection();
            repaint();
        }
    });
}

//==============================================================================
void MomentumAudioProcessorEditor::showOverlay (juce::Component* overlayToShow)
{
    if (visibleOverlay != nullptr && visibleOverlay != overlayToShow)
        visibleOverlay->setVisible (false);

    visibleOverlay = overlayToShow;

    if (visibleOverlay != nullptr)
    {
        visibleOverlay->setVisible (true);
        visibleOverlay->toFront (true);
    }
}

void MomentumAudioProcessorEditor::hideOverlay()
{
    if (visibleOverlay != nullptr)
        visibleOverlay->setVisible (false);

    visibleOverlay = nullptr;
}

//==============================================================================
void MomentumAudioProcessorEditor::switchToSlot (int slot)
{
    slot = juce::jlimit (0, 1, slot);

    if (slot == activeSlot)
        return;

    // Whatever is on screen belongs to the slot we are leaving.
    abSlots[(size_t) activeSlot] = processor.captureParameterValues();
    activeSlot = slot;
    processor.applyParameterValues (abSlots[(size_t) activeSlot]);

    slotAButton.setToggleState (activeSlot == 0, juce::dontSendNotification);
    slotBButton.setToggleState (activeSlot == 1, juce::dontSendNotification);

    snapAllAnimations();
    repaint();
}

void MomentumAudioProcessorEditor::copyActiveSlotToOther()
{
    abSlots[(size_t) activeSlot] = processor.captureParameterValues();
    abSlots[(size_t) (1 - activeSlot)] = abSlots[(size_t) activeSlot];
}

void MomentumAudioProcessorEditor::snapAllAnimations()
{
    for (auto& knob : knobs)
        knob.snapAnimationToValue();

    makeupSlider.snapAnimationToValue();
    repaint();
}
