#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H
#include <QtWidgets/QUndoCommand>
#include "simplevector.h"

class QGraphicsItem;
class QGraphicsScene;

// ###########################################################################
// ###########################################################################

class AddRemoveNotesCommand : public QUndoCommand // abstract base class for adding or removing notes
{
protected:
    AddRemoveNotesCommand(const SimpleVector<QGraphicsItem*> &notes, QGraphicsScene *scene);
    void addTheNotes();
    void deleteTheNotes();
    void removeTheNotes();

private:
    SimpleVector<QGraphicsItem*> notes;
    QGraphicsScene *scene;
};

class AddNotesCommand : public AddRemoveNotesCommand
{
public:
    AddNotesCommand(const SimpleVector<QGraphicsItem*> &notes, QGraphicsScene *scene);
    ~AddNotesCommand() {deleteTheNotes();}
    void redo() {addTheNotes();}
    void undo() {removeTheNotes();}
};

class DeleteNotesCommand : public AddRemoveNotesCommand
{
public:
    DeleteNotesCommand(const SimpleVector<QGraphicsItem*> &notes);
    void redo() {removeTheNotes();}
    void undo() {addTheNotes();}
};

// ###########################################################################
// ###########################################################################

class ChangeNoteVelocitiesCommand : public QUndoCommand
{
public:
    ChangeNoteVelocitiesCommand(const SimpleVector<QGraphicsItem *> &notes, const SimpleVector<unsigned char> &initialVelocities);
    void undo();
    void redo();

private:
    SimpleVector<QGraphicsItem *> notesToChange;
    SimpleVector<unsigned char> oldVelocities;
    unsigned char newVelocity;
};

// ###########################################################################
// ###########################################################################

class MoveNotesCommand : public QUndoCommand
{
public:
    MoveNotesCommand(const SimpleVector<QGraphicsItem *> &notes,
                     const SimpleVector<float> &initialPositions,
                     unsigned short int initialNoteLaneIndex);
    void undo();
    void redo();

private:
    void moveTheNotes(const SimpleVector<float> &positions, unsigned short int noteLaneIndex);
    SimpleVector<QGraphicsItem*> notesToMove;
    SimpleVector<float> oldPositions;
    SimpleVector<float> newPositions;
    unsigned short int oldNoteLaneIndex, newNoteLaneIndex;
};

// ###########################################################################
// ###########################################################################

class MoveNotesHorizontallyCommand : public QUndoCommand
{
public:
    MoveNotesHorizontallyCommand(const SimpleVector<QGraphicsItem *> &notes, const SimpleVector<float> &initialPositions);
    void undo();
    void redo();

private:
    void moveTheNotes(const SimpleVector<float> &positions);

    SimpleVector<QGraphicsItem*> notesToMove;
    SimpleVector<float> oldPositions;
    SimpleVector<float> newPositions;
};

// ###########################################################################
// ###########################################################################

class ResizeNotesCommand : public QUndoCommand
{
public:
    ResizeNotesCommand(const SimpleVector<QGraphicsItem *> &notes, const SimpleVector<float> &initialLengths);
    void undo();
    void redo();

private:
    void resizeTheNotes(const SimpleVector<float> &lengths);

    SimpleVector<QGraphicsItem*> notesToResize;
    SimpleVector<float> oldLengths;
    SimpleVector<float> newLengths;
};

#endif
