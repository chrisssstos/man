#include <JuceHeader.h>
#include "MainWindow.h"

class ManApplication : public juce::JUCEApplication
{
public:
    ManApplication() {}

    const juce::String getApplicationName() override    { return "man."; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override          { return false; }

    void initialise (const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow> (getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (ManApplication)
