#include "sequencersplitterhandle.h"
#include "abstractsequencerscene.h"
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QScrollBar>
#include <math.h>

SequencerSplitterHandle::SequencerSplitterHandle(QGraphicsView *sequencerView, QWidget *parent)
    : QWidget(parent),
      view(sequencerView),
      loopEnabled(false),
      measureLengthTicks(100)
{
#ifdef Q_OS_WIN32
    setToolTip("Left-click and drag to adjust the sizes of the panes."
               "<br><br>"
               "Right-click or Ctrl-click and drag to create a loop.");
#endif
#ifdef Q_OS_MAC
    setToolTip("Click and drag to adjust the sizes of the panes."
               "<br><br>"
               "Alt-click and drag to create a loop.");
#endif
}

void SequencerSplitterHandle::mouseMoveEvent(QMouseEvent *event)
{
    if (!mouseIsAdjustingLoop)
    {
        QWidget::mouseMoveEvent(event);
        return;
    }

    int eventXPos = event->pos().x();
    if (eventXPos < 0)
        eventXPos = 0;

    loopEnabled = true;
    draggedClickTickPos = static_cast<AbstractSequencerScene*>(view->scene())->roundToNearestSnapPos((view->horizontalScrollBar()->value() + eventXPos) / view->transform().m11());

    update();
}

void SequencerSplitterHandle::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_WIN32
    mouseIsAdjustingLoop = (event->button() == Qt::RightButton || event->modifiers().testFlag(Qt::ControlModifier));
#endif
#ifdef Q_OS_MAC
    mouseIsAdjustingLoop = (event->modifiers().testFlag(Qt::AltModifier));
#endif

    if (!mouseIsAdjustingLoop)
    {
        event->ignore();
        QWidget::mousePressEvent(event);
        return;
    }

    loopEnabled = false;
    initialClickTickPos = static_cast<AbstractSequencerScene*>(view->scene())->roundToLowerSnapPos((view->horizontalScrollBar()->value() + event->pos().x()) / view->transform().m11());
    QApplication::setOverrideCursor(Qt::IBeamCursor);
    update();
    event->accept();
}

void SequencerSplitterHandle::mouseReleaseEvent(QMouseEvent *event)
{
    if (!mouseIsAdjustingLoop)
    {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    double releaseClickTickPosition = static_cast<AbstractSequencerScene*>(view->scene())->roundToNearestSnapPos((view->horizontalScrollBar()->value() + event->pos().x()) / view->transform().m11());

    // disable the loop if it was dragged, but then released at the same position as the initial click
    if (qAbs(releaseClickTickPosition - initialClickTickPos) < static_cast<AbstractSequencerScene*>(view->scene())->getCurrentSnap())
    {
        loopEnabled = false;
        emit loopDisabled();
    }
    else
    {
        if (releaseClickTickPosition < initialClickTickPos)
            emit loopChanged(releaseClickTickPosition, initialClickTickPos);
        else
            emit loopChanged(initialClickTickPos, releaseClickTickPosition);
    }

    update();
    QApplication::restoreOverrideCursor();
}

void SequencerSplitterHandle::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // ============================================== DRAW THE MEASURE NUMBERS
    double measureLengthPixels = measureLengthTicks * view->transform().m11();
    int measureNumber = ceil(view->horizontalScrollBar()->value() / measureLengthPixels); // first measure number

    if (measureNumber == 0)
        ++measureNumber; // the 0 isn't drawn

    /* subtracting 20 at the end because we have to account for the fact that
     * the measure numbers are being drawn inside a rectangle of width = 40
     * (using a rectangle so that the numbers can be vertically centered) */
    double measureNumberPixelPosition = measureNumber * measureLengthPixels - view->horizontalScrollBar()->value() + 1 - 20;
    int rectRight = event->rect().right() - 20; // ditto
    int rectHeight = event->rect().height();

    while (measureNumberPixelPosition < rectRight)
    {
        painter.drawText(QRectF(measureNumberPixelPosition, 0, 40, rectHeight), Qt::AlignCenter, QString::number(measureNumber));
        ++measureNumber;
        measureNumberPixelPosition += measureLengthPixels;
    }
    // =======================================================================

    // ================================================== DRAW THE LOOP BOUNDS
    if (loopEnabled)
    {
        double initialClickPixelPos = initialClickTickPos * view->transform().m11();
        painter.setBrush(QBrush(QColor(0, 0, 0, 50), Qt::SolidPattern));
        painter.drawRect(QRectF(initialClickPixelPos - view->horizontalScrollBar()->value() + 1, -1,  // left, top
                                draggedClickTickPos * view->transform().m11() - initialClickPixelPos, // width
                                event->rect().height() + 1));                                         // height
    }
    // =======================================================================
}

void SequencerSplitterHandle::setMeasureLengthTicks(double length)
{
    measureLengthTicks = length;
    update();
}
