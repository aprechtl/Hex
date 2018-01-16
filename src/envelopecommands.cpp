#include "envelopecommands.h"
#include "trackmanagerdialog.h"

AddEnvelopeCommand::AddEnvelopeCommand(TrackManagerDialog *dialog, int track, int index, const EnvelopeData &data)
    : EnvelopeCommand(dialog, track, index), data(data)
{
    setText("Add Envelope");
}

void AddEnvelopeCommand::redo()
{
    trackManager->addMIDICCEnvelope(track, index, data);
}

void AddEnvelopeCommand::undo()
{
    trackManager->removeMIDICCEnvelope(track, index);
}

// ###########################################################################
// ###########################################################################

ChangeEnvelopeCommand::ChangeEnvelopeCommand(TrackManagerDialog *dialog, int track, int index,
                                             unsigned char oldChannel,
                                             unsigned char newChannel,
                                             unsigned char oldNumber,
                                             unsigned char newNumber)
    : EnvelopeCommand(dialog, track, index), oldChannel(oldChannel), newChannel(newChannel), oldNumber(oldNumber), newNumber(newNumber)
{
    setText(QObject::tr("Set Up Envelope"));
}

void ChangeEnvelopeCommand::changeTheEnvelope(unsigned char channel, unsigned char number)
{
    trackManager->changeMIDICCEnvelopeData(track, index, channel, number);
}

// ###########################################################################
// ###########################################################################

ChangeEnvelopeOrderCommand::ChangeEnvelopeOrderCommand(TrackManagerDialog *dialog, int track, int oldIndex, int newIndex)
    : EnvelopeCommand(dialog, track, oldIndex), newIndex(newIndex)
{
    setText(QObject::tr("Reorder Envelopes"));
}

void ChangeEnvelopeOrderCommand::changeTheOrder(unsigned char prevIndex, unsigned char nextIndex)
{
    trackManager->moveMIDICCEnvelope(track, prevIndex, nextIndex);
}

// ###########################################################################
// ###########################################################################

RemoveEnvelopeCommand::RemoveEnvelopeCommand(TrackManagerDialog *dialog, int track, int index)
    : EnvelopeCommand(dialog, track, index), data(dialog->getMIDICCEnvelope(track, index))
{
    setText(QObject::tr("Remove Envelope"));
}

void RemoveEnvelopeCommand::undo()
{
    trackManager->addMIDICCEnvelope(track, index, data);
}

void RemoveEnvelopeCommand::redo()
{
    trackManager->removeMIDICCEnvelope(track, index);
}
