#include "zoomhandler.h"
#include "sequencersplitterhandle.h"
#include <QtCore/QSettings>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QScrollBar>

const double zoomInFactor = 1.16;
const double zoomOutFactor = 1 / zoomInFactor;

ZoomHandler::ZoomHandler(QGraphicsView *latticeView, QGraphicsView *sequencerView, QGraphicsView *envelopeView, SequencerSplitterHandle *handle, QObject *parent)
    : QObject(parent),
      latticeView(latticeView),
      sequencerView(sequencerView),
      envelopeView(envelopeView),
      sequencerSplitterHandle(handle)
{
}

void ZoomHandler::captureSettings(QSettings *settings)
{
    settings->setValue("lathorizscale", latticeView->transform().m11());
    settings->setValue("seqhorizscale", sequencerView->transform().m11());
    settings->setValue("seqvertscale", sequencerView->transform().m22());
}

void ZoomHandler::resetZooms()
{
    latticeView->setTransform(QTransform::fromScale(120, 120));
    sequencerView->setTransform(QTransform(0.125, 0, 0, 0, 120, 0, 0, 0));
    envelopeView->setTransform(QTransform(0.125, 0, 0, 0, envelopeView->horizontalScrollBar()->height() - envelopeView->height(), 0, 0, 0));
    sequencerSplitterHandle->update();
}

void ZoomHandler::restoreSettings(QSettings *settings)
{
    double latHorizScale = settings->value("lathorizscale", 120.).toDouble();
    double seqHorizScale = settings->value("seqhorizscale", .125).toDouble();
    double seqVertScale = settings->value("seqvertscale", 120.).toDouble();

    latticeView->setTransform(QTransform::fromScale(latHorizScale, seqVertScale));
    sequencerView->setTransform(QTransform::fromScale(seqHorizScale, seqVertScale));
    envelopeView->setTransform(QTransform::fromScale(seqHorizScale, envelopeView->height() - envelopeView->horizontalScrollBar()->height()));
}

void ZoomHandler::zoomInLatticeHorizontally()
{
    latticeView->scale(zoomInFactor, 1);
}

void ZoomHandler::zoomOutLatticeHorizontally()
{
    if (latticeView->transform().m11() < .08)
        return;

    latticeView->scale(zoomOutFactor, 1);
}

void ZoomHandler::zoomInLatticeVertically()
{
    verticallyZoomLattice(zoomInFactor);
}

void ZoomHandler::zoomOutLatticeVertically()
{
    if (latticeView->transform().m22() < .2)
        return;

    verticallyZoomLattice(zoomOutFactor);
}

void ZoomHandler::zoomInSequencersHorizontally()
{
    if (sequencerView->transform().m11() > .99)
        return;

    horizontallyZoomSequencers(1 / ((1 / sequencerView->transform().m11()) - 1));
}

void ZoomHandler::zoomOutSequencersHorizontally()
{
    if (sequencerView->horizontalScrollBar()->maximum() == sequencerView->horizontalScrollBar()->minimum())
        return;

    horizontallyZoomSequencers(1 / ((1 / sequencerView->transform().m11()) + 1));
}

void ZoomHandler::horizontallyZoomSequencers(double zoom)
{
    sequencerView->setTransform(QTransform(zoom, 0, 0, 0, sequencerView->transform().m22(), 0, 0, 0));
    envelopeView->setTransform(QTransform(zoom, 0, 0, 0, envelopeView->transform().m22(), 0, 0, 0));
    sequencerSplitterHandle->update();
}

void ZoomHandler::verticallyZoomLattice(double zoom)
{
    latticeView->scale(1, zoom);
    sequencerView->scale(1, zoom);
}
