#include "midioutput.h"
#include "midiportmanager.h"

MIDIOutput::MIDIOutput(MIDIPortManager *manager)
    : MIDIPort(new RtMidiOut, manager), sendsDTMessages(true)
{
    // reserve memory for the vectors
    midi2.assign(2, 0);
    midi3.assign(3, 0);
}

MIDIOutput::~MIDIOutput()
{
    delete static_cast<RtMidiOut*>(rtMidi);
}

bool MIDIOutput::testPortNameForFeedback(const std::string &name) const
{
    return portManager->probeInputsForPotentialFeedback(name);
}
