#ifndef ZOOMHANDLER_H
#define ZOOMHANDLER_H
#include <QtCore/QObject>

class QGraphicsView;
class QSettings;
class SequencerSplitterHandle;

class ZoomHandler : public QObject
{
public:
    ZoomHandler(QGraphicsView *latticeView, QGraphicsView *sequencerView, QGraphicsView *envelopeView, SequencerSplitterHandle *handle, QObject *parent = 0);
    void captureSettings(QSettings *settings);
    void resetZooms();
    void restoreSettings(QSettings *settings);
    void zoomInLatticeHorizontally();
    void zoomInLatticeVertically();
    void zoomInSequencersHorizontally();
    void zoomOutLatticeHorizontally();
    void zoomOutLatticeVertically();
    void zoomOutSequencersHorizontally();

private:
    void horizontallyZoomSequencers(double zoom);
    void verticallyZoomLattice(double zoom);

    QGraphicsView *latticeView;
    QGraphicsView *sequencerView;
    QGraphicsView *envelopeView;
    SequencerSplitterHandle *sequencerSplitterHandle;
};

#endif
