#include "abstractsequencerscene.h"
#include "barlinedrawer.h"
#include <QtCore/QMimeData>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QClipboard>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QUndoStack>

AbstractSequencerScene::AbstractSequencerScene(BarLineDrawer *barLineDrawer, QUndoStack *undoStack, QWidget *view)
    : QGraphicsScene(view),
      barLineDrawer(barLineDrawer),
      undoStack(undoStack),
      m_label(new QLabel(view)),
      m_cursorPos(0),
      m_snapEnabled(false),
      m_snapSize(1),
      labelFadeEffect(new QGraphicsOpacityEffect(this)),
      fadeAnimation(new QPropertyAnimation(labelFadeEffect, "opacity"))
{
    m_label->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_label->setGraphicsEffect(labelFadeEffect);
    fadeAnimation->setDuration(150);
    labelFadeEffect->setOpacity(0);
}

void AbstractSequencerScene::copy()
{
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);

    if (!copyImplementation(stream))
        return;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(mimeType(), itemData);
    QApplication::clipboard()->setMimeData(mimeData);
}

void AbstractSequencerScene::cut()
{
    copy();
    deleteSelectedItems();
}

void AbstractSequencerScene::deleteSelectedItems()
{
    QUndoCommand *command = deleteCommand();

    if (command == NULL)
        return;

    undoStack->push(command);

    update();
}

void AbstractSequencerScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    barLineDrawer->drawBarLines(painter, rect);
}

void AbstractSequencerScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(cursorPen);
    painter->drawLine(QPointF(m_cursorPos, rect.top()), QPointF(m_cursorPos, rect.bottom()));
}

void AbstractSequencerScene::fadeLabelIn()
{
    fadeAnimation->stop();
    fadeAnimation->setEndValue(1.0);
    fadeAnimation->start();
}

void AbstractSequencerScene::fadeLabelOut()
{
    fadeAnimation->stop();
    fadeAnimation->setEndValue(0);
    fadeAnimation->start();
}

void AbstractSequencerScene::paste()
{
    clearSelection();

    QByteArray pastedData(QApplication::clipboard()->mimeData()->data(mimeType()));

    if (pastedData.isNull())
        return;

    QDataStream stream(&pastedData, QIODevice::ReadOnly);

    int numItems;
    stream >> numItems;

    if (numItems == 0)
        return;

    undoStack->push(pasteCommand(stream, numItems));
}

void AbstractSequencerScene::pushUndoCommand(QUndoCommand *command)
{
    undoStack->push(command);
}

void AbstractSequencerScene::roundSetAndEmitCursorPos(unsigned int pos)
{
    setCursorPos(roundToNearestSnapPos(pos));
    emit cursorMoved(m_cursorPos);
}

void AbstractSequencerScene::setLength(double length)
{
    setSceneRect(0, sceneRect().top(), length, sceneRect().height());
    update();
}

void AbstractSequencerScene::setTopAndHeight(double top, double height)
{
    setSceneRect(0, top, sceneRect().width(), height);
    update();
}

void AbstractSequencerScene::setLabelText(const QString &string)
{
    m_label->setText(string);
    m_label->adjustSize();
}
