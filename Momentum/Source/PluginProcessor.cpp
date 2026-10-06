#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

//==============================================================================
namespace MomentumParams
{
    static const Info table[] =
    {
        { sensitivity, "Sensitivity",  0.0f,  24.0f,    6.0f, 1.0f,  "dB",
          "How much faster-than-average the signal has to get before Momentum reacts. "
          "Low values catch almost every move; high values only catch sudden jumps." },

        { amount,      "Amount",       0.0f,   1.0f,    0.6f, 1.0f,  "%",
          "How much of the excess rate-of-change is turned into gain reduction. "
          "At 100%, 6 dB of excess produces 6 dB of gain reduction." },

        { speed,       "Speed",        1.0f,  50.0f,    8.0f, 0.4f,  "ms",
          "Window of the fast envelope follower. Short windows hear individual transients; "
          "longer windows hear phrases." },

        { recovery,    "Recovery",    20.0f, 1000.0f, 150.0f, 0.4f,  "ms",
          "How quickly gain reduction lets go. Attack is always instant - this is the "
          "only time constant on the gain stage." },

        { makeup,      "Makeup",     -12.0f,  12.0f,    0.0f, 1.0f,  "dB",
          "Output trim applied after compression, to match levels when comparing bypassed "
          "and processed sound." }
    };

    static_assert ((int) juce::numElementsInArray (table) == (int) numParameters,
                   "Parameter table and index enum must agree");

    const Info& get (int index)
    {
        jassert (juce::isPositiveAndBelow (index, (int) numParameters));
        return table[index];
    }

    int indexForID (const juce::String& id)
    {
        for (int i = 0; i < numParameters; ++i)
            if (id == table[i].id)
                return i;

        return -1;
    }
}

//==============================================================================
const juce::Identifier MidiLearnManager::mappingTreeType { "MIDIMAP" };

MidiLearnManager::MidiLearnManager()
{
    for (auto& slot : controllerToParameter)
        slot.store (-1);
}

int MidiLearnManager::getControllerFor (int parameterIndex) const noexcept
{
    for (int cc = 0; cc < 128; ++cc)
        if (controllerToParameter[(size_t) cc].load() == parameterIndex)
            return cc;

    return -1;
}

void MidiLearnManager::clearMapping (int parameterIndex) noexcept
{
    for (auto& slot : controllerToParameter)
        if (slot.load() == parameterIndex)
            slot.store (-1);

    if (learnTarget.load() == parameterIndex)
        learnTarget.store (-1);
}

void MidiLearnManager::clearAllMappings() noexcept
{
    for (auto& slot : controllerToParameter)
        slot.store (-1);

    learnTarget.store (-1);
}

bool MidiLearnManager::processMidi (const juce::MidiBuffer& midi,
                                    std::array<std::atomic<float>, MomentumParams::numParameters>& pendingValues,
                                    std::array<std::atomic<bool>,  MomentumParams::numParameters>& pendingFlags) noexcept
{
    bool somethingChanged = false;

    for (const auto metadata : midi)
    {
        const auto message = metadata.getMessage();

        if (! message.isController())
            continue;

        const int cc = message.getControllerNumber();

        if (! juce::isPositiveAndBelow (cc, 128))
            continue;

        if (const int target = learnTarget.load(); target >= 0)
        {
            // A controller can only drive one parameter, so drop any previous binding.
            for (auto& slot : controllerToParameter)
                if (slot.load() == target)
                    slot.store (-1);

            controllerToParameter[(size_t) cc].store (target);
            learnTarget.store (-1);
            somethingChanged = true;
        }

        const int mapped = controllerToParameter[(size_t) cc].load();

        if (juce::isPositiveAndBelow (mapped, (int) MomentumParams::numParameters))
        {
            pendingValues[(size_t) mapped].store ((float) message.getControllerValue() / 127.0f);
            pendingFlags[(size_t) mapped].store (true);
            somethingChanged = true;
        }
    }

    return somethingChanged;
}

juce::ValueTree MidiLearnManager::toValueTree() const
{
    juce::ValueTree tree (mappingTreeType);

    for (int cc = 0; cc < 128; ++cc)
    {
        const int mapped = controllerToParameter[(size_t) cc].load();

        if (mapped >= 0)
        {
            juce::ValueTree entry ("BINDING");
            entry.setProperty ("cc", cc, nullptr);
            entry.setProperty ("param", juce::String (MomentumParams::get (mapped).id), nullptr);
            tree.addChild (entry, -1, nullptr);
        }
    }

    return tree;
}

void MidiLearnManager::restoreFromValueTree (const juce::ValueTree& tree)
{
    clearAllMappings();

    if (! tree.hasType (mappingTreeType))
        return;

    for (const auto entry : tree)
    {
        const int cc = (int) entry.getProperty ("cc", -1);
        const int index = MomentumParams::indexForID (entry.getProperty ("param").toString());

        if (juce::isPositiveAndBelow (cc, 128) && index >= 0)
            controllerToParameter[(size_t) cc].store (index);
    }
}

//==============================================================================
namespace
{
    juce::String formatDecibels (float value, int places, bool showSign)
    {
        juce::String text (value, places);

        if (showSign && value > 0.0f)
            text = "+" + text;

        return text + " dB";
    }

    float parseLeadingFloat (const juce::String& text)
    {
        return text.retainCharacters ("0123456789.,-+eE").replaceCharacter (',', '.').getFloatValue();
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout MomentumAudioProcessor::createParameterLayout()
{
    using namespace MomentumParams;
    using Attributes = juce::AudioParameterFloatAttributes;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const auto makeRange = [] (const Info& info, float interval)
    {
        return juce::NormalisableRange<float> (info.minValue, info.maxValue, interval, info.skew);
    };

    //-- Sensitivity ------------------------------------------------------------
    {
        const auto& info = get (kSensitivity);
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { info.id, 1 }, info.name, makeRange (info, 0.1f), info.defaultValue,
            Attributes().withLabel (info.unit)
                        .withStringFromValueFunction ([] (float v, int) { return juce::String (v, 1) + " dB"; })
                        .withValueFromStringFunction ([] (const juce::String& t) { return parseLeadingFloat (t); })));
    }

    //-- Amount -----------------------------------------------------------------
    {
        const auto& info = get (kAmount);
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { info.id, 1 }, info.name, makeRange (info, 0.001f), info.defaultValue,
            Attributes().withLabel (info.unit)
                        .withStringFromValueFunction ([] (float v, int)
                                                      { return juce::String (juce::roundToInt (v * 100.0f)) + " %"; })
                        .withValueFromStringFunction ([] (const juce::String& t)
                                                      { return juce::jlimit (0.0f, 1.0f, parseLeadingFloat (t) / 100.0f); })));
    }

    //-- Speed ------------------------------------------------------------------
    {
        const auto& info = get (kSpeed);
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { info.id, 1 }, info.name, makeRange (info, 0.1f), info.defaultValue,
            Attributes().withLabel (info.unit)
                        .withStringFromValueFunction ([] (float v, int)
                                                      { return juce::String (v, v < 10.0f ? 1 : 0) + " ms"; })
                        .withValueFromStringFunction ([] (const juce::String& t) { return parseLeadingFloat (t); })));
    }

    //-- Recovery ---------------------------------------------------------------
    {
        const auto& info = get (kRecovery);
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { info.id, 1 }, info.name, makeRange (info, 1.0f), info.defaultValue,
            Attributes().withLabel (info.unit)
                        .withStringFromValueFunction ([] (float v, int)
                                                      { return juce::String (juce::roundToInt (v)) + " ms"; })
                        .withValueFromStringFunction ([] (const juce::String& t) { return parseLeadingFloat (t); })));
    }

    //-- Makeup -----------------------------------------------------------------
    {
        const auto& info = get (kMakeup);
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { info.id, 1 }, info.name, makeRange (info, 0.1f), info.defaultValue,
            Attributes().withLabel (info.unit)
                        .withStringFromValueFunction ([] (float v, int) { return formatDecibels (v, 1, true); })
                        .withValueFromStringFunction ([] (const juce::String& t) { return parseLeadingFloat (t); })));
    }

    return layout;
}

//==============================================================================
MomentumAudioProcessor::MomentumAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "MOMENTUM", createParameterLayout()),
      random (juce::Random::getSystemRandom().nextInt64())
{
    for (int i = 0; i < MomentumParams::numParameters; ++i)
    {
        const auto id = juce::String (MomentumParams::get (i).id);

        parameterValues[(size_t) i]   = apvts.getRawParameterValue (id);
        rangedParameters[(size_t) i]  = apvts.getParameter (id);

        jassert (parameterValues[(size_t) i]  != nullptr);
        jassert (rangedParameters[(size_t) i] != nullptr);

        pendingMidiValues[(size_t) i].store (0.0f);
        pendingMidiFlags[(size_t) i].store (false);
    }
}

MomentumAudioProcessor::~MomentumAudioProcessor()
{
    cancelPendingUpdate();
}

//==============================================================================
float MomentumAudioProcessor::onePoleCoefficient (double timeMilliseconds, double sampleRate) noexcept
{
    if (timeMilliseconds <= 0.0 || sampleRate <= 0.0)
        return 0.0f;

    return (float) std::exp (-1.0 / ((timeMilliseconds * 0.001) * sampleRate));
}

void MomentumAudioProcessor::updateEnvelopeCoefficients()
{
    const float speedMs    = parameterValues[MomentumParams::kSpeed]->load();
    const float recoveryMs = parameterValues[MomentumParams::kRecovery]->load();

    cachedSpeedMs    = speedMs;
    cachedRecoveryMs = recoveryMs;

    fastCoefficient     = onePoleCoefficient (speedMs,          currentSampleRate);
    slowCoefficient     = onePoleCoefficient (slowEnvelopeMs,   currentSampleRate);
    recoveryCoefficient = onePoleCoefficient (recoveryMs,       currentSampleRate);
}

void MomentumAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;

    fastEnvelope = 0.0f;
    slowEnvelope = 0.0f;
    currentGainReduction = 0.0f;
    gainReductionForMeter.store (0.0f);

    updateEnvelopeCoefficients();

    makeupGain.reset (currentSampleRate, 0.02);
    makeupGain.setCurrentAndTargetValue (
        juce::Decibels::decibelsToGain (parameterValues[MomentumParams::kMakeup]->load()));
}

void MomentumAudioProcessor::releaseResources()
{
}

bool MomentumAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn  = layouts.getMainInputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    return mainIn == mainOut;
}

//==============================================================================
void MomentumAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int numInputChannels  = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples        = buffer.getNumSamples();

    for (int channel = numInputChannels; channel < numOutputChannels; ++channel)
        buffer.clear (channel, 0, numSamples);

    if (midiLearn.processMidi (midiMessages, pendingMidiValues, pendingMidiFlags))
        triggerAsyncUpdate();

    if (numSamples <= 0 || numOutputChannels <= 0)
        return;

    const float sensitivity = parameterValues[MomentumParams::kSensitivity]->load();
    const float amount      = parameterValues[MomentumParams::kAmount]->load();
    const float speedMs     = parameterValues[MomentumParams::kSpeed]->load();
    const float recoveryMs  = parameterValues[MomentumParams::kRecovery]->load();
    const float makeupDb    = parameterValues[MomentumParams::kMakeup]->load();

    // Envelope coefficients are sample-rate aware and only recomputed when a knob actually moves.
    if (speedMs != cachedSpeedMs || recoveryMs != cachedRecoveryMs)
        updateEnvelopeCoefficients();

    makeupGain.setTargetValue (juce::Decibels::decibelsToGain (makeupDb));

    float* const left  = buffer.getWritePointer (0);
    float* const right = numOutputChannels > 1 ? buffer.getWritePointer (1) : nullptr;

    const float fastA = fastCoefficient;
    const float slowA = slowCoefficient;
    const float relA  = recoveryCoefficient;

    float peakGainReduction = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float l = left[i];
        const float r = right != nullptr ? right[i] : l;

        // 1-2. Mono detection signal, squared into two parallel RMS followers.
        const float mono = (l + r) * 0.5f;
        const float squared = mono * mono;

        fastEnvelope = fastA * fastEnvelope + (1.0f - fastA) * squared;
        slowEnvelope = slowA * slowEnvelope + (1.0f - slowA) * squared;

        // 3-4. Delta is how far ahead of the slow follower the fast one has got.
        const float fastDb = 10.0f * std::log10 (fastEnvelope + 1.0e-20f);
        const float slowDb = 10.0f * std::log10 (slowEnvelope + 1.0e-20f);
        const float delta = fastDb - slowDb;

        // 5-6. Only the excess above Sensitivity counts, scaled by Amount.
        const float over = juce::jmax (0.0f, delta - sensitivity);
        const float targetGainReduction = over * amount;

        // 7. Instantaneous attack; Recovery smooths the release only.
        if (targetGainReduction > currentGainReduction)
            currentGainReduction = targetGainReduction;
        else
            currentGainReduction = relA * currentGainReduction + (1.0f - relA) * targetGainReduction;

        // 8-10. Gain reduction to linear, stereo linked, then makeup.
        const float gainLinear = std::pow (10.0f, -currentGainReduction * 0.05f);
        const float gain = gainLinear * makeupGain.getNextValue();

        left[i] = l * gain;

        if (right != nullptr)
            right[i] = r * gain;

        peakGainReduction = juce::jmax (peakGainReduction, currentGainReduction);
    }

    gainReductionForMeter.store (peakGainReduction);
}

//==============================================================================
void MomentumAudioProcessor::handleAsyncUpdate()
{
    // MIDI CC values captured on the audio thread are committed here, on the message
    // thread, so that processBlock never calls into host listener code.
    for (int i = 0; i < MomentumParams::numParameters; ++i)
    {
        if (pendingMidiFlags[(size_t) i].exchange (false))
        {
            const float normalised = pendingMidiValues[(size_t) i].load();

            if (auto* parameter = rangedParameters[(size_t) i])
            {
                parameter->beginChangeGesture();
                parameter->setValueNotifyingHost (normalised);
                parameter->endChangeGesture();
            }
        }
    }

    midiMapRevision.fetch_add (1);
}

//==============================================================================
void MomentumAudioProcessor::resetToDefaults()
{
    for (int i = 0; i < MomentumParams::numParameters; ++i)
    {
        if (auto* parameter = rangedParameters[(size_t) i])
        {
            const auto& info = MomentumParams::get (i);
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (info.defaultValue));
            parameter->endChangeGesture();
        }
    }

    setCurrentPresetName ("Init - Neutral");
}

void MomentumAudioProcessor::randomiseParameters()
{
    // include.md: every press starts from the defaults, so no two presses can compound.
    resetToDefaults();

    // Re-seed so that repeated presses cannot fall into a repeating sequence.
    random.setSeed (juce::Random::getSystemRandom().nextInt64());

    struct RandomRange { int index; float low; float high; };

    // Deliberately narrower than the full ranges - these are the musically useful windows.
    const RandomRange ranges[] =
    {
        { MomentumParams::kSensitivity,  2.0f,  14.0f },
        { MomentumParams::kAmount,       0.25f,  0.95f },
        { MomentumParams::kSpeed,        1.5f,  28.0f },
        { MomentumParams::kRecovery,    40.0f, 600.0f },
        { MomentumParams::kMakeup,      -3.0f,   6.0f }
    };

    for (const auto& entry : ranges)
    {
        if (auto* parameter = rangedParameters[(size_t) entry.index])
        {
            const float value = entry.low + random.nextFloat() * (entry.high - entry.low);
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
            parameter->endChangeGesture();
        }
    }

    setCurrentPresetName ("Random");
}

void MomentumAudioProcessor::applyParameterValues (const std::array<float, MomentumParams::numParameters>& values)
{
    for (int i = 0; i < MomentumParams::numParameters; ++i)
    {
        if (auto* parameter = rangedParameters[(size_t) i])
        {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (values[(size_t) i]));
            parameter->endChangeGesture();
        }
    }
}

std::array<float, MomentumParams::numParameters> MomentumAudioProcessor::captureParameterValues() const
{
    std::array<float, MomentumParams::numParameters> values {};

    for (int i = 0; i < MomentumParams::numParameters; ++i)
        values[(size_t) i] = parameterValues[(size_t) i]->load();

    return values;
}

//==============================================================================
juce::String MomentumAudioProcessor::getCurrentPresetName() const
{
    const juce::ScopedLock lock (presetNameLock);
    return currentPresetName;
}

void MomentumAudioProcessor::setCurrentPresetName (const juce::String& name)
{
    const juce::ScopedLock lock (presetNameLock);
    currentPresetName = name;
}

//==============================================================================
void MomentumAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();

    state.setProperty ("tooltipsEnabled", tooltipsEnabled.load(), nullptr);
    state.setProperty ("presetName", getCurrentPresetName(), nullptr);

    state.removeChild (state.getChildWithName (MidiLearnManager::mappingTreeType), nullptr);
    state.addChild (midiLearn.toValueTree(), -1, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void MomentumAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);

    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return;

    const auto state = juce::ValueTree::fromXml (*xml);

    if (! state.isValid())
        return;

    tooltipsEnabled.store ((bool) state.getProperty ("tooltipsEnabled", true));
    setCurrentPresetName (state.getProperty ("presetName", "Init - Neutral").toString());
    midiLearn.restoreFromValueTree (state.getChildWithName (MidiLearnManager::mappingTreeType));

    apvts.replaceState (state);

    // The restored state can carry new Speed / Recovery values.
    updateEnvelopeCoefficients();
    midiMapRevision.fetch_add (1);
}

//==============================================================================
juce::AudioProcessorEditor* MomentumAudioProcessor::createEditor()
{
    return new MomentumAudioProcessorEditor (*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MomentumAudioProcessor();
}
