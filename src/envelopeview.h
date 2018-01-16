#ifndef ENVELOPEVIEW_H
#define ENVELOPEVIEW_H
#include <QtWidgets/QGraphicsView>
#include "envelopescene.h"
#include <QtGui/QMouseEvent>
#include <QtWidgets/QScrollBar>

/* EnvelopeView subclasses QGraphicsView for two reasons. First, it provides
 * appropriate rubber band functionality for EnvelopeScene. EnvelopeScene can't
 * use the default rubber band functionality because it isn't QGraphicsItem-
 * based, like most QGraphicsScenes. Second, it reimplements resizeEvent to
 * ensure that the view is always appropriately scaled to show the entire
 * vertical dimension of the scene. */

class EnvelopeView : public QGraphicsView
{
public:
    EnvelopeView(QWidget *parent = 0) : QGraphicsView(parent)
    {}

    void startRubberBandDrag() { isRubberBandDragging = true; }

protected:
    void mousePressEvent(QMouseEvent *event)
    {
        isRubberBandDragging = false;
        clickScenePos = mapToScene(event->pos());
        QGraphicsView::mousePressEvent(event); // this may result in startRubberBandDrag being called
    }

    void mouseMoveEvent(QMouseEvent *event)
    {
        QGraphicsView::mouseMoveEvent(event);

        if (isRubberBandDragging)
            static_cast<EnvelopeScene*>(scene())->evaluateSelection(QRectF(clickScenePos, mapToScene(event->pos())).normalized());
    }

    void resizeEvent(QResizeEvent *event)
    {
#ifdef Q_OS_WIN32
        setTransform(QTransform::fromScale(transform().m11(), -viewport()->height()));
#endif
#ifdef Q_OS_MAC // on mac the horizontal scroll bar is invisible
        setTransform(QTransform::fromScale(transform().m11(), horizontalScrollBar()->height() - viewport()->height()));
#endif
        QGraphicsView::resizeEvent(event);
    }

private:
    QPointF clickScenePos;
    bool isRubberBandDragging;
};

#endif // ENVELOPEVIEW_H
