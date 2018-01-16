#ifndef MIDIFILEBUILDER_H
#define MIDIFILEBUILDER_H
#include "sequencerevent.h"
#include <QtCore/QObject>
#include <vector>

class QDataStream;
template <class SequencerEvent>
class SimpleVector;

class MIDIFileBuilder
{
public:
    MIDIFileBuilder();
    ~MIDIFileBuilder();
    void setTimeSignature(int numerator, int denominator);
    void setTempo(double tempoBPM);
    void setResolution(int ticksPerQuarterNote);
    void setEvents(const SimpleVector<SequencerEvent> &events);
    void writeToFile(const QString& fileName);

private:
    void writeTrack(int track);

    void writeMetaEvent(long deltaTicks, int type, int data);
    void writeMetaEvent(long deltaTicks, int type);
    void writeMidiEvent(long deltaTicks, int type, int chan, int b1);
    void writeMidiEvent(long deltaTicks, int type, int chan, int b1, int b2);
    void writeTempo(long deltaTicks, long m_tempoMicrosecsPerQuarterNote);
    void writeTimeSignature(long deltaTicks, int num, int den, int cc, int bb);

    void writeByte(quint8 value);
    void write2Bytes(quint16 data);
    void write4Bytes(quint32 data);
    void writeVarLen(quint64 value);

    QDataStream *m_dataStream;
    std::vector<SequencerEvent> *m_eventVectors;
    quint64 m_numBytesWritten;
    int m_numTracks;
    int m_previousStatus;
    int m_tempoMicrosecsPerQuarterNote;
    int m_ticksPerQuarterNote;
    int m_timeSigDenom;
    int m_timeSigNumer;
};

#endif
