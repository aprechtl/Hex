#ifndef MIDIPORT_H
#define MIDIPORT_H
#include <QtCore/QObject>
#include "simplevector.h"
#include <string>

class MIDIPortManager;
class QMenu;
class RtMidi;

class MIDIPort
{
    friend class MIDIPortManager;

public:
    void closePort();
    void openPort(int number);
    void refreshMenuActions();

    // inline methods
    std::string currentPortName() const {return portName;}
    int currentPortNumber() const {return portNumber;}
    bool isOpen() const {return portIsOpen;}
    QMenu *menu() const {return portMenu;}
    void openLastPort() {openPort(portNumber);}

protected:
    MIDIPort(RtMidi *rtMidi, MIDIPortManager *checker);
    virtual ~MIDIPort();

    RtMidi *rtMidi;
    MIDIPortManager *portManager;

private:
    virtual bool testPortNameForFeedback(const std::string &name) const = 0;

    QMenu *portMenu;
    bool portIsOpen;
    std::string portName;
    int portNumber;
    int lastPortNumber;
};

#endif // MIDIPORT_H
