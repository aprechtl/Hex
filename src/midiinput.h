#ifndef MIDIINPUT_H
#define MIDIINPUT_H
#include "midiport.h"

class MIDIEventHandler;

class MIDIInput : public MIDIPort
{
    friend class MIDIPortManager;

private:
    MIDIInput(MIDIEventHandler *handler, MIDIPortManager *manager);
    ~MIDIInput();
    bool testPortNameForFeedback(const std::string &name) const;

    MIDIEventHandler *midiEventHandler;
};

#endif
