#ifndef ENVELOPECOMMANDS_H
#define ENVELOPECOMMANDS_H
#include <QtWidgets/QUndoCommand>
#include "simplevector.h"
#include <QtCore/QPointF>

class TrackManagerDialog;

class AddRemoveNodesCommand : public QUndoCommand
{
protected:
    AddRemoveNodesCommand(int track, int envelope, const SimpleVector<QPointF> &nodes, TrackManagerDialog *dialog);
    void addTheNodes();
    void removeTheNodes();

private:
    int track, envelope;
    SimpleVector<QPointF> nodes;
    TrackManagerDialog *trackManager;
};

class AddNodesCommand : public AddRemoveNodesCommand
{
public:
    AddNodesCommand(int track, int envelope, const SimpleVector<QPointF> &nodes, TrackManagerDialog *dialog);
    void redo() {addTheNodes();}
    void undo() {removeTheNodes();}
};

class DeleteNodesCommand : public AddRemoveNodesCommand
{
public:
    DeleteNodesCommand(int track, int envelope, const SimpleVector<QPointF> &nodes, TrackManagerDialog *dialog);
    void redo() {removeTheNodes();}
    void undo() {addTheNodes();}
};

// ###########################################################################
// ###########################################################################

class MoveNodesCommand : public QUndoCommand
{
public:
    MoveNodesCommand(int track, int envelope,
                     const SimpleVector<int> &indices,
                     const SimpleVector<QPointF> &oldPositions,
                     const SimpleVector<QPointF> &newPositions,
                     TrackManagerDialog *dialog);
    void redo() {moveTheNodes(newPositions);}
    void undo() {moveTheNodes(oldPositions);}

private:
    void moveTheNodes(const SimpleVector<QPointF> &positions);

    const int track, envelope;
    const SimpleVector<int> indices;
    const SimpleVector<QPointF> oldPositions;
    const SimpleVector<QPointF> newPositions;
    TrackManagerDialog *trackManager;
};

#endif
