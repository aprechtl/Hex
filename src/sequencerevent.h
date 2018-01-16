#ifndef SEQUENCEREVENT_H
#define SEQUENCEREVENT_H

struct SequencerEvent
{
    // the order implies the order in which events should be sorted
    enum Type {MIDINoteOff = 0,
               MIDICC = 1,
               MIDIOther = 2,
               Period = 3,
               Generator = 4,
               JI = 5,
               Harmonicity = 6,
               MIDINoteOn = 7} type;
    unsigned int ticks;
    union {
        struct {unsigned char byte1;
                unsigned char byte2;
                unsigned char byte3;
                unsigned char track;
               } midiData;
        float value;
    };

    void setData(Type type, unsigned int ticks, unsigned char byte1, unsigned char byte2, unsigned char byte3, unsigned char track);
    void setData(Type type, unsigned int ticks, float value);
    bool operator <(const SequencerEvent &other);
    static bool compareEvents(const SequencerEvent &first, const SequencerEvent &second);
};

inline void SequencerEvent::setData(Type type, unsigned int ticks, unsigned char byte1, unsigned char byte2, unsigned char byte3, unsigned char track)
{
    this->type = type;
    this->ticks = ticks;
    this->midiData.byte1 = byte1;
    this->midiData.byte2 = byte2;
    this->midiData.byte3 = byte3;
    this->midiData.track = track;
}

inline void SequencerEvent::setData(SequencerEvent::Type type, unsigned int ticks, float value)
{
    this->type = type;
    this->ticks = ticks;
    this->value = value;
}

inline bool SequencerEvent::operator<(const SequencerEvent &other)
{ return compareEvents(*this, other); }

inline bool SequencerEvent::compareEvents(const SequencerEvent &first, const SequencerEvent &second)
{
    if (first.ticks != second.ticks)
        return (first.ticks < second.ticks);

    return (static_cast<int>(first.type) < static_cast<int>(second.type));
}

#endif
