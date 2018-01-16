#ifndef MIDIPORTMANAGER_H
#define MIDIPORTMANAGER_H
#include "simplevector.h"
#include "midiinput.h"
#include "midioutput.h"

class MIDIPortManager
{
public:
    MIDIPortManager();
    ~MIDIPortManager();
    MIDIInput *createInput(MIDIEventHandler *handler);
    MIDIOutput *createOutput(int port = -1);
    void destroyOutput(int index);
    void sendDTMessage(int byte1, int byte2, int byte3);
    void sendGeneratorMessages(double generator);
    void sendPeriodMessages(double period);

    // inline methods
    void clearInputs() {clearMIDIPorts(midiInputs);}
    void clearOutputs() {clearMIDIPorts(midiOutputs);}
    void destroyOutput(MIDIOutput *output) {destroyOutput(midiOutputs.find(output));}

    int getCurrentTrack() const {return currentTrack;}
    MIDIInput *input(int index) {return static_cast<MIDIInput*>(midiInputs[index]);}
    MIDIOutput *output(int index) {return static_cast<MIDIOutput*>(midiOutputs[index]);}
    bool probeInputsForPotentialFeedback(const std::string &outputPortName) {return probePortsForPotentialFeedback(outputPortName, midiInputs);}
    bool probeOutputsForPotentialFeedback(const std::string &inputPortName) {return probePortsForPotentialFeedback(inputPortName, midiOutputs);}
    void sendMessage(unsigned char a, unsigned char b, int track);
    void sendMessage(unsigned char a, unsigned char b, unsigned char c, int track);
    void setCurrentTrack(int track) {currentTrack = track;}

private:
    void clearMIDIPorts(SimpleVector<MIDIPort*> &ports);
    bool probePortsForPotentialFeedback(const std::string &portName, const SimpleVector<MIDIPort*> &portsToProbe);

    int currentTrack;
    SimpleVector<MIDIPort*> midiInputs;
    SimpleVector<MIDIPort*> midiOutputs;
};

inline void MIDIPortManager::sendMessage(unsigned char a, unsigned char b, int track)
{
    if (midiOutputs[track]->isOpen())
        static_cast<MIDIOutput*>(midiOutputs[track])->sendMessage(a, b);
}

inline void MIDIPortManager::sendMessage(unsigned char a, unsigned char b, unsigned char c, int track)
{
    if (midiOutputs[track]->isOpen())
        static_cast<MIDIOutput*>(midiOutputs[track])->sendMessage(a, b, c);
}

#endif // MIDIPORTMANAGER_H
