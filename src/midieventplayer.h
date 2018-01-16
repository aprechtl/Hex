#ifndef MIDIEVENTPLAYER_H
#define MIDIEVENTPLAYER_H
#include <QtCore/QObject>
#include "sequencerevent.h"
#include "simplevector.h"


class MIDIPortManager;

class MIDIEventPlayer : public QObject
{
    Q_OBJECT

public:
    MIDIEventPlayer(MIDIPortManager *portManager);
    void setBeatAndMeasureLength(double beatLength, double measureLength);
    void setEvents(const SimpleVector<SequencerEvent> &events);
    void setLoopBounds(double start, double end);
    void setTempo(double ticksPerMillisecond);
    void setTickPosition(double position);
    void start();
    void stop();

    // inline
    void disableLoop() {loopEnabled = false;}
    void temporarilyDisableBetaOutput() {betaOutputEnabled = false;}

signals:
    void beatChanged();
    void betaChanged(double beta);
    void finished();
    void measureChanged();
    void tickPositionChanged(double ticks);

private:
    void allNotesOff();
    void play();
    void playPreviousEnvelopeEvents();
    void recalculateCurrentEventIndices();

    // these are initialized in the initializer list
    MIDIPortManager *midiPortManager;
    bool playing;
    double tickPosition;
    int sleepIntervalCounter;
    double ticksPerSleepInterval;
    bool loopEnabled;
    bool betaOutputEnabled;
    SimpleVector<std::pair<std::pair<unsigned char, unsigned char>, unsigned char> > heldNotes; // note/channel paired with track

    SimpleVector<SequencerEvent> events;
    int currentEventIndex;
    bool playWasStartedBeforeLoopEnd;
    double loopStart;
    double loopEnd;
    double beatLength;
    double measureLength;
    double nextBeatTime;
    double nextMeasureTime;
    float lastBetaValue;
    bool betaHasChangedSinceLastGUIUpdate;
};

#endif
