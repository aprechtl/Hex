#ifndef TRACKCOMMANDS_H
#define TRACKCOMMANDS_H
#include <QtWidgets/QUndoCommand>

class TrackManagerDialog;

class AddNewTrackCommand : public QUndoCommand
{
public:
    AddNewTrackCommand(TrackManagerDialog *dialog);
    void redo();
    void undo();

private:
    int track;
    int id;
    TrackManagerDialog *trackManager;
};

// ###########################################################################
// ###########################################################################

class ChangeTrackOrderCommand : public QUndoCommand
{
public:
    ChangeTrackOrderCommand(int oldIndex, int newIndex, TrackManagerDialog *dialog);
    void redo();
    void undo();

private:
    int oldIndex;
    int newIndex;
    TrackManagerDialog *trackManager;
    bool isFirstRedo;
};

// ###########################################################################
// ###########################################################################

class RemoveTrackCommand : public QUndoCommand
{
public:
    RemoveTrackCommand(int track, TrackManagerDialog *dialog);
    void redo();
    void undo();

private:
    int track;
    int id;
    TrackManagerDialog *trackManager;
};

#endif // TRACKCOMMANDS_H
