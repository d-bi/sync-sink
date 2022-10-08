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

    addPlotParamsLabel = new Label("Plot Parameters", "0,0");
    addPlotParamsLabel->setEditable(true);
    addPlotParamsLabel->setBounds(10, 30, 100, 20);
    addPlotParamsLabel->setFont(Font("Default", 20, Font::plain));
    addAndMakeVisible(addPlotParamsLabel);

    addPlotButton = new UtilityButton("Add Plot", Font("Default", 20, Font::plain));
    addPlotButton->addListener(this);
    addPlotButton->setBounds(110, 30, 80, 20);
    addAndMakeVisible(addPlotButton);

    stimClassLegendLabel = new Label("Trial Number", "Trial #: NA");
    stimClassLegendLabel->setEditable(false);
    stimClassLegendLabel->setBounds(10, 60, 100, 20);
    stimClassLegendLabel->setFont(Font("Default", 16, Font::plain));
    addAndMakeVisible(stimClassLegendLabel);

    resetTensorButton = new UtilityButton("Reset PSTHs", Font("Default", 20, Font::plain));
    resetTensorButton->addListener(this);
    resetTensorButton->setBounds(110, 60, 80, 20);
    addAndMakeVisible(resetTensorButton);

    binParamsLabel = new Label("Bin Parameters", "50,10");
    binParamsLabel->setEditable(true);
    binParamsLabel->setBounds(10, 90, 100, 20);
    binParamsLabel->setFont(Font("Default", 20, Font::plain));
    addAndMakeVisible(binParamsLabel);

    rebinButton = new UtilityButton("Set Bins", Font("Default", 20, Font::plain));
    rebinButton->addListener(this);
    rebinButton->setBounds(110, 90, 80, 20);
    addAndMakeVisible(rebinButton);

    resetButton = new UtilityButton("Reset Vars", Font("Default", 20, Font::plain));
    resetButton->addListener(this);
    resetButton->setBounds(200, 90, 80, 20);
    addAndMakeVisible(resetButton);
}

SyncSinkEditor::~SyncSinkEditor(){}

void SyncSinkEditor::updateSettings(){}

void SyncSinkEditor::buttonEvent(Button* button)
{
    if (button == addPlotButton)
    {
        StringArray tokens;
        tokens.addTokens(addPlotParamsLabel->getText(), ",", "");
        /* tokens[0] == channel_idx; tokens[1] == sorted_id; tokens[2] == stim_class */
        if (tokens.size() == 3)
        {
            if (tokens[2].getIntValue() >= processor->numConditions)
            {
                std::cout << "stim class specified out of bounds" << std::endl;
                return;
            }
            processor->addPSTHPlot(
                tokens[0].getIntValue(),
                tokens[1].getIntValue(),
                std::vector<int>(1, tokens[2].getIntValue())
            );
        }
        else if (tokens.size() == 2)
        {
            if (processor->numConditions == 0)
            {
                std::cout << "empty stim class list" << std::endl;
                return;
            }
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
    else if (button == resetTensorButton)
    {
        processor->resetTensor();
    }
    else if (button == rebinButton)
    {
        StringArray tokens;
        tokens.addTokens(binParamsLabel->getText(), ",", "");
        /* tokens[0] == n_bins; tokens[1] == bin_size */
        if (tokens.size() == 2)
        {
            processor->rebin(
                tokens[0].getIntValue(),
                tokens[1].getIntValue()
            );
            processor->resetTensor();
        }
        else
        {
            std::cout << "unable to parse bin param string " << binParamsLabel->getText() << std::endl;
        }
    }
    else if (button == resetButton)
    {
        processor->clearVars();
        if (processor->canvas != nullptr)
        {
            // (processor->canvas)->update();
        }
    }
}

void SyncSinkEditor::updateLegend()
{
    //String legendText = "";
    //for (int i = 0; i < processor->numConditions; i++)
    //{
    //    legendText += String(i) + String(": ") + processor->getStimClassLabel(i) + String("\n");
    //}
    //std::cout << legendText << std::endl;
    String legendText = "Trial #: " + String(processor->getNTrial());
    stimClassLegendLabel->setText(legendText, juce::dontSendNotification);
}


Visualizer* SyncSinkEditor::createNewCanvas()
{
    SyncSink* processor = (SyncSink*) getProcessor();
    syncSinkCanvas =  new SyncSinkCanvas(processor);
    return syncSinkCanvas;
}
