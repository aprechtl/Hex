#include "note.h"
#include "draghandlers.h"
#include "sequencerscene.h"
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneHoverEvent>
#include <QtWidgets/QGraphicsView>

Note::Note(float length, unsigned char velocity, unsigned short int laneIndex, int track)
    : bRect(QRectF(0, -.05, length, .1)), laneIndex(laneIndex), velocity(velocity), dragHandler(0), m_track(track)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setZValue(-laneIndex);
}

NoteDragHandler *Note::createNoteDragger(QGraphicsSceneMouseEvent *event)
{
    if (hoveredOnRightOfNote(event->pos().x(), static_cast<QGraphicsView*>(event->widget()->parent())->transform().m11()))
        return new NoteResizer(this);

    if (hoveredOnTopOfNote(event->pos().y(), static_cast<QGraphicsView*>(event->widget()->parent())->transform().m22()))
        return new NoteVelocityAdjuster(this, event->pos().y());

    if (selectedNotesAreInTheSameLane())
        return new NoteMover(this, event->pos().x());

    return new HorizontalNoteMover(this, event->pos().x());
}

bool Note::hoveredOnRightOfNote(double xPos, double xScale) const
{ return xPos > bRect.right() - 8 / xScale; }

bool Note::hoveredOnTopOfNote(double yPos, double yScale) const
{ return yPos < bRect.top() + 3 / yScale; }

void Note::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{ setCursor(QCursor()); }

void Note::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (hoveredOnRightOfNote(event->pos().x(), static_cast<QGraphicsView*>(event->widget()->parent())->transform().m11()))
        setCursor(Qt::SizeHorCursor);
    else if (hoveredOnTopOfNote(event->pos().y(), static_cast<QGraphicsView*>(event->widget()->parent())->transform().m22()))
        setCursor(Qt::SizeVerCursor);
    else setCursor(QCursor());
}

void Note::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (dragHandler != NULL)
        dragHandler->onMouseMove(event);
}

void Note::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // ensure press was actually intended for this note
    if (m_track != static_cast<SequencerScene*>(scene())->currentTrack())
    {
        event->ignore();
        return;
    }

    if (event->modifiers().testFlag(Qt::ControlModifier) || event->modifiers().testFlag(Qt::ShiftModifier))
    {
        setSelected(!isSelected());
    }
    else
    {
        if (!isSelected())
        {
            scene()->clearSelection();
            setSelected(true);
        }

        dragHandler = createNoteDragger(event);
    }
}

void Note::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    if (dragHandler != NULL)
    {
        dragHandler->onMouseRelease();
        delete dragHandler;
        dragHandler = 0;
    }
}

void Note::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (static_cast<SequencerScene*>(scene())->currentTrack() == m_track)
    {
        painter->setPen((isSelected() ? static_cast<SequencerScene*>(scene())->getSelectedNotePen()
                                      : static_cast<SequencerScene*>(scene())->getUnselectedNotePen()));

        painter->setBrush(static_cast<SequencerScene*>(scene())->getActiveNoteBrush(velocity));
    }
    else
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(static_cast<SequencerScene*>(scene())->getInactiveNoteBrush(velocity));
    }

    painter->drawRect(bRect);
}

bool Note::selectedNotesAreInTheSameLane() const
{
    SimpleVector<QGraphicsItem*> selectedItems(scene()->selectedItems());
    for (int i = 0; i < selectedItems.size(); ++i)
    {
        if (laneIndex != static_cast<Note*>(selectedItems[i])->getLaneIndex())
            return false;
    }

    return true;
}
