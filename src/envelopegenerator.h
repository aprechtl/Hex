#ifndef ENVELOPEGENERATOR_H
#define ENVELOPEGENERATOR_H
#include "sequencerevent.h"
#include "simplemap.h"

class EnvelopeGenerator
{
public:
    void generate();
    SequencerEvent *eventArray() {return sequencerEventArray;}
    int numEvents() const {return m_numEvents;}

protected:
    EnvelopeGenerator(const SimpleMap<unsigned int, float> * const nodes, double millisecondsPerTick, SequencerEvent::Type eventType);

    // these are initialized in the initializer list
    const SimpleMap<unsigned int, float> * const nodes;
    const double millisecondsPerTick;
    const double ticksPerMillisecond;
    SequencerEvent *sequencerEventArray;
    const SequencerEvent::Type eventType;

    int m_numEvents;
    double *nodePositionsInMS;

private:
    void calculateNodePositionsInMS();
    void calculateNumMidiEvents();
    virtual void generateTheEnvelope() = 0;
};

// ###########################################################################
// ###########################################################################

class FloatEnvelopeGenerator : public EnvelopeGenerator
{
public:
    FloatEnvelopeGenerator(const SimpleMap<unsigned int, float> * const nodes, double millisecondsPerTick, SequencerEvent::Type eventType);

private:
    void generateTheEnvelope();
};

// ###########################################################################
// ###########################################################################

class MIDICCEnvelopeGenerator : public EnvelopeGenerator
{
public:
    MIDICCEnvelopeGenerator(const SimpleMap<unsigned int, float> * const nodes,
                            double millisecondsPerTick,
                            unsigned char ccType,
                            unsigned char channel,
                            unsigned char track,
                            SequencerEvent::Type eventType
                            );

private:
    void generateTheEnvelope();

    unsigned char ccType;
    unsigned char channel;
    unsigned char track;
};
// ===========================================================================

#endif
