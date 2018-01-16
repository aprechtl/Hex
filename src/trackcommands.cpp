#include "trackcommands.h"
#include "trackmanagerdialog.h"

AddNewTrackCommand::AddNewTrackCommand(TrackManagerDialog *dialog)
    : track(dialog->currentTrack() + 1), id(-1), trackManager(dialog)
{
    setText(QObject::tr("Add New Track"));
}

void AddNewTrackCommand::redo()
{
    if (id == -1)
        trackManager->addNewTrack();
    else
        trackManager->restoreTrack(id, track);
}

void AddNewTrackCommand::undo()
{
    id = trackManager->removeTrack(track);
}

// ###########################################################################
// ###########################################################################

ChangeTrackOrderCommand::ChangeTrackOrderCommand(int oldIndex, int newIndex, TrackManagerDialog *dialog)
    : oldIndex(oldIndex), newIndex(newIndex), trackManager(dialog), isFirstRedo(true)
{}

void ChangeTrackOrderCommand::redo()
{
    // the isFirstRedo stuff is because this command is created by dragging/dropping the list widget item,
    // so it doesn't make sense for this method to move it on the first redo, since the item has already been moved
    trackManager->moveTrack(oldIndex, newIndex, !isFirstRedo);

    if (isFirstRedo)
        isFirstRedo = false;
}

void ChangeTrackOrderCommand::undo()
{
    trackManager->moveTrack(oldIndex, newIndex, true);
}

// ###########################################################################
// ###########################################################################

RemoveTrackCommand::RemoveTrackCommand(int track, TrackManagerDialog *dialog)
    : track(track), id(-1), trackManager(dialog)
{
    setText(QObject::tr("Remove Track"));
}

void RemoveTrackCommand::redo()
{
    id = trackManager->removeTrack(track);
}

void RemoveTrackCommand::undo()
{
    trackManager->restoreTrack(id, track);
}
