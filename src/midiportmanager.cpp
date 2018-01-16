#include "midiportmanager.h"

MIDIPortManager::MIDIPortManager()
    : currentTrack(0), midiInputs(1), midiOutputs(32) // max num ports
{
}

MIDIPortManager::~MIDIPortManager()
{
    clearInputs();
    clearOutputs();
}

void MIDIPortManager::clearMIDIPorts(SimpleVector<MIDIPort*> &ports)
{
    for (int i = 0; i < ports.size(); ++i)
        delete ports[i];

    ports.setSize(0);
}

MIDIInput *MIDIPortManager::createInput(MIDIEventHandler *handler)
{
    MIDIInput *newInput = new MIDIInput(handler, this);
    midiInputs.appendSafely(newInput);
    return newInput;
}

MIDIOutput *MIDIPortManager::createOutput(int port)
{
    MIDIOutput *newOutput = new MIDIOutput(this);
    newOutput->openPort(port);
    midiOutputs.appendSafely(newOutput);
    return newOutput;
}

void MIDIPortManager::destroyOutput(int index)
{
    delete midiOutputs[index];
    midiOutputs.removeIndex(index);
}

bool MIDIPortManager::probePortsForPotentialFeedback(const std::string &portName, const SimpleVector<MIDIPort*> &portsToProbe)
{
    // Feedback is tested for by comparing the last five characters of the port
    // names. If a match is found, feedback is assumed.

    QString testName(portName.c_str());
    testName.remove(0, testName.size() - 5);

    for (int i = 0; i < portsToProbe.size(); ++i)
    {
        if (!portsToProbe[i]->isOpen())
            continue;

        QString existingPortName(portsToProbe[i]->currentPortName().c_str());

        if (testName == existingPortName.remove(0, existingPortName.size() - 5))
            return true; // feedback likely
    }

    return false; // feedback unlikely
}

void MIDIPortManager::sendDTMessage(int byte1, int byte2, int byte3)
{
    for (int i = 0; i < midiOutputs.size(); ++i)
    {
        if (midiOutputs[i]->isOpen() && static_cast<MIDIOutput*>(midiOutputs[i])->sendsDTMessages)
            static_cast<MIDIOutput*>(midiOutputs[i])->sendMessage(byte1, byte2, byte3);
    }
}

void MIDIPortManager::sendGeneratorMessages(double generator)
{
    unsigned char cc53, cc54, cc55;
    DynamicTonality::captureMIDICCValues(generator, cc53, cc54, cc55);

    for (int i = 0; i < midiOutputs.size(); ++i)
    {
        if (midiOutputs[i]->isOpen() && static_cast<MIDIOutput*>(midiOutputs[i])->sendsDTMessages)
        {
            static_cast<MIDIOutput*>(midiOutputs[i])->sendMessage(176, 53, cc53);
            static_cast<MIDIOutput*>(midiOutputs[i])->sendMessage(176, 54, cc54);
            static_cast<MIDIOutput*>(midiOutputs[i])->sendMessage(176, 55, cc55);
        }
    }
}

void MIDIPortManager::sendPeriodMessages(double period)
{
    unsigned char cc50, cc51, cc52;
    DynamicTonality::captureMIDICCValues(period, cc50, cc51, cc52);

    for (int i = 0; i < midiOutputs.size(); ++i)
    {
        if (midiOutputs[i]->isOpen() && static_cast<MIDIOutput*>(midiOutputs[i])->sendsDTMessages)
        {
            static_cast<MIDIOutput*>(midiOutputs[i])->sendMessage(176, 50, cc50);
            static_cast<MIDIOutput*>(midiOutputs[i])->sendMessage(176, 51, cc51);
            static_cast<MIDIOutput*>(midiOutputs[i])->sendMessage(176, 52, cc52);
        }
    }
}
