#include "SyncSink.h"
#include "SyncSinkCanvas.h"
#include "SyncSinkEditor.h"
#include <string>

using namespace SyncSinkSpace;

SyncSinkEditor::SyncSinkEditor(SyncSink* parentNode, bool useDefaultParameterEditors = true)
   : VisualizerEditor(parentNode, useDefaultParameterEditors)
{
   processor = parentNode;
   processor->setEditor(this);

   tabText = "Sync Sink";
   int silksize;
   const char* silk = CoreServices::getApplicationResource("silkscreenserialized", silksize);
   MemoryInputStream mis(silk, silksize, false);
   Typeface::Ptr typeface = new CustomTypeface(mis);
   font = Font(typeface);

   setDesiredWidth(300);

    addPlotButton = new UtilityButton("Add Plot", Font("Default", 20, Font::plain));
    addPlotButton->addListener(this);
    addPlotButton->setBounds(10, 60, 100, 20);
    addAndMakeVisible(addPlotButton);

    addPlotParamsLabel = new Label("Parameters", "0,0,0");
    addPlotParamsLabel->setEditable(true);
    addPlotParamsLabel->setBounds(10, 30, 100, 20);
    addAndMakeVisible(addPlotParamsLabel);

    stimClassLegendLabel = new Label("Stimuli Classes", "Stimuli Classes");
    stimClassLegendLabel->setEditable(false);
    stimClassLegendLabel->setBounds(120, 30, 100, 100);
    addAndMakeVisible(stimClassLegendLabel);

}

SyncSinkEditor::~SyncSinkEditor(){}

void SyncSinkEditor::updateSettings(){}

void SyncSinkEditor::buttonEvent(Button* button)
{
    if (button == addPlotButton)
    {
        StringArray tokens;
        tokens.addTokens(addPlotParamsLabel->getText(), ",", "");
        /* tokens[0] == channel_idx; tokens[1] == sorted_id; tokens[2] == stim_class*/
        if (tokens.size() == 3)
        {
            processor->addPSTHPlot(
                tokens[0].getIntValue(),
                tokens[1].getIntValue(),
                std::vector<int>(1, tokens[2].getIntValue())
            );
        }
        else if (tokens.size() == 2)
        {
            processor->addPSTHPlot(
                tokens[0].getIntValue(),
                tokens[1].getIntValue(),
                processor->getStimClasses()
            );
        }
        else 
        {
            std::cout << "unable to parse plot param string " << addPlotParamsLabel->getText() << std::endl;
        }

    }
}

void SyncSinkEditor::updateLegend()
{
    String legendText = "";
    for (int i = 0; i < processor->numConditions; i++)
    {
        legendText += String(i) + String(": ") + processor->getStimClassLabel(i) + String("\n");
    }
    std::cout << legendText << std::endl;
    stimClassLegendLabel->setText(legendText, juce::dontSendNotification);
}


Visualizer* SyncSinkEditor::createNewCanvas()
{
    SyncSink* processor = (SyncSink*) getProcessor();
    syncSinkCanvas =  new SyncSinkCanvas(processor);
    return syncSinkCanvas;
}
