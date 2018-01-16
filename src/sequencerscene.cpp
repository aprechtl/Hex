#include "sequencerscene.h"
#include "latticedata.h"
#include "note.h"
#include "notestruct.h"
#include "sequencercommands.h"
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsView>

// ===================================================== QDATASTREAM OPERATORS
QDataStream &operator>>(QDataStream &in, NoteStruct &note)
{
    in >> note.track >> note.startPosition >> note.duration >> note.velocity >> note.j >> note.k;
    return in;
}

QDataStream &operator<<(QDataStream &out, Note *note)
{
    short int j, k;
    HexSettings::convertNoteLaneIndexToJK(note->getLaneIndex(), j, k);
    out << note->track() << static_cast<float>(note->pos().x()) << static_cast<float>(note->boundingRect().width()) << note->getVelocity() << j << k;
    return out;
}
// ===========================================================================

const double noteLaneWidth = .09;
const double halfNoteLaneWidth = noteLaneWidth * .5;

SequencerScene::SequencerScene(LatticeData *latticeData, BarLineDrawer *barLineDrawer, QUndoStack *undoStack, QWidget *view)
    : AbstractSequencerScene(barLineDrawer, undoStack, view),
      latticeData(latticeData),
      noteBeingCreated(0),
      defaultVelocity(90),
      darkLaneBrush(Qt::SolidPattern),
      lightLaneBrush(Qt::SolidPattern),
      pressedLaneBrush(Qt::SolidPattern),
      m_currentTrack(0)
{
    selectedNotePen.setCosmetic(true);
    unselectedNotePen.setCosmetic(true);

    for (int i = 0; i < 128; ++i)
    {
        activeNoteBrushes[i].setStyle(Qt::SolidPattern);
        inactiveNoteBrushes[i].setStyle(Qt::SolidPattern);
    }
}

QGraphicsItem *SequencerScene::addNote(const NoteStruct &note)
{
    int noteLaneIndex = HexSettings::convertJKToNoteLaneIndex(note.j, note.k);
    Note *newNote = new Note(note.duration, note.velocity, noteLaneIndex, note.track);
    newNote->setPos(note.startPosition, latticeData->buttonPositions[noteLaneIndex].y());
    addItem(newNote);
    return newNote;
}

bool SequencerScene::copyImplementation(QDataStream &stream)
{
    SimpleVector<QGraphicsItem*> itemsToCopy(selectedItems());

    if (itemsToCopy.size() == 0)
        return false;

    stream << itemsToCopy.size();

    for (int i = 0; i < itemsToCopy.size(); ++i)
    {
        stream << static_cast<Note*>(itemsToCopy[i]);
    }

    return true;
}

QUndoCommand *SequencerScene::deleteCommand()
{
    SimpleVector<QGraphicsItem*> itemsToDelete(selectedItems());

    if (itemsToDelete.size() == 0)
        return 0;
    else
        return new DeleteNotesCommand(itemsToDelete);
}

void SequencerScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setBrush(darkLaneBrush);
    drawLineSet(painter, rect, latticeData->darkIndices);
    painter->setBrush(lightLaneBrush);
    drawLineSet(painter, rect, latticeData->lightIndices);
    painter->setBrush(pressedLaneBrush);
    drawLineSet(painter, rect, latticeData->pressedIndices);

    painter->setRenderHint(QPainter::Antialiasing, false);

    AbstractSequencerScene::drawBackground(painter, rect); // draws the bar lines
}

void SequencerScene::drawLineSet(QPainter *painter, const QRectF &rect, const SimpleVector<unsigned int> &indices)
{
    // QRectF is implemented in terms of left/top/width/height, so it's faster to calculate this just once
    double rectBottom = rect.bottom() + noteLaneWidth; // allows lanes slightly off screen to be partially visible

    for (int i = 0; i < indices.size(); ++i)
    {
        if (latticeData->buttonPositions[indices[i]].y() > rect.top() && latticeData->buttonPositions[indices[i]].y() < rectBottom)
            painter->drawRect(QRectF(rect.left(), latticeData->buttonPositions[indices[i]].y() - halfNoteLaneWidth, rect.width(), noteLaneWidth));
    }
}

int SequencerScene::findClosestNoteLane(double yPos) const
{
    double closestDistance = 999999999;
    unsigned int closestIndex = 0;

    auto scanIndices = [=](const SimpleVector<unsigned int> &indices, double &closestDistance, unsigned int &closestIndex) {
        for (int i = 0; i < indices.size(); ++i) // look through the light lines
        {
            double thisDistance = qAbs(yPos - latticeData->buttonPositions[indices[i]].y());
            if (thisDistance < closestDistance)
            {
                closestDistance = thisDistance;
                closestIndex = indices[i];
            }
        }
    };

    scanIndices(latticeData->lightIndices, closestDistance, closestIndex);
    scanIndices(latticeData->darkIndices, closestDistance, closestIndex);

    return closestIndex;
}

int SequencerScene::findTopmostNoteLane(double yPos) const
{
    double yPosLowerBound = yPos + halfNoteLaneWidth;
    double yPosUpperBound = yPos - halfNoteLaneWidth;

    for (int i = 0; i < latticeData->lightIndices.size(); ++i)
    {
        if (yPosLowerBound > latticeData->buttonPositions[latticeData->lightIndices[i]].y() && yPosUpperBound < latticeData->buttonPositions[latticeData->lightIndices[i]].y())
            return latticeData->lightIndices[i];
    }

    for (int i = 0; i < latticeData->darkIndices.size(); ++i)
    {
        if (yPosLowerBound > latticeData->buttonPositions[latticeData->darkIndices[i]].y() && yPosUpperBound < latticeData->buttonPositions[latticeData->darkIndices[i]].y())
            return latticeData->darkIndices[i];
    }

    return -1; // no note lane found
}

void SequencerScene::insertTrack(int index, const SimpleVector<Note*> &itemsInTrack)
{
    SimpleVector<QGraphicsItem*> allItems(items());
    for (int i = 0; i < allItems.size(); ++i)
    {
        if (static_cast<Note*>(allItems[i])->track() >= index)
            static_cast<Note*>(allItems[i])->setTrack(static_cast<Note*>(allItems[i])->track() + 1);
    }

    for (int i = 0; i < itemsInTrack.size(); ++i)
    {
        itemsInTrack[i]->setTrack(index); // items might not actually "be" in this track
        addItem(itemsInTrack[i]);
    }
}

void SequencerScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (noteBeingCreated != NULL)
    {
        if (noteBeingCreated->scene() == 0)
            addItem(noteBeingCreated);

        double noteWidth = roundToNearestSnapPos(event->scenePos().x()) - noteBeingCreated->pos().x();

        if (noteWidth < 0)
            noteWidth = 0;

        noteBeingCreated->setWidth(noteWidth);
    }
    else
    {
        QGraphicsScene::mouseMoveEvent(event);
    }
}
#include <QDebug>
void SequencerScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    noteBeingCreated = NULL;

    if (event->button() != Qt::LeftButton)
        return; // start a rubber band drag

    // check if an item wants the event
    QGraphicsScene::mousePressEvent(event);
    if (event->isAccepted()) // if clicked on a note
        return;

    int noteLaneIndex = findTopmostNoteLane(event->scenePos().y());
    if (noteLaneIndex == -1) // if no note lane at click position
        return; // start a rubber band drag

    clearSelection();
    noteBeingCreated = new Note(snapSize(), defaultVelocity, noteLaneIndex, currentTrack()); // create but do not add (unless dragged)
    noteBeingCreated->setPos(roundToLowerSnapPos(event->scenePos().x()), latticeData->buttonPositions[noteLaneIndex].y());
    noteBeingCreated->setSelected(true);
    event->accept(); // prevents rubber band drag
}

void SequencerScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (noteBeingCreated != NULL)
    {
        if (noteBeingCreated->scene() == 0 || noteBeingCreated->boundingRect().width() == 0)
        {
            delete noteBeingCreated;
            roundSetAndEmitCursorPos(event->scenePos().x());
        }
        else
        {
            SimpleVector<QGraphicsItem*> note(1);
            note.appendByValue(noteBeingCreated);
            pushUndoCommand(new AddNotesCommand(note, this));
        }

        noteBeingCreated = NULL;
    }
    else
    {
        QGraphicsScene::mouseReleaseEvent(event);

        if (event->screenPos() == event->buttonDownScreenPos(event->button()))
            roundSetAndEmitCursorPos(event->scenePos().x());
    }
}

QUndoCommand *SequencerScene::pasteCommand(QDataStream &stream, int numItems)
{
    NoteStruct *copiedNotes = new NoteStruct[numItems];
    double leftmostCopiedNoteXPos = 999999999;
    for (int i = 0; i < numItems; ++i)
    {
        stream >> copiedNotes[i];
        if (copiedNotes[i].startPosition < leftmostCopiedNoteXPos)
            leftmostCopiedNoteXPos = copiedNotes[i].startPosition;
    }

    double pastedNoteOffset = cursorPos() - leftmostCopiedNoteXPos;

    SimpleVector<QGraphicsItem*> notesToPaste(numItems);
    for (int i = 0; i < numItems; ++i)
    {
        int noteLaneIndex = HexSettings::convertJKToNoteLaneIndex(copiedNotes[i].j, copiedNotes[i].k);
        Note *newNote = new Note(copiedNotes[i].duration, copiedNotes[i].velocity, noteLaneIndex, currentTrack());
        newNote->setPos(copiedNotes[i].startPosition + pastedNoteOffset, latticeData->buttonPositions[noteLaneIndex].y());
        newNote->setSelected(true);
        notesToPaste.appendByValue(newNote);
    }

    delete [] copiedNotes;

    AddNotesCommand *addNotesCommand = new AddNotesCommand(notesToPaste, this);
    addNotesCommand->setText((numItems == 1) ? tr("Paste Note") : tr("Paste Notes"));
    return addNotesCommand;
}

SimpleVector<Note*> SequencerScene::removeTrack(int index)
{
    SimpleVector<QGraphicsItem*> allItems(items());
    SimpleVector<Note*> itemsToRemove(allItems.size());
    for (int i = 0; i < allItems.size(); ++i)
    {
        if (static_cast<Note*>(allItems[i])->track() == index)
        {
            itemsToRemove.appendByValue(static_cast<Note*>(allItems[i]));
            removeItem(allItems[i]);
        }
        else if (static_cast<Note*>(allItems[i])->track() > index)
        {
            static_cast<Note*>(allItems[i])->setTrack(static_cast<Note*>(allItems[i])->track() - 1);
        }
    }

    return itemsToRemove;
}

void SequencerScene::restoreNotes(QDataStream &in)
{
    clear();

    int numNotes;
    in >> numNotes;

    NoteStruct note;
    for (int i = 0; i < numNotes; ++i)
    {
        in >> note;
        addNote(note);
    }
}

void SequencerScene::saveNotes(QDataStream &out) const
{
    SimpleVector<QGraphicsItem*> allNotes(items());
    out << allNotes.size();
    for (int i = 0; i < allNotes.size(); ++i)
    {
        out << static_cast<Note*>(allNotes[i]);
    }
}

void SequencerScene::selectAll()
{
    SimpleVector<QGraphicsItem*> allItems(items());

    for (int i = 0; i < allItems.size(); ++i)
    {
        if (m_currentTrack == static_cast<Note*>(allItems[i])->track())
            allItems[i]->setSelected(true);
    }
}

void SequencerScene::setCurrentTrack(int track)
{
    if (track == m_currentTrack)
        return;

    SimpleVector<QGraphicsItem*> allItems(items());
    for (int i = 0; i < allItems.size(); ++i)
    {
        if (static_cast<Note*>(allItems[i])->track() == m_currentTrack)
        {
            allItems[i]->setAcceptedMouseButtons(0);
            allItems[i]->setAcceptHoverEvents(false);
            allItems[i]->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }
        else if (static_cast<Note*>(allItems[i])->track() == track)
        {
            allItems[i]->setAcceptedMouseButtons(Qt::LeftButton);
            allItems[i]->setAcceptHoverEvents(true);
            allItems[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);
        }
    }

    m_currentTrack = track;
    update();
}

void SequencerScene::updateNoteBrushColors()
{
    static const double oneOverOneTwentySeven = 1. / 127.;
    const QColor &minVelColor = activeNoteBrushes[0].color();
    const QColor &maxVelColor = activeNoteBrushes[127].color();

    for (int i = 0; i < 128; ++i)
    {
        double amtCol1 = i * oneOverOneTwentySeven;
        double amtCol2 = 1 - amtCol1;

        QColor color;
        color.setRgbF(maxVelColor.redF() * amtCol1 + minVelColor.redF() * amtCol2,
                      maxVelColor.greenF() * amtCol1 + minVelColor.greenF() * amtCol2,
                      maxVelColor.blueF() * amtCol1 + minVelColor.blueF() * amtCol2);

        activeNoteBrushes[i].setColor(color);

        color.setAlpha(inactiveNoteBrushOpacity);
        inactiveNoteBrushes[i].setColor(color);
    }

    update();
}

void SequencerScene::updateNotePositions()
{
    SimpleVector<QGraphicsItem*> allNotes(items());
    for (int i = 0; i < allNotes.size(); ++i)
    {
        allNotes[i]->setY(latticeData->buttonPositions[static_cast<Note*>(allNotes[i])->getLaneIndex()].y());
    }
}
