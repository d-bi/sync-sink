#include "SyncSink.h"
#include "SyncSinkCanvas.h"
#include "SyncSinkEditor.h"
#include <string>

using namespace SyncSinkSpace;

SyncSinkEditor::SyncSinkEditor(SyncSink* parentNode, bool useDefaultParameterEditors = true)
   : VisualizerEditor(parentNode, useDefaultParameterEditors)
{
   processor = parentNode;

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

    addPlotParams = new Label("Parameters", "0,0,0");
    addPlotParams->setEditable(true);
    addPlotParams->setBounds(10, 30, 100, 20);
    addAndMakeVisible(addPlotParams);

}

SyncSinkEditor::~SyncSinkEditor(){}

void SyncSinkEditor::updateSettings(){}

void SyncSinkEditor::buttonEvent(Button* button)
{
    if (button == addPlotButton)
    {
        StringArray tokens;
        tokens.addTokens(addPlotParams->getText(), ",", "");
        /* tokens[0] == channel_idx; tokens[1] == sorted_id; tokens[2] == stim_class*/
        if (tokens.size() != 3)
        {
            std::cout << "unable to parse plot param string " << addPlotParams->getText() << std::endl;
        }
        processor->addPSTHPlot(
            tokens[0].getIntValue(),
            tokens[1].getIntValue(),
            tokens[2].getIntValue());
    }
}

Visualizer* SyncSinkEditor::createNewCanvas()
{
    SyncSink* processor = (SyncSink*) getProcessor();
    syncSinkCanvas =  new SyncSinkCanvas(processor);
    return syncSinkCanvas;
}
