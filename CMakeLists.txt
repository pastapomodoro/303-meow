cmake_minimum_required(VERSION 3.22)
project(TB303Clone VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.6
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(TB303Clone
    COMPANY_NAME            "YourCompany"
    IS_SYNTH                TRUE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE             Tb03
    FORMATS                 VST3 Standalone
    PRODUCT_NAME            "303 Meow"
    VERSION                 "1.0.0"
)

if(APPLE)
    juce_add_plugin(TB303Clone_AU
        COMPANY_NAME            "YourCompany"
        IS_SYNTH                TRUE
        NEEDS_MIDI_INPUT        TRUE
        NEEDS_MIDI_OUTPUT       FALSE
        IS_MIDI_EFFECT          FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PLUGIN_MANUFACTURER_CODE Manu
        PLUGIN_CODE             Tb04
        FORMATS                 AU
        PRODUCT_NAME            "303 Meow"
        VERSION                 "1.0.0"
    )
endif()

set(SOURCE_FILES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Synth/VCO.cpp
    Source/Synth/VCF.cpp
    Source/Synth/VCA.cpp
    Source/Synth/Envelope.cpp
    Source/Synth/TB303Engine.cpp
    Source/Sequencer/StepSequencer.cpp
    Source/Sequencer/Pattern.cpp
    Source/UI/SequencerGrid.cpp
)

target_sources(TB303Clone PRIVATE ${SOURCE_FILES})

juce_generate_juce_header(TB303Clone)

target_compile_definitions(TB303Clone
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(TB303Clone
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_devices
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(APPLE)
    juce_generate_juce_header(TB303Clone_AU)
    target_sources(TB303Clone_AU PRIVATE ${SOURCE_FILES})
    target_compile_definitions(TB303Clone_AU
        PUBLIC
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
    )
    target_link_libraries(TB303Clone_AU
        PRIVATE
            juce::juce_audio_utils
            juce::juce_audio_processors
            juce::juce_audio_devices
            juce::juce_dsp
            juce::juce_gui_basics
            juce::juce_gui_extra
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )
endif()
