#include "MainWindow.h"

MainWindow::MainWindow (const juce::String& name)
    : DocumentWindow (name,
                      juce::Colour (0xff1a1a2e),
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (true);
    setContentOwned (new MainComponent(), true);
    setResizable (true, true);
    centreWithSize (1280, 800);
    setVisible (true);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
