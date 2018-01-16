#include "midieventplayer.h"
#include "hexsettings.h"
#include "midiportmanager.h"

#ifdef Q_OS_WIN32
#include <windows.h> // contains windows Sleep function
#endif

#ifdef Q_OS_MAC
#include <unistd.h> // contains Mac usleep function
#endif
#include <functional>
MIDIEventPlayer::MIDIEventPlayer(MIDIPortManager *portManager)
    : midiPortManager(portManager),
      playing(false),
      tickPosition(0),
      sleepIntervalCounter(0),
      ticksPerSleepInterval(100),
      loopEnabled(false),
      betaOutputEnabled(true),
      heldNotes(HexSettings::maxPolyphony)
{
}

void MIDIEventPlayer::allNotesOff()
{
    for (int i = 0; i < heldNotes.size(); ++i)
    {
        midiPortManager->sendMessage(heldNotes[i].first.first, heldNotes[i].first.second, 0, heldNotes[i].second);
    }

    heldNotes.setSize(0);
}

void MIDIEventPlayer::play()
{
    while (playing)
    {
        // check if any events need to be played
        while (currentEventIndex < events.size() && events[currentEventIndex].ticks <= tickPosition)
        {
            switch (events[currentEventIndex].type)
            {
            case SequencerEvent::Generator:
                if (betaOutputEnabled)
                {
                    midiPortManager->sendGeneratorMessages(events[currentEventIndex].value);

                    // this will cause betaChanged() to be emitted below (we don't want to
                    // emit the signal every time, only when there is a GUI update
                    betaHasChangedSinceLastGUIUpdate = true;
                    lastBetaValue = events[currentEventIndex].value;
                }
                break;
            case SequencerEvent::Period:
                midiPortManager->sendPeriodMessages(events[currentEventIndex].value);
                break;
            case SequencerEvent::Harmonicity:
            case SequencerEvent::JI:
                midiPortManager->sendDTMessage(events[currentEventIndex].midiData.byte1, events[currentEventIndex].midiData.byte2, events[currentEventIndex].midiData.byte3);
                break;
            case SequencerEvent::MIDICC:
                midiPortManager->sendMessage(events[currentEventIndex].midiData.byte1, events[currentEventIndex].midiData.byte2, events[currentEventIndex].midiData.byte3, events[currentEventIndex].midiData.track);
                break;
            case SequencerEvent::MIDINoteOn:
                midiPortManager->sendMessage(events[currentEventIndex].midiData.byte1, events[currentEventIndex].midiData.byte2, events[currentEventIndex].midiData.byte3, events[currentEventIndex].midiData.track);
                heldNotes.appendSafely(std::make_pair(std::make_pair(events[currentEventIndex].midiData.byte1, events[currentEventIndex].midiData.byte2), events[currentEventIndex].midiData.track));
                break;
            case SequencerEvent::MIDINoteOff:
                midiPortManager->sendMessage(events[currentEventIndex].midiData.byte1, events[currentEventIndex].midiData.byte2, events[currentEventIndex].midiData.byte3, events[currentEventIndex].midiData.track);
                heldNotes.removeValue(std::make_pair(std::make_pair(events[currentEventIndex].midiData.byte1, events[currentEventIndex].midiData.byte2), events[currentEventIndex].midiData.track));
                break;
            default: // shouldn't be any others...
                break;
            }

            ++currentEventIndex;
        }

        // check if it's time to update the GUI
        if (sleepIntervalCounter >= HexSettings::numSleepIntervalsBetweenGUIUpdates)
        {
            if (betaHasChangedSinceLastGUIUpdate)
            {
                emit betaChanged(lastBetaValue);
                betaHasChangedSinceLastGUIUpdate = false;
            }

            emit tickPositionChanged(tickPosition);
            sleepIntervalCounter = 0;
        }
        else
        {
            ++sleepIntervalCounter;
        }

        // check if it's time to notify of a beat or measure change
        if (tickPosition > nextMeasureTime)
        {
            emit measureChanged();
            nextMeasureTime += measureLength;
            nextBeatTime += beatLength;
        }
        else if (tickPosition > nextBeatTime)
        {
            emit beatChanged();
            nextBeatTime += beatLength;
        }

#ifdef Q_OS_WIN32
        Sleep(HexSettings::sleepIntervalMilliseconds);
#endif

#ifdef Q_OS_MAC
        usleep(HexSettings::sleepIntervalMicroseconds);
#endif

        tickPosition += ticksPerSleepInterval;

        // check if the loop end has been reached
        if (loopEnabled && playWasStartedBeforeLoopEnd && tickPosition > loopEnd)
            setTickPosition(loopStart);
    }
}

void MIDIEventPlayer::playPreviousEnvelopeEvents()
{
    bool periodSent = false;
    bool generatorSent = false;

    SimpleVector<unsigned char> MIDICCEnvelopes(20);
    for (int i = currentEventIndex - 1; i > -1; --i)
    {
        switch (events[i].type)
        {
        case SequencerEvent::Period:
            if (!periodSent)
            {
                midiPortManager->sendPeriodMessages(events[i].value);
                periodSent = true;
            }
            break;
        case SequencerEvent::Generator:
            if (!generatorSent)
            {
                midiPortManager->sendGeneratorMessages(events[i].value);
                emit betaChanged(events[i].value);
                generatorSent = true;
            }
            break;
        case SequencerEvent::MIDICC:
            if (!MIDICCEnvelopes.contains(events[i].midiData.byte2))
            {
                midiPortManager->sendMessage(events[i].midiData.byte1, events[i].midiData.byte2, events[i].midiData.byte3, events[i].midiData.track);
                MIDICCEnvelopes.appendSafely(events[i].midiData.byte2);
            }
            break;
        default: // ignore note events, etc.
            break;
        }
    }
}

void MIDIEventPlayer::recalculateCurrentEventIndices()
{
    currentEventIndex = 0;
    if (events.size() > 0)
    {
        while (events[currentEventIndex].ticks < tickPosition)
            ++currentEventIndex;
    }

    // calculate the metronome position
    nextBeatTime = beatLength * (static_cast<int>(tickPosition / beatLength));
    nextMeasureTime = measureLength * (static_cast<int>(tickPosition / measureLength));
    if (nextBeatTime < tickPosition) {nextBeatTime += beatLength;}
    if (nextMeasureTime < tickPosition) {nextMeasureTime += measureLength;}

    // calculate whether or not the loop should be active
    playWasStartedBeforeLoopEnd = (tickPosition < loopEnd);
}

void MIDIEventPlayer::setBeatAndMeasureLength(double beatLength, double measureLength)
{
    this->beatLength = beatLength;
    this->measureLength = measureLength;
}

void MIDIEventPlayer::setLoopBounds(double start, double end)
{
    loopEnabled = true;
    loopStart = start;
    loopEnd = end;

    if (playing)
        recalculateCurrentEventIndices();
}

void MIDIEventPlayer::setEvents(const SimpleVector<SequencerEvent> &events)
{
    allNotesOff();
    this->events = events;
}

void MIDIEventPlayer::setTempo(double ticksPerMillisecond)
{
    ticksPerSleepInterval = HexSettings::sleepIntervalMilliseconds * ticksPerMillisecond;
}

void MIDIEventPlayer::setTickPosition(double position)
{
    tickPosition = position;

    if (playing)
    {
        allNotesOff();
        recalculateCurrentEventIndices();
    }
}

void MIDIEventPlayer::start()
{
#ifdef Q_OS_WIN32
    timeBeginPeriod(1);
#endif

    playing = true;
    betaOutputEnabled = true;
    betaHasChangedSinceLastGUIUpdate = false;
    sleepIntervalCounter = 0;

    recalculateCurrentEventIndices();

    playPreviousEnvelopeEvents();

    play(); // enter a recursive play loop
}

void MIDIEventPlayer::stop()
{
    if (!playing)
        return;

    playing = false;
    allNotesOff();

    if (betaHasChangedSinceLastGUIUpdate)
        emit betaChanged(lastBetaValue);

    emit tickPositionChanged(tickPosition);

    emit finished();

#ifdef Q_OS_WIN32
    timeEndPeriod(1);
#endif
}

