#ifndef SYNCSINKCANVAS_H_DEFINED
#define SYNCSINKCANVAS_H_DEFINED

#include <VisualizerWindowHeaders.h>
#include "SyncSink.h"
#include <vector>

namespace SyncSinkSpace
{
    class SyncSinkDisplay;
    class PSTHPlot;
    class HistogramAxes;

    class SyncSinkCanvas : public Visualizer, public Button::Listener
    {
    public:
        SyncSinkCanvas(SyncSink* s);
        ~SyncSinkCanvas();
        void paint(Graphics& g);
        void refresh();

        void beginAnimation();
        void endAnimation();
        void refreshState();
        void update();
        void setParameter(int, float) {}
        void setParameter(int, int, int, float) {}
        void buttonClicked(Button* button);
        void resized();

        void updatePlots();
        void addPlot(int channel_idx, int sorted_id, int stim_class);

        SyncSink* processor;

    private:
        ScopedPointer<Viewport> viewport;
        ScopedPointer<SyncSinkDisplay> display;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SyncSinkCanvas);

    };

    class SyncSinkDisplay : public Component
    {
    public:
        SyncSinkDisplay(SyncSink* s, SyncSinkCanvas* c, Viewport* v);
        ~SyncSinkDisplay();
        void paint(Graphics& g);
        void resized();

        void removePlots();
        void clear();
        
        void updatePlots();
        void addPSTHPlot(int channel_idx, int sorted_id, int stim_class);

    private:
        SyncSink* processor;
        SyncSinkCanvas* canvas;
        Viewport* viewport;
        OwnedArray<PSTHPlot> plots;

    };

    class PSTHPlot : public Component
    {
    public:
        PSTHPlot(SyncSink* s, SyncSinkCanvas* c, int channel_idx, int sorted_id, int stim_class);
        ~PSTHPlot();

        void paint(Graphics& g);
        void resized();

        void updatePlot();
        
        SyncSinkCanvas* canvas;
        int channel_idx;
        int sorted_id;
        int stim_class;
    private:
        SyncSink* processor;
        Font font;
        String name;
        std::vector<double> histogram;
    };

}

#endif