#ifndef SYNCSINKEDITOR_H_DEFINED
#define SYNCSINKEDITOR_H_DEFINED

#include <VisualizerEditorHeaders.h>
#include "SyncSink.h"

class SyncSinkCanvas;

namespace SyncSinkSpace
{
    class SyncSink;

    class SyncSinkEditor : public VisualizerEditor
    {
        public:
            SyncSinkEditor(SyncSink* parentNode, bool useDefaultParameterEditors);
            ~SyncSinkEditor();
            void updateSettings() override;
            void buttonEvent(Button* button) override;
            void updateLegend();

            Visualizer* createNewCanvas();
            SyncSinkCanvas* syncSinkCanvas;

        private:
        SyncSink* processor;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SyncSinkEditor);
        ScopedPointer<UtilityButton> addPlotButton, resetTensorButton, rebinButton;
        ScopedPointer<Label> addPlotParamsLabel, stimClassLegendLabel, binParamsLabel;
        Font font;
    };
}

#endif