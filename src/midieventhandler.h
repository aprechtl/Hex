#ifndef MIDIEVENTHANDLER_H
#define MIDIEVENTHANDLER_H
#include "notestruct.h"
#include "simplevector.h"
#include <QtCore/QObject>
#include <QtWidgets/QGraphicsItem>

class QKeyEvent;
class LatticeManager;
class MIDIPortManager;
class SequencerScene;

class MIDIEventHandler : public QObject
{
public:
    enum MIDIInputType {Standard = 0, AXiS = 1, Relayer = 2};

    MIDIEventHandler(LatticeManager *manager, SequencerScene *scene, MIDIPortManager *portManager);
    void MIDIInEvent(unsigned char a, unsigned char b, unsigned char c);
    void onKeyEvent(QKeyEvent *event);
    void sendAndRecordNoteOff(short j, short k);
    void sendAndRecordNoteOn(short j, short k, unsigned char velocity);
    void startRecording(SequencerScene *scene);
    void stopRecording();

    // inline methods
    MIDIInputType getMIDIInputType() const {return midiInputType;}
    bool isRecording() const {return recording;}
    void setMIDIInputType(MIDIInputType type) {midiInputType = type;}

protected:
    void customEvent(QEvent *event); // for MIDI events

private:
    // these are initialized in the initializer list
    LatticeManager *latticeManager;
    SequencerScene *sequencerScene;
    MIDIPortManager *midiPortManager;
    bool recording;
    MIDIInputType midiInputType;

    SimpleVector<NoteStruct> heldNotes;
    SimpleVector<QGraphicsItem*> recordedNotes;
};

#endif
