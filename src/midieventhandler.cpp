#include "midieventhandler.h"
#include "latticemanager.h"
#include "midiportmanager.h"
#include "sequencercommands.h"
#include "sequencerscene.h"
#include <QtCore/QCoreApplication>
#include <QtGui/QKeyEvent>

// ===========================================================================
/* MIDI data comes in a different thread. This isn't normally a problem, except
 * that while recording, when a note off comes in, the relevant note is added
 * to the sequencer. The problem is that you can only add items to a
 * QGraphicsScene from the main thread. So, when a note off is received while
 * recording, the note isn't added to the scene directly, but instead an event
 * is posted so that the main thread can handle it when it's ready. */
class DoneRecordingNoteEvent : public QEvent
{
public:
    DoneRecordingNoteEvent(int indexOfHeldNote)
        : QEvent(HexSettings::doneRecordingNoteEventType), indexOfHeldNote(indexOfHeldNote) {}
    const int indexOfHeldNote;
};
// ===========================================================================

MIDIEventHandler::MIDIEventHandler(LatticeManager *manager, SequencerScene *scene, MIDIPortManager *portManager)
    : latticeManager(manager), sequencerScene(scene), midiPortManager(portManager), recording(false), midiInputType(Standard), heldNotes(HexSettings::maxPolyphony)
{
}

void MIDIEventHandler::customEvent(QEvent *event)
{
    if (event->type() != HexSettings::doneRecordingNoteEventType) // see note above
        return;

    recordedNotes.appendSafely(sequencerScene->addNote(heldNotes[static_cast<DoneRecordingNoteEvent*>(event)->indexOfHeldNote]));
    heldNotes.removeIndex(static_cast<DoneRecordingNoteEvent*>(event)->indexOfHeldNote);
}

void MIDIEventHandler::MIDIInEvent(unsigned char a, unsigned char b, unsigned char c)
{
    // if not a note message, pass through
    if (a < 128 || a > 159)
    {
        midiPortManager->sendMessage(a, b, c, midiPortManager->getCurrentTrack());
        return;
    }

    short int j, k;
    switch (midiInputType)
    {
    case Standard: DynamicTonality::captureJKFromPianoMIDI(b, j, k); break;
    case AXiS: DynamicTonality::captureJKFromAxis(b, j, k); break;
    case Relayer: DynamicTonality::captureJKFromRelayer(a - 144, b, j, k);
    }

    latticeManager->getDT()->convertWickiJKToLayoutJK(j, k);

    if (a > 143 && c > 0)
        sendAndRecordNoteOn(j, k, c);
    else
        sendAndRecordNoteOff(j, k);
}

void MIDIEventHandler::onKeyEvent(QKeyEvent *event)
{
    short int j = 0, k = 0;

    if (!HexSettings::convertQWERTYKeyToWickiJK(event->key(), j, k))
    {
        event->ignore();
        return;
    }

    latticeManager->getDT()->convertWickiJKToLayoutJK(j, k);

    if (event->type() == QKeyEvent::KeyPress)
        sendAndRecordNoteOn(j, k, 90);
    else
        sendAndRecordNoteOff(j, k);
}

void MIDIEventHandler::sendAndRecordNoteOff(short j, short k)
{
    latticeManager->setButtonPressed(HexSettings::convertJKToNoteLaneIndex(j, k), false);

    // send the midi message
    unsigned char channel, pitch;
    if (!DynamicTonality::captureMIDIFromJK(j, k, channel, pitch))
        return;
    midiPortManager->sendMessage(channel + 143, pitch, 0, midiPortManager->getCurrentTrack()); // note on with velocity = 0

    if (!recording)
        return;

    // look for matching note on and remove it
    for (unsigned char i = 0; i < HexSettings::maxPolyphony; ++i)
    {
        if (heldNotes[i].j == j && heldNotes[i].k == k)
        {
            heldNotes[i].duration = sequencerScene->cursorPos() - heldNotes[i].startPosition;
            QCoreApplication::postEvent(this, new DoneRecordingNoteEvent(i)); // this adds the note to the scene (see note above)
            return;
        }
    }
}

void MIDIEventHandler::sendAndRecordNoteOn(short j, short k, unsigned char velocity)
{
    if (static_cast<unsigned int>(heldNotes.size()) >= HexSettings::maxPolyphony) // if note stack is full
        return;

    latticeManager->setButtonPressed(HexSettings::convertJKToNoteLaneIndex(j, k), true);

    // send the MIDI message
    unsigned char channel, pitch;
    if (!DynamicTonality::captureMIDIFromJK(j, k, channel, pitch))
        return;

    midiPortManager->sendMessage(channel + 143, pitch, velocity, midiPortManager->getCurrentTrack()); // send note on

    if (!recording)
        return;

    // store the note
    NoteStruct note = {midiPortManager->getCurrentTrack(), sequencerScene->cursorPos(), 0, velocity, j, k};
    heldNotes.append(note);
}

void MIDIEventHandler::startRecording(SequencerScene *scene)
{
    sequencerScene = scene;
    recording = true;
    recordedNotes = SimpleVector<QGraphicsItem*>(200);
}

void MIDIEventHandler::stopRecording()
{
    recording = false;
    if (recordedNotes.size() > 0)
    {
        AddNotesCommand *addNotesCommand = new AddNotesCommand(recordedNotes, sequencerScene);
        addNotesCommand->setText(QObject::tr("Record Notes"));
        sequencerScene->pushUndoCommand(addNotesCommand);
    }

    recordedNotes = SimpleVector<QGraphicsItem*>();
    sequencerScene = NULL;
}
