cmake_minimum_required(VERSION 3.22)

project(man VERSION 0.1.0)

add_subdirectory(JUCE)

juce_add_gui_app(man
    PRODUCT_NAME "man."
    COMPANY_NAME "man"
    BUNDLE_ID "com.man.app"
    NEEDS_CURL FALSE
    NEEDS_WEB_BROWSER FALSE)

juce_generate_juce_header(man)

target_sources(man
    PRIVATE
        Source/Main.cpp
        Source/MainWindow.cpp
        Source/MainComponent.cpp
        Source/Model/Element.cpp
        Source/Model/ElementLibrary.cpp
        Source/Model/Recipe.cpp
        Source/Model/RecipeBook.cpp
        Source/Model/DiscoveryLog.cpp
        Source/Model/Sketch.cpp
        Source/Model/Clip.cpp
        Source/Audio/AudioEngine.cpp
        Source/Audio/SampleManager.cpp
        Source/Audio/SampleVoice.cpp
        Source/Audio/SampleSound.cpp
        Source/Visuals/VisualRenderer.cpp
        Source/Visuals/WaveformVisual.cpp
        Source/Visuals/GeometricVisual.cpp
        Source/Visuals/ParticleVisual.cpp
        Source/Visuals/CompositeVisual.cpp
        Source/Visuals/VisualCanvas.cpp
        Source/UI/Discovery/DiscoveryView.cpp
        Source/UI/Discovery/ElementTile.cpp
        Source/UI/Discovery/VisualPanel.cpp
        Source/UI/Discovery/SoundPanel.cpp
        Source/UI/Discovery/DiscoveryPanel.cpp
        Source/UI/Discovery/CombineZone.cpp
        Source/UI/Discovery/DiscoveryAnimation.cpp
        Source/UI/Arrangement/ArrangementView.cpp
        Source/UI/Arrangement/Timeline.cpp
        Source/UI/Arrangement/ClipComponent.cpp
        Source/UI/Arrangement/TrackLane.cpp
        Source/UI/Arrangement/TransportBar.cpp
        Source/UI/Arrangement/ElementPalette.cpp
        Source/UI/Live/LiveView.cpp
        Source/UI/Live/TriggerPad.cpp
        Source/UI/Live/TriggerGrid.cpp
        Source/UI/Common/ModeSelector.cpp
        Source/UI/Common/VisualPreview.cpp
        Source/Visuals/ImageVisual.cpp
        Source/UI/Browser/BrowserSidebar.cpp
        Source/UI/Browser/SoundBrowserPanel.cpp
        Source/UI/Browser/VisualBrowserPanel.cpp
        Source/UI/Browser/VisualParamEditor.cpp
        Source/Export/AudioExporter.cpp
        Source/Export/VideoExporter.cpp)

target_compile_definitions(man
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:man,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:man,JUCE_VERSION>")

target_include_directories(man
    PRIVATE
        Source)

target_link_libraries(man
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

juce_add_binary_data(ManResources
    SOURCES
        Resources/Recipes/default_recipes.json)

target_link_libraries(man PRIVATE ManResources)
