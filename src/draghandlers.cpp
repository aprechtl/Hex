#include "draghandlers.h"
#include "envelopescene.h"
#include "nodecommands.h"
#include "note.h"
#include "sequencerscene.h"
#include "sequencercommands.h"
#include "trackmanagerdialog.h"
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsView>

DragHandler::DragHandler()
    : dragged(false)
{
}

void DragHandler::onMouseMove(QGraphicsSceneMouseEvent *event)
{
    dragged = true;
    onMouseDragDerived(event);
}

void DragHandler::onMouseRelease()
{
    if (dragged)
        onDraggedMouseReleaseDerived();
    else
        onUndraggedMouseReleaseDerived();
}

NoteDragHandler::NoteDragHandler(QGraphicsItem *note)
    : scene(static_cast<SequencerScene*>(note->scene())),
      draggedItem(note)
{
}

void NoteDragHandler::onUndraggedMouseReleaseDerived()
{
    scene->clearSelection();
    draggedItem->setSelected(true);
    scene->fadeLabelOut();
}

// ###########################################################################
// ########################################################### DERIVED CLASSES

NodeMover::NodeMover(int track, int envelopeIndex, int indexOfDraggedNode,
                     SimpleMap<unsigned int, float> *envelope, const SimpleVector<int> &draggedNodeIndices,
                     TrackManagerDialog *trackManager, EnvelopeScene *scene)
    : track(track),
      envelopeIndex(envelopeIndex),
      indexOfDraggedNode(indexOfDraggedNode),
      envelope(envelope),
      trackManager(trackManager),
      scene(scene),
      draggedNodeIndices(draggedNodeIndices),
      draggedNodeInitialPositions(draggedNodeIndices.size())
{
    const unsigned int *xPositions = envelope->getKeyArray();
    const float *yPositions = envelope->getValueArray();

    // populate the initialPositions array
    for (int i = 0; i < draggedNodeIndices.size(); ++i)
    {
        draggedNodeInitialPositions.append(QPointF(xPositions[draggedNodeIndices[i]], yPositions[draggedNodeIndices[i]]));
    }

    double highestYPos = 0; // default... will change below
    double lowestYPos = 1; // default... will change below

    unsigned int minXOffset = xPositions[indexOfDraggedNode];
    unsigned int maxXOffset = scene->sceneRect().width() - xPositions[indexOfDraggedNode];

    /*
      Here I am finding how far left and right the nodes can be dragged. To do
      so I am iterating through each selected node and finding its neighbors.
      It's a little complicated though because one of the selected nodes could
      be the first or last node in the envelope.
    */

    bool theFirstNodeIsBeingDragged = false;
    for (int i = 0; i < draggedNodeIndices.size(); ++i)
    {
        // find the previous node (that's what the -1 is for)
        int neighborSubscript = draggedNodeIndices[i] - 1;
        if (neighborSubscript < 0) // if this node is the first one in the envelope
        {
            theFirstNodeIsBeingDragged = true;

            if (xPositions[draggedNodeIndices[i]] < minXOffset)
                minXOffset = xPositions[draggedNodeIndices[i]];
        }
        else
        {
            unsigned int neighborNodeXPos = xPositions[neighborSubscript];
            if (!draggedNodeIndices.contains(neighborNodeXPos))
            {
                unsigned int suggestedOffset = xPositions[draggedNodeIndices[i]] - neighborNodeXPos;
                if (suggestedOffset < minXOffset)
                    minXOffset = suggestedOffset;
            }
        }

        // find the following node (that's what the +2 is for)
        neighborSubscript += 2;
        if (neighborSubscript < envelope->count()) // we can ignore the last node
        {
            unsigned int neighborNodeXPos = xPositions[neighborSubscript];
            if (!draggedNodeIndices.contains(neighborNodeXPos)) // if following node is not being dragged
            {
                unsigned int suggestedOffset = neighborNodeXPos - xPositions[draggedNodeIndices[i]];
                if (suggestedOffset < maxXOffset) {maxXOffset = suggestedOffset;}
            }
        }

        if (draggedNodeInitialPositions[i].y() > highestYPos) {highestYPos = draggedNodeInitialPositions[i].y();}
        if (draggedNodeInitialPositions[i].y() < lowestYPos) {lowestYPos = draggedNodeInitialPositions[i].y();}
    }

    // note that for x, we have to add/subtract the snap size to prevent nodes from having exactly the same x position
    minXPos = xPositions[indexOfDraggedNode] - minXOffset + ((theFirstNodeIsBeingDragged) ? 0 : scene->getCurrentSnap());
    maxXPos = xPositions[indexOfDraggedNode] + maxXOffset - scene->getCurrentSnap();
    minYPos = yPositions[indexOfDraggedNode] - lowestYPos;
    maxYPos = yPositions[indexOfDraggedNode] + (1 - highestYPos);

    updateLabel();
}

void NodeMover::onMouseDragDerived(QGraphicsSceneMouseEvent *event)
{
    const unsigned int *xPositions = envelope->getKeyArray();
    const float *yPositions = envelope->getValueArray();

    unsigned int xPos = static_cast<unsigned int>((event->scenePos().x() < 0) ? 0 : event->scenePos().x());
    double yPos = event->scenePos().y();

    // check x and y bounds
    if (xPos < minXPos) {xPos = minXPos;}
    else if (xPos > maxXPos) {xPos = maxXPos;}
    if (yPos < minYPos) {yPos = minYPos;}
    else if (yPos > maxYPos) {yPos = maxYPos;}

    xPos = scene->roundToNearestSnapPos(xPos);
    xPos -= xPositions[indexOfDraggedNode];
    yPos -= yPositions[indexOfDraggedNode];
    // xPos & yPos are now the offset between the mouse position and the dragged item position

    for (int i = 0; i < draggedNodeIndices.size(); ++i)
    {
        unsigned int nodeNewXPos = xPos + xPositions[draggedNodeIndices[i]];
        double nodeNewYPos = yPos + yPositions[draggedNodeIndices[i]];
        trackManager->moveNode(track, envelopeIndex, draggedNodeIndices[i], nodeNewXPos, nodeNewYPos);
    }

    scene->update();
    updateLabel();
}

void NodeMover::onDraggedMouseReleaseDerived()
{
    SimpleVector<QPointF> newPositions(draggedNodeIndices.size());
    for (int i = 0; i < draggedNodeIndices.size(); ++i)
    {
        newPositions.append(QPointF(envelope->keyAt(draggedNodeIndices[i]),
                                    envelope->valueAt(draggedNodeIndices[i])));
    }
    scene->pushUndoCommand(new MoveNodesCommand(track, envelopeIndex, draggedNodeIndices, draggedNodeInitialPositions, newPositions, trackManager));
}

void NodeMover::onUndraggedMouseReleaseDerived()
{
    scene->clearSelectedNodes();
    scene->setNodeSelected(indexOfDraggedNode);
}

void NodeMover::updateLabel()
{
    QString number;
    double value = envelope->valueAt(indexOfDraggedNode);
    if (track == -1)
    {
        switch (envelopeIndex)
        {
        case 0: // beta
            number = QString::number(value * 1200.);
            break;
        case 1: // lambda
            number = QString::number(value);
            break;
        case 2: // gamma
            number = QString::number(value * 2. - 1.);
            break;
        }
    }
    else
    {
        number = QString::number(static_cast<int>(value * 127. + .5));
    }

    scene->setLabelText(QObject::tr("Value: ") + number);
}

// ###########################################################################
// ###########################################################################

HorizontalNoteMover::HorizontalNoteMover(QGraphicsItem *note, double initialClickXOffset)
    : NoteDragHandler(note),
      initialClickXOffset(initialClickXOffset),
      notesToMove(scene->selectedItems()),
      initialPositions(notesToMove.size())
{
    // find leftmost note's x position, and create array of initial positions
    double leftmostNoteXPos = draggedItem->pos().x();
    for (int i = 0; i < notesToMove.size(); ++i)
    {
        initialPositions.appendByValue(notesToMove[i]->pos().x());
        leftmostNoteXPos = (leftmostNoteXPos < notesToMove[i]->pos().x()) ? leftmostNoteXPos : notesToMove[i]->pos().x();
    }

    minXPos = initialClickXOffset + draggedItem->pos().x() - leftmostNoteXPos;
}

void HorizontalNoteMover::onMouseDragDerived(QGraphicsSceneMouseEvent *event)
{
    double draggedItemOldXPos = draggedItem->pos().x();
    double xOffset = ((event->scenePos().x() < minXPos) ? minXPos : event->scenePos().x()) - (draggedItemOldXPos + initialClickXOffset);
    double draggedItemNewPos = scene->roundToNearestSnapPos(draggedItemOldXPos + xOffset);
    double adjustedXOffset = draggedItemNewPos - draggedItemOldXPos;
    for (int i = 0; i < notesToMove.size(); ++i)
    {
        notesToMove[i]->setX(notesToMove[i]->pos().x() + adjustedXOffset);
    }
}

void HorizontalNoteMover::onDraggedMouseReleaseDerived()
{
    scene->pushUndoCommand(new MoveNotesHorizontallyCommand(notesToMove, initialPositions));
}

// ###########################################################################
// ###########################################################################

NoteMover::NoteMover(Note *note, double initialClickXOffset)
    : NoteDragHandler(note),
      initialClickXOffset(initialClickXOffset),
      initialNoteLaneIndex(static_cast<Note*>(note)->getLaneIndex()),
      notesToMove(scene->selectedItems()),
      initialPositions(notesToMove.size())
{
    // find leftmost note's x position, and create array of initial positions
    double leftmostNoteXPos = note->pos().x();

    for (int i = 0; i < notesToMove.size(); ++i)
    {
        initialPositions.appendByValue(notesToMove[i]->pos().x());
        leftmostNoteXPos = (leftmostNoteXPos < notesToMove[i]->pos().x()) ? leftmostNoteXPos : notesToMove[i]->pos().x();
    }

    minXPos = initialClickXOffset + note->pos().x() - leftmostNoteXPos;
}

void NoteMover::onMouseDragDerived(QGraphicsSceneMouseEvent *event)
{
    int closestNoteLane = scene->findClosestNoteLane(event->scenePos().y());

    double draggedItemOldXPos = draggedItem->pos().x();
    double xOffset = ((event->scenePos().x() < minXPos) ? minXPos : event->scenePos().x()) - (draggedItemOldXPos + initialClickXOffset);
    double draggedItemNewPos = scene->roundToNearestSnapPos(draggedItemOldXPos + xOffset);
    double adjustedXOffset = draggedItemNewPos - draggedItemOldXPos;

    for (int i = 0; i < notesToMove.size(); ++i)
    {
        static_cast<Note*>(notesToMove[i])->setLaneIndex(closestNoteLane);
        notesToMove[i]->setX(notesToMove[i]->pos().x() + adjustedXOffset);
    }

    scene->updateNotePositions();
}

void NoteMover::onDraggedMouseReleaseDerived()
{
    scene->pushUndoCommand(new MoveNotesCommand(notesToMove, initialPositions, initialNoteLaneIndex));
}

// ###########################################################################
// ###########################################################################

NoteResizer::NoteResizer(Note *note)
    : NoteDragHandler(note),
      notesToResize(scene->selectedItems()),
      initialLengths(notesToResize.size())
{
    double lengthOfShortestNote = scene->sceneRect().width(); // initialize to some huge value
    for (int i = 0; i < notesToResize.size(); ++i)
    {
        initialLengths.appendByValue(notesToResize[i]->boundingRect().width());
        lengthOfShortestNote = (lengthOfShortestNote < initialLengths[i]) ? lengthOfShortestNote : initialLengths[i];
    }

    draggedNoteOldEndPos = draggedItem->boundingRect().width() + draggedItem->pos().x();
    minXPos = draggedNoteOldEndPos - lengthOfShortestNote + scene->getCurrentSnap();
}

void NoteResizer::onMouseDragDerived(QGraphicsSceneMouseEvent *event)
{
    double eventPosX = scene->roundToNearestSnapPos(event->scenePos().x());
    double noteLengthDifference = ((eventPosX < minXPos) ? minXPos : eventPosX) - draggedNoteOldEndPos;

    for (int i = 0; i < notesToResize.size(); ++i)
    {
        static_cast<Note*>(notesToResize[i])->setWidth(initialLengths[i] + noteLengthDifference);
    }
}

void NoteResizer::onDraggedMouseReleaseDerived()
{
    scene->pushUndoCommand(new ResizeNotesCommand(notesToResize, initialLengths));
}

// ###########################################################################
// ###########################################################################

NoteVelocityAdjuster::NoteVelocityAdjuster(Note *note, double initialClickYOffset)
    : NoteDragHandler(note),
      clickedNoteInitialVelocity(note->getVelocity()),
      initialClickYOffset(initialClickYOffset),
      notes(scene->selectedItems()),
      initialVelocities(notes.size())
{
    scene->setLabelText(QObject::tr("Velocity: ") + QString::number(clickedNoteInitialVelocity));
    scene->fadeLabelIn();

    for (int i = 0; i < notes.size(); ++i)
    {
        initialVelocities.appendByValue(static_cast<Note*>(notes[i])->getVelocity());
    }
}

void NoteVelocityAdjuster::onMouseDragDerived(QGraphicsSceneMouseEvent *event)
{
    int newVelocity = clickedNoteInitialVelocity - qRound((event->pos().y() - initialClickYOffset) * static_cast<QGraphicsView*>(scene->parent())->transform().m22() * .5);

    // check bounds
    if (newVelocity > 127)
        newVelocity = 127;
    else if (newVelocity < 0)
        newVelocity = 0;

    if (newVelocity == static_cast<Note *>(draggedItem)->getVelocity())
        return;

    for (int i = 0; i < notes.size(); ++i)
    {
        static_cast<Note *>(notes[i])->setVelocity(newVelocity);
    }

    scene->setLabelText(QObject::tr("Velocity: ") + QString::number(newVelocity));
}

void NoteVelocityAdjuster::onDraggedMouseReleaseDerived()
{
    scene->setDefaultVelocity(static_cast<Note*>(notes[0])->getVelocity());
    scene->pushUndoCommand(new ChangeNoteVelocitiesCommand(notes, initialVelocities));
    scene->fadeLabelOut();
}

