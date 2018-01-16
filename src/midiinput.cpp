#include "midiinput.h"
#include "midieventhandler.h"
#include "midiportmanager.h"

// MIDI callback function
void onMIDIInput(double, std::vector<unsigned char> *message, void *midiEventHandler)
{
    static_cast<MIDIEventHandler*>(midiEventHandler)->MIDIInEvent(message->at(0), message->at(1), message->at(2));
}

MIDIInput::MIDIInput(MIDIEventHandler *handler, MIDIPortManager *manager)
    : MIDIPort(new RtMidiIn, manager), midiEventHandler(handler)
{
    static_cast<RtMidiIn*>(rtMidi)->setCallback(onMIDIInput, midiEventHandler);
}

MIDIInput::~MIDIInput()
{
    delete static_cast<RtMidiIn*>(rtMidi);
}

bool MIDIInput::testPortNameForFeedback(const std::string &name) const
{
    return portManager->probeOutputsForPotentialFeedback(name);
}
