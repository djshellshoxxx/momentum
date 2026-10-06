#include "PresetManager.h"

//==============================================================================
const juce::Identifier PresetManager::presetTreeType { "MomentumPreset" };

//==============================================================================
const std::vector<MomentumPreset>& PresetManager::getFactoryPresets()
{
    // Order: Sensitivity (dB), Amount (0-1), Speed (ms), Recovery (ms), Makeup (dB).
    static const std::vector<MomentumPreset> presets
    {
        { "Init - Neutral",                "Every control at its default. The starting point.",
          {  6.0f, 0.60f,   8.0f, 150.0f,  0.0f } },

        { "Vocal - Shout Tamer",           "Catches sudden shouted syllables and leaves the rest of the take alone.",
          {  4.5f, 0.85f,   6.0f, 120.0f,  1.5f } },

        { "Vocal - Gentle Ride",           "Light, slow-handed levelling for an already well-sung take.",
          {  8.0f, 0.35f,  14.0f, 320.0f,  1.0f } },

        { "Vocal - Consonant Control",     "Very fast window aimed at hard consonants and plosive edges.",
          {  3.0f, 0.70f,   2.0f,  90.0f,  2.0f } },

        { "Drums - Spike Control",         "Flattens stray hard hits without pumping the kit between them.",
          {  5.0f, 0.75f,   3.0f,  80.0f,  2.0f } },

        { "Drums - Room Glue",             "Slower window so the room tail keeps moving while spikes get held.",
          {  7.0f, 0.55f,  20.0f, 400.0f,  1.5f } },

        { "Drums - Snare Crack Limiter",   "Aggressive and short, for one loud snare in an otherwise even part.",
          {  2.5f, 0.95f,   1.5f,  45.0f,  3.0f } },

        { "Bass - Pick Attack Tamer",      "Takes the edge off pick or slap attacks, keeps the note body intact.",
          {  6.0f, 0.65f,   4.0f, 180.0f,  1.0f } },

        { "Guitar - Strum Smoother",       "Evens out heavy-handed strums across an acoustic part.",
          {  7.5f, 0.50f,  12.0f, 260.0f,  1.0f } },

        { "Piano - Accent Leveller",       "Holds sforzando accents down without touching the phrasing.",
          {  9.0f, 0.45f,  10.0f, 300.0f,  0.5f } },

        { "Orchestra - Let Swells Breathe","High threshold and long recovery, so crescendos pass untouched.",
          { 14.0f, 0.40f,  30.0f, 700.0f,  0.0f } },

        { "Strings - Bow Bite Control",    "Catches the bite at the start of an aggressive bow stroke.",
          { 10.0f, 0.50f,   7.0f, 260.0f,  0.5f } },

        { "Mix Bus - Transparent Safety",  "A quiet safety net. Only unusually sudden events get held.",
          { 12.0f, 0.30f,  16.0f, 350.0f,  0.0f } },

        { "Mastering - Surprise Catcher",  "Barely audible until something genuinely unexpected happens.",
          { 16.0f, 0.25f,  22.0f, 500.0f,  0.0f } },

        { "Podcast - Plosive Clamp",       "Speech-tuned. Clamps pops and desk knocks, leaves speech dynamics.",
          {  4.0f, 0.80f,   5.0f, 140.0f,  3.0f } },

        { "Live - Feedback Guard",         "Fast and firm, to hold a runaway before it becomes a problem.",
          {  3.5f, 0.90f,   2.5f,  60.0f,  0.0f } },

        { "Synth - Pluck Softener",        "Rounds off hard synth plucks while the sustain stays put.",
          {  5.5f, 0.60f,   3.5f, 220.0f,  1.5f } },

        { "FX - Pumping Sweep",            "Deliberately overdone. Long recovery turns transients into pumping.",
          {  1.0f, 1.00f,   1.0f, 900.0f,  4.0f } },

        { "FX - Brick Derivative",         "Extreme. Almost any upward move is flattened immediately.",
          {  0.0f, 1.00f,   1.0f,  20.0f,  6.0f } },

        { "Broadcast - Level Guard",       "Conservative settings for unattended broadcast or streaming chains.",
          { 11.0f, 0.55f,  18.0f, 280.0f,  2.0f } }
    };

    // MomentumEdition::liteFactoryPresets holds indices into this bank. Reordering the bank
    // would silently hand the Lite edition a different six presets, so check the indices
    // still point at the names they were chosen for. Debug-only; jassert compiles out.
    // Deliberately not behind #if JUCE_DEBUG: guarding it that way would mean release
    // builds never compile it, and an uncompiled check is not a check. jassert itself
    // compiles out in release, so this costs one pass over six ints, once, ever.
    static const bool liteSelectionChecked = []
    {
        for (size_t i = 0; i < std::size (MomentumEdition::liteFactoryPresets); ++i)
        {
            const int index = MomentumEdition::liteFactoryPresets[i];
            const bool inRange = juce::isPositiveAndBelow (index, (int) presets.size());
            const bool nameMatches = inRange
                                      && presets[(size_t) index].name
                                           == MomentumEdition::liteFactoryPresetNames[i];

            jassert (inRange);
            jassert (nameMatches);
            juce::ignoreUnused (inRange, nameMatches);
        }

        return true;
    }();

    juce::ignoreUnused (liteSelectionChecked);

    return presets;
}

//==============================================================================
PresetManager::PresetManager (MomentumAudioProcessor& p)
    : processor (p)
{
    refreshUserPresets();
}

juce::File PresetManager::getUserPresetDirectory()
{
    auto directory = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                         .getChildFile ("Sheldon Davidson")
                         .getChildFile ("Momentum")
                         .getChildFile ("Presets");

    if (! directory.exists())
        directory.createDirectory();

    return directory;
}

void PresetManager::refreshUserPresets()
{
    userPresetFiles.clear();

    const auto directory = getUserPresetDirectory();

    if (directory.isDirectory())
        directory.findChildFiles (userPresetFiles, juce::File::findFiles, false, getFileWildcard());

    // findChildFiles gives no useful order, so sort by name for a stable menu.
    struct ByName
    {
        static int compareElements (const juce::File& a, const juce::File& b)
        {
            return a.getFileNameWithoutExtension()
                    .compareIgnoreCase (b.getFileNameWithoutExtension());
        }
    };

    ByName comparator;
    userPresetFiles.sort (comparator);
}

//==============================================================================
void PresetManager::loadFactoryPreset (int index)
{
    const auto& presets = getFactoryPresets();

    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return;

    const auto& preset = presets[(size_t) index];

    processor.applyParameterValues (preset.values);
    processor.setCurrentPresetName (preset.name);
    currentPresetFile = juce::File();
}

bool PresetManager::loadFromFile (const juce::File& file)
{
    if (! file.existsAsFile())
        return false;

    auto xml = juce::XmlDocument::parse (file);

    if (xml == nullptr || ! xml->hasTagName (presetTreeType.toString()))
        return false;

    const auto tree = juce::ValueTree::fromXml (*xml);

    if (! tree.isValid())
        return false;

    auto values = processor.captureParameterValues();

    for (int i = 0; i < MomentumParams::numParameters; ++i)
    {
        const auto& info = MomentumParams::get (i);

        if (tree.hasProperty (info.id))
            values[(size_t) i] = juce::jlimit (info.minValue, info.maxValue,
                                               (float) (double) tree.getProperty (info.id));
    }

    processor.applyParameterValues (values);
    processor.setCurrentPresetName (tree.getProperty ("name", file.getFileNameWithoutExtension()).toString());
    currentPresetFile = file;

    return true;
}

bool PresetManager::saveToFile (const juce::File& file, const juce::String& presetName)
{
    const auto values = processor.captureParameterValues();

    juce::ValueTree tree (presetTreeType);
    tree.setProperty ("name", presetName, nullptr);
    tree.setProperty ("pluginVersion", MomentumAudioProcessor::getVersionString(), nullptr);

    for (int i = 0; i < MomentumParams::numParameters; ++i)
        tree.setProperty (MomentumParams::get (i).id, values[(size_t) i], nullptr);

    auto xml = tree.createXml();

    if (xml == nullptr)
        return false;

    if (! file.getParentDirectory().exists())
        file.getParentDirectory().createDirectory();

    if (! xml->writeTo (file))
        return false;

    processor.setCurrentPresetName (presetName);
    currentPresetFile = file;
    refreshUserPresets();

    return true;
}
