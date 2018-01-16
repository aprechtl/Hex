#include "sequencercommands.h"
#include "note.h"
#include "sequencerscene.h"

AddRemoveNotesCommand::AddRemoveNotesCommand(const SimpleVector<QGraphicsItem *> &notes, QGraphicsScene *scene)
    : notes(notes), scene(scene)
{
}

void AddRemoveNotesCommand::addTheNotes()
{
    if (notes[0]->scene() == scene) // prevents annoying warning message if items are already in the scene
        return;

    for (int i = 0; i < notes.size(); ++i)
    {
        scene->addItem(notes[i]);
    }
}

void AddRemoveNotesCommand::deleteTheNotes()
{
    if (notes[0]->scene() != NULL)
        return;

    for (int i = 0; i < notes.size(); ++i)
    {
        delete notes[i];
    }
}

void AddRemoveNotesCommand::removeTheNotes()
{
    for (int i = 0; i < notes.size(); ++i)
    {
        scene->removeItem(notes[i]);
    }
}

AddNotesCommand::AddNotesCommand(const SimpleVector<QGraphicsItem *> &notes, QGraphicsScene *scene)
    : AddRemoveNotesCommand(notes, scene)
{
    setText((notes.size() == 1) ? QObject::tr("Add Note") : QObject::tr("Add Notes"));
}

DeleteNotesCommand::DeleteNotesCommand(const SimpleVector<QGraphicsItem *> &notes)
    : AddRemoveNotesCommand(notes, notes[0]->scene())
{
    setText((notes.size() == 1) ? QObject::tr("Delete Note") : QObject::tr("Delete Notes"));
}

// ###########################################################################
// ###########################################################################

ChangeNoteVelocitiesCommand::ChangeNoteVelocitiesCommand(const SimpleVector<QGraphicsItem *> &notes,
                                                         const SimpleVector<unsigned char> &initialVelocities)
    : notesToChange(notes),
      oldVelocities(initialVelocities),
      newVelocity(static_cast<Note*>(notesToChange[0])->getVelocity())
{
    setText((notesToChange.size() == 1) ? "Change Note Velocity" : "Change Note Velocities");
}

void ChangeNoteVelocitiesCommand::undo()
{
    for (int i = 0; i < notesToChange.size(); ++i)
    {
        static_cast<Note*>(notesToChange[i])->setVelocity(oldVelocities[i]);
    }
}

void ChangeNoteVelocitiesCommand::redo()
{
    for (int i = 0; i < notesToChange.size(); ++i)
    {
        static_cast<Note*>(notesToChange[i])->setVelocity(newVelocity);
    }
}

// ###########################################################################
// ###########################################################################

MoveNotesCommand::MoveNotesCommand(const SimpleVector<QGraphicsItem*> &notes,
                                   const SimpleVector<float> &initialPositions,
                                   unsigned short int initialNoteLaneIndex)
    : notesToMove(notes),
      oldPositions(initialPositions),
      newPositions(notesToMove.size()),
      oldNoteLaneIndex(initialNoteLaneIndex),
      newNoteLaneIndex(static_cast<Note*>(notes[0])->getLaneIndex())
{
    setText((notesToMove.size() == 1) ? "Move Note" : "Move Notes");

    for (int i = 0; i < notesToMove.size(); ++i)
    {
        newPositions.appendByValue(notesToMove[i]->x());
    }
}

void MoveNotesCommand::undo() {moveTheNotes(oldPositions, oldNoteLaneIndex);}
void MoveNotesCommand::redo() {moveTheNotes(newPositions, newNoteLaneIndex);}

void MoveNotesCommand::moveTheNotes(const SimpleVector<float> &positions, unsigned short int noteLaneIndex)
{
    for (int i = 0; i < notesToMove.size(); ++i)
    {
        notesToMove[i]->setX(positions[i]);
        static_cast<Note*>(notesToMove[i])->setLaneIndex(noteLaneIndex);
    }

    static_cast<SequencerScene*>(notesToMove[0]->scene())->updateNotePositions();
}

// ###########################################################################
// ###########################################################################

MoveNotesHorizontallyCommand::MoveNotesHorizontallyCommand(const SimpleVector<QGraphicsItem*> &notes, const SimpleVector<float> &initialPositions)
    : notesToMove(notes), oldPositions(initialPositions), newPositions(notesToMove.size())
{
    setText((notesToMove.size() == 1) ? "Move Note" : "Move Notes");

    for (int i = 0; i < notesToMove.size(); ++i)
    {
        newPositions.appendByValue(notesToMove[i]->pos().x());
    }
}

void MoveNotesHorizontallyCommand::undo() {moveTheNotes(oldPositions);}
void MoveNotesHorizontallyCommand::redo() {moveTheNotes(newPositions);}

void MoveNotesHorizontallyCommand::moveTheNotes(const SimpleVector<float> &positions)
{
    for (int i = 0; i < notesToMove.size(); ++i)
    {
        notesToMove[i]->setX(positions[i]);
    }
}

// ###########################################################################
// ###########################################################################

ResizeNotesCommand::ResizeNotesCommand(const SimpleVector<QGraphicsItem *> &notes, const SimpleVector<float> &initialLengths)
    : notesToResize(notes), oldLengths(initialLengths), newLengths(notesToResize.size())
{
    setText((notesToResize.size() == 1) ? "Resize Note" : "Resize Notes");

    for (int i = 0; i < notesToResize.size(); ++i)
    {
        newLengths.appendByValue(notesToResize.at(i)->boundingRect().width());
    }
}

void ResizeNotesCommand::undo() {resizeTheNotes(oldLengths);}
void ResizeNotesCommand::redo() {resizeTheNotes(newLengths);}

void ResizeNotesCommand::resizeTheNotes(const SimpleVector<float> &lengths)
{
    for (int i = 0; i < notesToResize.size(); ++i)
    {
        static_cast<Note *>(notesToResize[i])->setWidth(lengths[i]);
    }
}
