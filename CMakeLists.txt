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

find_package(Python3 3.8 COMPONENTS Interpreter QUIET)

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
    PRODUCT_NAME            "AcidLab"
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
        PRODUCT_NAME            "AcidLab"
        VERSION                 "1.0.0"
    )
endif()

juce_add_binary_data(TB303Assets
    SOURCES
        Resources/ui.html
        acidlab-knob-61f-128px.png
)

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
)

target_sources(TB303Clone PRIVATE ${SOURCE_FILES})

juce_generate_juce_header(TB303Clone)

target_compile_definitions(TB303Clone
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(TB303Clone
    PRIVATE
        TB303Assets
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
            JUCE_WEB_BROWSER=1
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
    )
    target_link_libraries(TB303Clone_AU
        PRIVATE
            TB303Assets
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

# Steinberg moduleinfotool can emit trailing commas; strict hosts reject invalid JSON.
if(Python3_FOUND)
    add_custom_command(
        TARGET TB303Clone_VST3 POST_BUILD
        COMMAND ${Python3_EXECUTABLE}
            "${CMAKE_CURRENT_SOURCE_DIR}/fix_moduleinfo_json.py"
            "$<TARGET_FILE_DIR:TB303Clone_VST3>/../Resources/moduleinfo.json"
        # Editing moduleinfo invalidates the bundle seal from the link step.
        COMMAND codesign --force --sign - --deep --timestamp=none
            "$<TARGET_FILE_DIR:TB303Clone_VST3>/../.."
        COMMENT "Normalize VST3 moduleinfo.json and re-seal bundle"
        VERBATIM)
endif()
