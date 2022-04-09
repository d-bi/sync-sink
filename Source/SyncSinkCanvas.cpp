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
    display->setBounds(0, 0, getWidth(), getHeight());
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
        for (PSTHPlot* plot : plots) {
            int w_i = plot->identifier % 4;
            int h_i = plot->identifier / 4;
            plot->setBounds(w_i * getWidth() / 4, h_i * getHeight() / 2, getWidth() / 4, getHeight() / 2);
        }
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
    PSTHPlot* plot = new PSTHPlot(processor, canvas, this, channel_idx, sorted_id, stim_class, (plotCounter++) % 8);
    addAndMakeVisible(plot);
    plots.add(plot);
    while (plots.size() > 8) {
        plots.remove(0);
    }
    std::cout << plots.size() << std::endl;
}

PSTHPlot::PSTHPlot(SyncSink* s, SyncSinkCanvas* c, SyncSinkDisplay* d,
    int channel_idx, int sorted_id, int stim_class, int identifier) :
    canvas(c), processor(s), display(d),
    channel_idx(channel_idx), sorted_id(sorted_id), stim_class(stim_class),
    identifier(identifier)
{
    font = Font("Default", 15, Font::plain);
    int w_i = identifier % 4;
    int h_i = identifier / 4;
    setBounds(w_i * d->getWidth() / 4, h_i * d->getHeight() / 2, d->getWidth() / 4, d->getHeight() / 2);
}

PSTHPlot::~PSTHPlot()
{
}

void PSTHPlot::paint(Graphics& g)
{
    g.fillAll(Colours::white);
    std::cout << "psth paint " << identifier << std::endl;
    g.drawRect(0, 0, getWidth(), getHeight());
    g.setFont(font);
    g.drawText(String::formatted("PSTH ch-%d unit-%d cl-%d", channel_idx, sorted_id, stim_class), 10, 0, 200, 20, Justification::left, false);
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
 //           std::cout << max_y << std::endl;
            for (int i = 0; i < 49; i++)
            {
                float y1 = max_y == 0 ? 0 : float (histogram[i]) / max_y * h;
                float y2 = max_y == 0 ? 0 : float (histogram[i + 1]) / max_y * h;
//                std::cout << y1 << " ";
                g.drawLine(x, h - y1, x + dx, h - y2, 2);
                x += dx;
            }
//            std::cout << std::endl;
        }
    }
}

void PSTHPlot::resized()
{
    repaint();
}
