#include "SyncSinkCanvas.h"
#include <vector>

using namespace SyncSinkSpace;

SyncSinkCanvas::SyncSinkCanvas(SyncSink* s) : processor(s)
{
    processor->setCanvas(this);

    viewport = new Viewport();
    display = new SyncSinkDisplay(s, this, viewport);

    viewport->setViewedComponent(display, false);
    viewport->setScrollBarsShown(true, true);

    addAndMakeVisible(viewport);

    setWantsKeyboardFocus(true);

    update();

}

SyncSinkCanvas::~SyncSinkCanvas()
{
}

void SyncSinkCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
}

void SyncSinkCanvas::refresh()
{
    repaint();
}

void SyncSinkCanvas::beginAnimation()
{
    startCallbacks();
}

void SyncSinkCanvas::endAnimation()
{
    stopCallbacks();
}

void SyncSinkCanvas::refreshState()
{
    resized();
}

void SyncSinkCanvas::update()
{
    std::cout << "update canvas" << std::endl;

    display->resized();
    display->repaint();
}

void SyncSinkCanvas::buttonClicked(Button* button)
{
    repaint();
}

void SyncSinkCanvas::resized()
{
    viewport->setBounds(0, 0, getWidth(), getHeight());
    display->setBounds(0, 0, 300, 300);
}

void SyncSinkCanvas::updatePlots()
{
    display->updatePlots();
}

void SyncSinkCanvas::addPlot(int channel_idx, int sorted_id, int stim_class)
{
    display->addPSTHPlot(channel_idx, sorted_id, stim_class);
}



SyncSinkDisplay::SyncSinkDisplay(SyncSink* s, SyncSinkCanvas* c, Viewport* v) :
    processor(s), canvas(c), viewport(v)
{
}

SyncSinkDisplay::~SyncSinkDisplay()
{
}

void SyncSinkDisplay::paint(Graphics& g)
{
    g.fillAll(Colours::grey);
    g.drawText(String(processor->getNTrial()), getLocalBounds(), juce::Justification::centred, true);
}

void SyncSinkDisplay::resized()
{
    std::cout << "display resized" << std::endl;
    setBounds(0, 0, getWidth(), getHeight());
    if (plots.size() > 0)
    {
        plots[0]->setBounds(0, 0, getWidth(), getHeight());
        std::cout << plots[0]->getX() << " " << plots[0]->getY() << " " << plots[0]->getWidth() << " " << plots[0]->getHeight() << std::endl;
    }
}

void SyncSinkDisplay::removePlots()
{
}

void SyncSinkDisplay::clear()
{
}

void SyncSinkDisplay::updatePlots()
{
    for (PSTHPlot* plot : plots) {
        std::cout << "updating plot" << std::endl;
        plot->repaint();
    }
//    repaint();
}

void SyncSinkDisplay::addPSTHPlot(int channel_idx, int sorted_id, int stim_class)
{
    std::cout << "add PSTH plot with params"
        << " channel_idx=" << channel_idx
        << "; sorted_id=" << sorted_id
        << "; stim_class=" << stim_class
        << std::endl;
    PSTHPlot* plot = new PSTHPlot(processor, canvas, channel_idx, sorted_id, stim_class);
    addAndMakeVisible(plot);
    plots.add(plot);
}

PSTHPlot::PSTHPlot(SyncSink* s, SyncSinkCanvas* c, int channel_idx, int sorted_id, int stim_class) :
    canvas(c), processor(s), channel_idx(channel_idx), sorted_id(sorted_id), stim_class(stim_class)
{
    font = Font("Default", 15, Font::plain);
    setBounds(0, 0, 300, 300);
}

PSTHPlot::~PSTHPlot()
{
}

void PSTHPlot::paint(Graphics& g)
{
    g.fillAll(Colours::white);
    std::cout << "psth paint" << getWidth() << getHeight() << std::endl;
    g.drawRect(0, 0, getWidth(), getHeight());
    g.setFont(font);
    g.drawText(String::formatted("PSTH PLOT %d %d %d", channel_idx, sorted_id, stim_class), 10, 0, 200, 20, Justification::left, false);
    if (processor)
    {
        std::vector<double> histogram = processor->getHistogram(channel_idx, sorted_id, stim_class);
        if (histogram.size() >= 50) {
            g.drawText(String(processor->getNTrial()), getLocalBounds(), juce::Justification::centred, true);
            g.setColour(Colours::blue);
            float dx = getWidth() / float(50);
            float h = getHeight();
            float x = 0.0f;
            double max_y = *std::max_element(histogram.begin(), histogram.end());
            //            double min_y = *std::min_element(histogram.begin(), histogram.end());
            std::cout << max_y << std::endl;
            for (int i = 0; i < 49; i++)
            {
                float y1 = max_y == 0 ? 0 : (histogram[i]) / max_y * h;
                float y2 = max_y == 0 ? 0 : (histogram[i + 1]) / max_y * h;
                std::cout << y1 << " ";
                g.drawLine(x, y1, x + dx, y2, 2);
                x += dx;
            }
            std::cout << std::endl;
        }
    }
}

void PSTHPlot::resized()
{
    repaint();
}
