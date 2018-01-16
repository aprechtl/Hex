#include "midiport.h"
#include "midiportmanager.h"
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>

MIDIPort::MIDIPort(RtMidi *rtMidi, MIDIPortManager *checker)
    : rtMidi(rtMidi), portManager(checker), portIsOpen(false), portNumber(-1)
{
    portMenu = new QMenu;
    QObject::connect(portMenu, &QMenu::aboutToShow, [=](){refreshMenuActions();});
    QObject::connect(portMenu, &QMenu::triggered, [=](QAction *action)
    {
        if (action->data().toInt() == currentPortNumber())
            closePort();
        else
            openPort(action->data().toInt());
    });
}

MIDIPort::~MIDIPort()
{
    delete portMenu;
}

void MIDIPort::closePort()
{
    rtMidi->closePort();
    portIsOpen = false;
    portName = std::string();
    portNumber = -1;
}

void MIDIPort::openPort(int number)
{
    if (number < 0 || number >= static_cast<int>(rtMidi->getPortCount()))
        return;

    if (testPortNameForFeedback(rtMidi->getPortName(number)))
    {
        QMessageBox::warning(portMenu->parentWidget(),
                             QObject::tr("Feedback"),
                             QObject::tr("A probable MIDI feedback loop has been detected. "
                                         "This occurs when a MIDI input and output point to the same port. "
                                         "You will probably need to choose another port."));
    }

    rtMidi->openPort(number);
    portIsOpen = true;
    portName = rtMidi->getPortName(number);
    portNumber = number;
    lastPortNumber = number;
}

void MIDIPort::refreshMenuActions()
{
    portMenu->clear();

    bool portNameAlreadyFound = false;

    for (unsigned int i = 0; i < rtMidi->getPortCount(); ++i)
    {
        QAction *action = new QAction(QString(rtMidi->getPortName(i).c_str()), portMenu);
        action->setData(i);

        if (portIsOpen && !portNameAlreadyFound && portName == rtMidi->getPortName(i))
        {
            action->setCheckable(true);
            action->setChecked(true);
            portNameAlreadyFound = true; // no need to compare the strings if matching port name already found
        }

        portMenu->addAction(action);
    }
}
