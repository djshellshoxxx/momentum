cmake_minimum_required(VERSION 3.22)

# CMake 4.x refuses projects that declare an old policy minimum. JUCE 7.0.9 and some of
# its helper sub-projects still do, so relax that before anything is fetched.
if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.0)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum CMake policy version" FORCE)
endif()

project(Momentum VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#==============================================================================
# Edition
#
# include.md: "this build may become the pro version, and a free version might be made
# in future builds". Pro is the default, so a plain build is unchanged.
#
#   cmake -B build-lite -G "Visual Studio 17 2022" -A x64 -DMOMENTUM_EDITION=Lite
#
# Pro and Lite get different plugin codes, bundle IDs and product names so both can sit
# in C:\Program Files\Common Files\VST3 at once without clobbering each other.
#==============================================================================
set(MOMENTUM_EDITION "Pro" CACHE STRING "Momentum edition to build: Pro or Lite")
set_property(CACHE MOMENTUM_EDITION PROPERTY STRINGS Pro Lite)

if(MOMENTUM_EDITION STREQUAL "Pro")
    set(MOMENTUM_EDITION_PRO   1)
    set(MOMENTUM_PRODUCT_NAME  "Momentum")
    set(MOMENTUM_PLUGIN_CODE   Mmnt)
    set(MOMENTUM_BUNDLE_ID     com.sheldondavidson.momentum)
elseif(MOMENTUM_EDITION STREQUAL "Lite")
    set(MOMENTUM_EDITION_PRO   0)
    set(MOMENTUM_PRODUCT_NAME  "Momentum Lite")
    set(MOMENTUM_PLUGIN_CODE   MmnL)
    set(MOMENTUM_BUNDLE_ID     com.sheldondavidson.momentumlite)
else()
    message(FATAL_ERROR
        "MOMENTUM_EDITION must be either Pro or Lite, but was '${MOMENTUM_EDITION}'.")
endif()

message(STATUS "Momentum: building the ${MOMENTUM_EDITION} edition (${MOMENTUM_PRODUCT_NAME})")

#==============================================================================
# JUCE 7.0.9
#==============================================================================
include(FetchContent)

FetchContent_Declare(JUCE
    GIT_REPOSITORY  https://github.com/juce-framework/JUCE.git
    GIT_TAG         7.0.9
    GIT_SHALLOW     TRUE
    GIT_PROGRESS    TRUE)

FetchContent_MakeAvailable(JUCE)

# CLAP support comes from free-audio/clap-juce-extensions, a thin wrapper over the same
# JUCE processor. On by default; -DMOMENTUM_BUILD_CLAP=OFF skips it.
option(MOMENTUM_BUILD_CLAP "Also build a CLAP plug-in" ON)

if(MOMENTUM_BUILD_CLAP)
    FetchContent_Declare(clap-juce-extensions
        GIT_REPOSITORY  https://github.com/free-audio/clap-juce-extensions.git
        GIT_TAG         main
        GIT_SHALLOW     TRUE)
    FetchContent_MakeAvailable(clap-juce-extensions)
endif()

# Copying into the system plug-in folders is only wanted on a developer's own machine.
option(MOMENTUM_COPY_AFTER_BUILD "Install built plug-ins into the system plug-in folders" ON)

#==============================================================================
# Plugin target
#==============================================================================
juce_add_plugin(Momentum
    PRODUCT_NAME                "${MOMENTUM_PRODUCT_NAME}"
    COMPANY_NAME                "Sheldon Davidson"
    BUNDLE_ID                   ${MOMENTUM_BUNDLE_ID}
    PLUGIN_MANUFACTURER_CODE    ShDv
    PLUGIN_CODE                 ${MOMENTUM_PLUGIN_CODE}
    FORMATS                     VST3 Standalone
    VST3_CATEGORIES             Fx Dynamics
    IS_SYNTH                    FALSE
    # start.md specifies NEEDS_MIDI_INPUT FALSE. include.md requires right-click MIDI
    # mapping on every control, and VST3 only exposes IMidiMapping when the plugin has an
    # event input bus, so this is TRUE. It adds an empty MIDI bus and nothing else; the
    # audio path is unchanged. See README.md.
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     ${MOMENTUM_COPY_AFTER_BUILD}
    ICON_BIG                    "${CMAKE_CURRENT_SOURCE_DIR}/Resources/MomentumIcon.png"
    ICON_SMALL                  "${CMAKE_CURRENT_SOURCE_DIR}/Resources/MomentumIcon.png")

if(MOMENTUM_BUILD_CLAP)
    clap_juce_extensions_plugin(TARGET Momentum
        CLAP_ID         "${MOMENTUM_BUNDLE_ID}"
        CLAP_FEATURES   audio-effect compressor)
endif()

target_sources(Momentum PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/MomentumLookAndFeel.cpp
    Source/MomentumControls.cpp
    Source/PresetManager.cpp
    Source/HelpComponent.cpp
    Source/OptionsComponent.cpp)

target_compile_definitions(Momentum PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_MODAL_LOOPS_PERMITTED=0
    MOMENTUM_EDITION_PRO=${MOMENTUM_EDITION_PRO})

# Link-time optimisation is the peak-memory moment of a JUCE build. On a small machine
# (4 GB, or anything sharing the box with another build) it is the step most likely to be
# killed by the OS. This switch drops it so the project can still be built and tested there.
# OFF by default, so release builds are unchanged.
#
#   cmake -B build-lite ... -DMOMENTUM_DISABLE_LTO=ON
#
# Do not ship a binary built this way - it is a development escape hatch, not a release
# configuration.
option(MOMENTUM_DISABLE_LTO
       "Drop link-time optimisation to lower peak build memory. Development use only." OFF)

set(MOMENTUM_LTO_FLAGS juce::juce_recommended_lto_flags)

if(MOMENTUM_DISABLE_LTO)
    set(MOMENTUM_LTO_FLAGS "")
    message(STATUS "Momentum: LTO disabled - lower build memory, NOT a release configuration")
endif()

target_link_libraries(Momentum
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        ${MOMENTUM_LTO_FLAGS}
        juce::juce_recommended_warning_flags)
