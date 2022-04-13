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
        void addPlot(int channel_idx, int sorted_id, std::vector<int> stimClasses);

        std::vector<Colour> colorList = {
            Colour(30,118,179), Colour(255,126,13), Colour(43,159,43),
            Colour(213,38,39), Colour(147,102,188), Colour(139,85,74),
            Colour(226,118,193), Colour(126,126,126), Colour(187,188,33), 
            Colour(22,189,206)};

        //std::vector<String> conditionList = {"face","object","scramble"};
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
        void addPSTHPlot(int channel_idx, int sorted_id, std::vector<int> stimClasses);

    private:
        SyncSink* processor;
        int plotCounter = 0;
        SyncSinkCanvas* canvas;
        Viewport* viewport;
        OwnedArray<PSTHPlot> plots;

    };

    class PSTHPlot : public Component
    {
    public:
        PSTHPlot(SyncSink* s, SyncSinkCanvas* c, SyncSinkDisplay* d,
            int channel_idx, int sorted_id, int stim_class, int identifier);
        PSTHPlot(SyncSink* s, SyncSinkCanvas* c, SyncSinkDisplay* d,
            int channel_idx, int sorted_id, std::vector<int> stimClasses, int identifier);
        ~PSTHPlot();

        void paint(Graphics& g);
        void resized();
        void clearPlot();

        //void updatePlot();
        
        int channel_idx;
        int sorted_id;
        //int stim_class;
        std::vector<int> stimClasses;
        int identifier;
    private:
        SyncSink* processor;
        SyncSinkDisplay* display;
        SyncSinkCanvas* canvas;
        Font font;
        String name;
        std::vector<double> histogram;
        bool alive;
    };

}

#endif