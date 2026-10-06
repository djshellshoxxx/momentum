/*
    PluginProcessor.h

    Momentum - a stereo compressor that reacts to the RATE OF CHANGE of input loudness
    rather than to absolute loudness.

    Two RMS envelope followers run in parallel on the mono sum of the input: a FAST one
    whose window is set by Speed, and a SLOW one fixed at 300 ms. Their difference in dB
    (Delta) measures how quickly the signal is getting louder. Whatever part of Delta
    exceeds Sensitivity is multiplied by Amount and applied as gain reduction. Attack is
    instantaneous by design; only the release is smoothed, by Recovery.

    Slow crescendos produce a small Delta and pass through untouched. Sudden jumps produce
    a large Delta and get clamped.
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

#include "MomentumEdition.h"

#include <array>
#include <atomic>

//==============================================================================
/** Parameter identifiers and static metadata, shared by the processor, editor and presets. */
namespace MomentumParams
{
    constexpr const char* sensitivity = "sensitivity";
    constexpr const char* amount      = "amount";
    constexpr const char* speed       = "speed";
    constexpr const char* recovery    = "recovery";
    constexpr const char* makeup      = "makeup";

    /** Index into the fixed parameter table. The order is the GUI order. */
    enum Index
    {
        kSensitivity = 0,
        kAmount,
        kSpeed,
        kRecovery,
        kMakeup,
        numParameters
    };

    struct Info
    {
        const char* id;
        const char* name;
        float       minValue;
        float       maxValue;
        float       defaultValue;
        float       skew;          // 1.0f == linear
        const char* unit;
        const char* tooltip;
    };

    /** The single source of truth for ranges and defaults. */
    const Info& get (int index);

    /** -1 when the id is unknown. */
    int indexForID (const juce::String& id);
}

//==============================================================================
/**
    Lock-free MIDI CC learn and mapping.

    The audio thread only ever touches atomics here; the actual parameter writes are
    handed to the message thread so that processBlock stays allocation free.
*/
class MidiLearnManager
{
public:
    MidiLearnManager();

    /** Pass MomentumParams::numParameters to cancel learning. */
    void setLearnTarget (int parameterIndex) noexcept   { learnTarget.store (parameterIndex); }
    int  getLearnTarget() const noexcept                { return learnTarget.load(); }
    bool isLearning() const noexcept                    { return learnTarget.load() >= 0; }

    /** -1 when the parameter has no CC bound to it. */
    int  getControllerFor (int parameterIndex) const noexcept;
    void clearMapping (int parameterIndex) noexcept;
    void clearAllMappings() noexcept;

    /** Audio thread. Returns true when something changed and the editor should refresh. */
    bool processMidi (const juce::MidiBuffer& midi,
                      std::array<std::atomic<float>, MomentumParams::numParameters>& pendingValues,
                      std::array<std::atomic<bool>,  MomentumParams::numParameters>& pendingFlags) noexcept;

    juce::ValueTree toValueTree() const;
    void restoreFromValueTree (const juce::ValueTree&);

    static const juce::Identifier mappingTreeType;

private:
    std::atomic<int> learnTarget { -1 };
    std::array<std::atomic<int>, 128> controllerToParameter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiLearnManager)
};

//==============================================================================
class MomentumAudioProcessor : public juce::AudioProcessor,
                               private juce::AsyncUpdater
{
public:
    MomentumAudioProcessor();
    ~MomentumAudioProcessor() override;

    //== AudioProcessor =========================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                                 { return true; }

    const juce::String getName() const override                     { return JucePlugin_Name; }
    bool acceptsMidi() const override                               { return true; }
    bool producesMidi() const override                              { return false; }
    bool isMidiEffect() const override                              { return false; }
    double getTailLengthSeconds() const override                    { return 0.0; }

    int getNumPrograms() override                                   { return 1; }
    int getCurrentProgram() override                                { return 0; }
    void setCurrentProgram (int) override                           {}
    const juce::String getProgramName (int) override                { return {}; }
    void changeProgramName (int, const juce::String&) override      {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int sizeInBytes) override;

    //== Momentum ===============================================================
    juce::AudioProcessorValueTreeState& getAPVTS() noexcept         { return apvts; }
    MidiLearnManager& getMidiLearn() noexcept                       { return midiLearn; }

    /** Peak gain reduction in dB for the block just processed. Meter ballistics live in the editor. */
    float getCurrentGainReductionDb() const noexcept                { return gainReductionForMeter.load(); }

    /** Puts every parameter back to the value in the parameter table. */
    void resetToDefaults();

    /** Resets first, then picks a fresh musically useful value for every parameter. */
    void randomiseParameters();

    /** Applies a full set of normalised-to-real parameter values, e.g. from a preset. */
    void applyParameterValues (const std::array<float, MomentumParams::numParameters>& values);
    std::array<float, MomentumParams::numParameters> captureParameterValues() const;

    /** Non-automated UI preference, persisted with the plugin state. */
    bool getTooltipsEnabled() const noexcept                        { return tooltipsEnabled.load(); }
    void setTooltipsEnabled (bool shouldBeEnabled) noexcept         { tooltipsEnabled.store (shouldBeEnabled); }

    /** Name shown in the preset selector; persisted with the plugin state. */
    juce::String getCurrentPresetName() const;
    void setCurrentPresetName (const juce::String&);

    /** Bumped whenever a MIDI mapping is created on the audio thread, so the editor can repaint. */
    int getMidiMapRevision() const noexcept                         { return midiMapRevision.load(); }

    static juce::String getVersionString()                          { return JucePlugin_VersionString; }
    static juce::String getEditionName()                            { return MomentumEdition::name; }

private:
    //== Helpers ================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateEnvelopeCoefficients();
    static float onePoleCoefficient (double timeMilliseconds, double sampleRate) noexcept;

    void handleAsyncUpdate() override;

    //== State ==================================================================
    juce::AudioProcessorValueTreeState apvts;
    MidiLearnManager midiLearn;

    std::array<std::atomic<float>*, MomentumParams::numParameters> parameterValues { {} };
    std::array<juce::RangedAudioParameter*, MomentumParams::numParameters> rangedParameters { {} };

    //== DSP ====================================================================
    double currentSampleRate { 44100.0 };

    float fastEnvelope { 0.0f };
    float slowEnvelope { 0.0f };
    float currentGainReduction { 0.0f };

    float fastCoefficient     { 0.0f };
    float slowCoefficient     { 0.0f };
    float recoveryCoefficient { 0.0f };

    float cachedSpeedMs    { -1.0f };
    float cachedRecoveryMs { -1.0f };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> makeupGain;

    /** The slow follower is fixed, per the design. */
    static constexpr double slowEnvelopeMs = 300.0;

    //== Cross-thread ===========================================================
    std::atomic<float> gainReductionForMeter { 0.0f };
    std::atomic<bool>  tooltipsEnabled { true };
    std::atomic<int>   midiMapRevision { 0 };

    std::array<std::atomic<float>, MomentumParams::numParameters> pendingMidiValues;
    std::array<std::atomic<bool>,  MomentumParams::numParameters> pendingMidiFlags;

    juce::CriticalSection presetNameLock;
    juce::String currentPresetName { "Init - Neutral" };

    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MomentumAudioProcessor)
};
