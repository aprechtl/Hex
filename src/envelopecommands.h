#ifndef ENVELOPECOMMANDS_H
#define ENVELOPECOMMANDS_H
#include <QtWidgets/QUndoCommand>
#include "envelopedata.h"

class TrackManagerDialog;

class EnvelopeCommand : public QUndoCommand
{
protected:
    EnvelopeCommand(TrackManagerDialog *trackManager, int track, int index)
        : trackManager(trackManager), track(track), index(index)
    {}

    TrackManagerDialog *trackManager;
    const int track;
    const int index;
};

class AddEnvelopeCommand : public EnvelopeCommand
{
public:
    AddEnvelopeCommand(TrackManagerDialog *dialog, int track, int index, const EnvelopeData &data);
    void undo();
    void redo();

private:
    const EnvelopeData data;
};

// ###########################################################################
// ###########################################################################

class ChangeEnvelopeCommand : public EnvelopeCommand
{
public:
    ChangeEnvelopeCommand(TrackManagerDialog *dialog, int track, int index,
                          unsigned char oldChannel, unsigned char newChannel, unsigned char oldNumber, unsigned char newNumber);
    void redo() {changeTheEnvelope(newChannel, newNumber);}
    void undo() {changeTheEnvelope(oldChannel, oldNumber);}

private:
    void changeTheEnvelope(unsigned char channel, unsigned char number);

    const unsigned char oldChannel, newChannel;
    const unsigned char oldNumber, newNumber;
};

// ###########################################################################
// ###########################################################################

class ChangeEnvelopeOrderCommand : public EnvelopeCommand
{
public:
    ChangeEnvelopeOrderCommand(TrackManagerDialog *dialog, int track, int oldIndex, int newIndex);
    void redo() {changeTheOrder(index, newIndex);}
    void undo() {changeTheOrder(newIndex, index);}

private:
    void changeTheOrder(unsigned char prevIndex, unsigned char nextIndex);

    const int newIndex;
};

// ###########################################################################
// ###########################################################################

class RemoveEnvelopeCommand : public EnvelopeCommand
{
public:
    RemoveEnvelopeCommand(TrackManagerDialog *dialog, int track, int index);
    void undo();
    void redo();

private:
    const EnvelopeData data;
};

#endif
