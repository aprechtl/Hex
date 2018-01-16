#include "midifilebuilder.h"
#include "dynamictonality.h"
#include "simplevector.h"
#include <QtCore/QDataStream>
#include <QtCore/QFile>

#define CONTROL_CHANGE 0xb0
#define END_OF_TRACK 0x2f
#define META_EVENT 0xff
#define NOTE_OFF 0x80
#define NOTE_ON 0x90
#define TEMPO 0x51
#define TIME_SIGNATURE 0x58

MIDIFileBuilder::MIDIFileBuilder()
    :
      m_dataStream(0),
      m_eventVectors(NULL),
      m_numBytesWritten(0),
      m_numTracks(0),
      m_previousStatus(0),
      m_tempoMicrosecsPerQuarterNote(1000000),
      m_ticksPerQuarterNote(480),
      m_timeSigDenom(2), // expressed as exponent of 2
      m_timeSigNumer(4)
{
}

MIDIFileBuilder::~MIDIFileBuilder()
{
    if (m_eventVectors != NULL)
        delete [] m_eventVectors;
}

void MIDIFileBuilder::setTimeSignature(int numerator, int denominator)
{
    m_timeSigNumer = numerator;

    // convert denominator to exponent of 2
    switch (denominator)
    {
        case 1: m_timeSigDenom = 0; break;
        case 2: m_timeSigDenom = 1; break;
        case 4: m_timeSigDenom = 2; break;
        case 8: m_timeSigDenom = 3; break;
        case 16: m_timeSigDenom = 4; break;
        case 32: m_timeSigDenom = 5; break;
    }
}

void MIDIFileBuilder::setTempo(double tempoBPM)
{
    m_tempoMicrosecsPerQuarterNote = 60000000 / tempoBPM;
}

void MIDIFileBuilder::setResolution(int ticksPerQuarterNote)
{
    m_ticksPerQuarterNote = ticksPerQuarterNote;
}

void MIDIFileBuilder::setEvents(const SimpleVector<SequencerEvent> &events)
{
    if (m_eventVectors != NULL)
        delete [] m_eventVectors;

    // ======================================================= FIND NUM TRACKS
    int highestTrack = 0;
    for (int i = 0; i < events.size(); ++i)
    {
        switch (events[i].type)
        {
        case SequencerEvent::MIDICC:
        case SequencerEvent::MIDINoteOn:
        case SequencerEvent::MIDINoteOff:
            if (events[i].midiData.track > highestTrack)
                highestTrack = events[i].midiData.track;
            break;
        default: // there shouldn't be any other types
            break;
        }
    }

    m_numTracks = highestTrack + 1;
    m_eventVectors = new std::vector<SequencerEvent>[m_numTracks];
    // =======================================================================

    // ================================ SEPARATE EVENTS INTO DIFFERENT VECTORS
    for (int i = 0; i < events.size(); ++i)
    {
        switch (events[i].type)
        {
        case SequencerEvent::Period:
        case SequencerEvent::Generator:
        case SequencerEvent::Harmonicity:
        case SequencerEvent::JI:
            for (int j = 0; j < m_numTracks; ++j) // the DT envelopes get sent to all tracks
            {
                m_eventVectors[j].push_back(events[i]);
            }
            break;
        case SequencerEvent::MIDICC:
        case SequencerEvent::MIDINoteOn:
        case SequencerEvent::MIDINoteOff:
        case SequencerEvent::MIDIOther:
            m_eventVectors[events[i].midiData.track].push_back(events[i]);
            break;
        default: break;
        }
    }
    // =======================================================================

    // ================================================ CONVERT TO DELTA TICKS
    for (int i = 0; i < m_numTracks; ++i)
    {
        unsigned int prevTicks = 0;
        for (unsigned int j = 0; j < m_eventVectors[i].size(); ++j)
        {
            unsigned int temp = m_eventVectors[i][j].ticks;
            m_eventVectors[i][j].ticks -= prevTicks;
            prevTicks = temp;
        }
    }
    // =======================================================================

}

void MIDIFileBuilder::writeToFile(const QString& fileName) // writes to file
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QDataStream ds(&file);
    m_dataStream = &ds;

    // write header
    m_previousStatus = 0;
    write4Bytes(0x4d546864); // SMF header prefix
    write4Bytes(6); // represents the 4-byte size of the header (this never changes in a SMF)
    write2Bytes(0); // 0 is format
    write2Bytes(m_numTracks);
    write2Bytes(m_ticksPerQuarterNote);

    // write tracks
    m_previousStatus = 0;
    for (int i = 0; i < m_numTracks; ++i)
    {
        writeTrack(i);
    }

    file.close();
}

void MIDIFileBuilder::writeTrack(int track)
{
    quint32 trackHeader;
    quint32 trackLength;
    qint64 offset;
    qint64 placeMarker;

    m_previousStatus = 0;
    trackHeader = 0x4d54726b; // track prefix
    trackLength = 0; // track length (will change later???)
    offset = m_dataStream->device()->pos();
    write4Bytes(trackHeader);
    write4Bytes(trackLength);
    m_numBytesWritten = 0;

    // ==================================================== WRITE TRACK EVENTS
    writeTempo(0, m_tempoMicrosecsPerQuarterNote);
    writeTimeSignature(0, m_timeSigNumer, m_timeSigDenom, 24, 8);

    for (unsigned int i = 0; i < m_eventVectors[track].size(); ++i)
    {
        switch (m_eventVectors[track][i].type)
        {
        case SequencerEvent::Period:
            unsigned char cc50, cc51, cc52;
            DynamicTonality::captureMIDICCValues(m_eventVectors[track][i].value, cc50, cc51, cc52);
            writeMidiEvent(m_eventVectors[track][i].ticks, CONTROL_CHANGE, 0, 50, cc50);
            writeMidiEvent(0,                              CONTROL_CHANGE, 0, 51, cc51);
            writeMidiEvent(0,                              CONTROL_CHANGE, 0, 52, cc52);
            break;
        case SequencerEvent::Generator:
            unsigned char cc20, cc21, cc22;
            DynamicTonality::captureMIDICCValues(m_eventVectors[track][i].value, cc20, cc21, cc22);
            writeMidiEvent(m_eventVectors[track][i].ticks, CONTROL_CHANGE, 0, 20, cc20);
            writeMidiEvent(0,                              CONTROL_CHANGE, 0, 21, cc21);
            writeMidiEvent(0,                              CONTROL_CHANGE, 0, 22, cc22);
            break;
        case SequencerEvent::MIDICC:
            writeMidiEvent(m_eventVectors[track][i].ticks, CONTROL_CHANGE, m_eventVectors[track][i].midiData.byte1 - 175, m_eventVectors[track][i].midiData.byte2, m_eventVectors[track][i].midiData.byte3);
            break;
        case SequencerEvent::MIDINoteOn:
            writeMidiEvent(m_eventVectors[track][i].ticks, NOTE_ON, m_eventVectors[track][i].midiData.byte1 - 143, m_eventVectors[track][i].midiData.byte2, m_eventVectors[track][i].midiData.byte3);
            break;
        case SequencerEvent::MIDINoteOff:
            writeMidiEvent(m_eventVectors[track][i].ticks, NOTE_ON, m_eventVectors[track][i].midiData.byte1 - 143, m_eventVectors[track][i].midiData.byte2, 0);
            break;
        default: // there shouldn't be any others
            break;
        }
    }

    writeMetaEvent(0, END_OF_TRACK);
    // =======================================================================

    placeMarker = m_dataStream->device()->pos();
    m_dataStream->device()->seek(offset);
    trackLength = m_numBytesWritten;
    write4Bytes(trackHeader);
    write4Bytes(trackLength);
    m_dataStream->device()->seek(placeMarker);
}

void MIDIFileBuilder::writeMetaEvent(long deltaTicks, int type, int data)
{
    writeVarLen(deltaTicks);
    writeByte(META_EVENT);
    writeByte(type);
    writeByte(1);
    writeByte(data);
    m_previousStatus = META_EVENT;
}

void MIDIFileBuilder::writeMetaEvent(long deltaTicks, int type)
{
    writeVarLen(deltaTicks);
    writeByte(META_EVENT);
    writeByte(type);
    writeByte(0);
    m_previousStatus = META_EVENT;
}

void MIDIFileBuilder::writeMidiEvent(long deltaTicks, int type, int chan, int b1)
{
    writeVarLen(deltaTicks);
    quint8 c = type | chan;
    if (m_previousStatus != c) // check if we can omit status byte ("running status")
    {
        m_previousStatus = c;
        writeByte(c);
    }
    writeByte(b1);
}

void MIDIFileBuilder::writeMidiEvent(long deltaTicks, int type, int chan, int b1, int b2)
{
    writeMidiEvent(deltaTicks, type, chan, b1);
    writeByte(b2);
}

void MIDIFileBuilder::writeTempo(long deltaTicks, long tempoMicrosecondsPerQuarterNote)
{
    writeVarLen(deltaTicks);
    writeByte(META_EVENT);
    writeByte(TEMPO);
    writeByte(3); // number of bytes to follow
    writeByte((tempoMicrosecondsPerQuarterNote >> 16) & 0xff);
    writeByte((tempoMicrosecondsPerQuarterNote >> 8) & 0xff);
    writeByte(tempoMicrosecondsPerQuarterNote & 0xff);
    m_previousStatus = META_EVENT;
}

void MIDIFileBuilder::writeTimeSignature(long deltaTicks, int num, int den, int cc = 24, int bb = 8)
{
    writeVarLen(deltaTicks);
    writeByte(META_EVENT);
    writeByte(TIME_SIGNATURE);
    writeByte(4); // number of bytes to follow
    writeByte(num & 0xff);
    writeByte(den & 0xff); // denominator is exponent power of 2
    writeByte(cc & 0xff); // cc is number of MIDI clocks in a metronome click (24 should be OK)
    writeByte(bb & 0xff); // bb is number of 32nd notes per quarter note (usually 8)
    m_previousStatus = META_EVENT;
}

void MIDIFileBuilder::writeByte(quint8 value) // puts a single byte into the stream
{
    *m_dataStream << value;
    m_numBytesWritten++;
}

void MIDIFileBuilder::write2Bytes(quint16 data)
{
    writeByte((data >> 8) & 0xff);
    writeByte(data & 0xff);
}

void MIDIFileBuilder::write4Bytes(quint32 data)
{
    writeByte((data >> 24) & 0xff);
    writeByte((data >> 16) & 0xff);
    writeByte((data >> 8) & 0xff);
    writeByte(data & 0xff);
}

void MIDIFileBuilder::writeVarLen(quint64 value)
{
    quint64 buffer = value & 0x7f;
    while ((value >>= 7) > 0)
    {
        buffer <<= 8;
        buffer |= 0x80;
        buffer += (value & 0x7f);
    }
    while (true)
    {
        writeByte(buffer & 0xff);
        if (buffer & 0x80)
            buffer >>= 8;
        else
            break;
    }
}
