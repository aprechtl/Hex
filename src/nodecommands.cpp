#include "nodecommands.h"
#include "trackmanagerdialog.h"

AddRemoveNodesCommand::AddRemoveNodesCommand(int track, int envelope, const SimpleVector<QPointF> &nodes, TrackManagerDialog *dialog)
    : track(track), envelope(envelope), nodes(nodes), trackManager(dialog)
{}

void AddRemoveNodesCommand::addTheNodes()
{
    for (int i = 0; i < nodes.size(); ++i)
    {
        trackManager->addNode(track, envelope, nodes[i].x(), nodes[i].y());
    }
}

void AddRemoveNodesCommand::removeTheNodes()
{
    for (int i = 0; i < nodes.size(); ++i)
    {
        trackManager->removeNode(track, envelope, nodes[i].x());
    }
}

AddNodesCommand::AddNodesCommand(int track, int envelope, const SimpleVector<QPointF> &nodes, TrackManagerDialog *dialog)
    : AddRemoveNodesCommand(track, envelope, nodes, dialog)
{
    setText((nodes.size() == 1) ? QObject::tr("Add Node") : QObject::tr("Add Nodes"));
}

DeleteNodesCommand::DeleteNodesCommand(int track, int envelope, const SimpleVector<QPointF> &nodes, TrackManagerDialog *dialog)
    : AddRemoveNodesCommand(track, envelope, nodes, dialog)
{
    setText((nodes.size() == 1) ? QObject::tr("Delete Node") : QObject::tr("Delete Nodes"));
}

// ###########################################################################
// ###########################################################################

MoveNodesCommand::MoveNodesCommand(int track, int envelope,
                                   const SimpleVector<int> &indices,
                                   const SimpleVector<QPointF> &initialPositions,
                                   const SimpleVector<QPointF> &newPositions,
                                   TrackManagerDialog *dialog)
    : track(track),
      envelope(envelope),
      indices(indices),
      oldPositions(initialPositions),
      newPositions(newPositions),
      trackManager(dialog)
{
    setText((indices.size() == 1) ? QObject::tr("Move Node") : QObject::tr("Move Nodes"));
}

void MoveNodesCommand::moveTheNodes(const SimpleVector<QPointF> &positions)
{
    for (int i = 0; i < indices.size(); ++i)
    {
        trackManager->moveNode(track, envelope, indices[i], positions[i].x(), positions[i].y());
    }
}
