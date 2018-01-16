#include "envelopegenerator.h"
#include "hexsettings.h"
#include <math.h>

EnvelopeGenerator::EnvelopeGenerator(const SimpleMap<unsigned int, float> * const nodes, double millisecondsPerTick, SequencerEvent::Type type)
    : nodes(nodes),
      millisecondsPerTick(millisecondsPerTick),
      ticksPerMillisecond(1. / millisecondsPerTick),
      sequencerEventArray(0),
      eventType(type)
{
}

void EnvelopeGenerator::calculateNodePositionsInMS()
{
    const unsigned int *nodePositionsInTicks = nodes->getKeyArray();
    for (int i = 0; i < nodes->count(); ++i)
    {
        nodePositionsInMS[i] = nodePositionsInTicks[i] * millisecondsPerTick;
    }
}

void EnvelopeGenerator::calculateNumMidiEvents()
{
    const float *values = nodes->getValueArray();
    int i = 0, j = 1;
    while (j < nodes->count())
    {
        if (values[i] != values[j]) // this is for the ramp
            m_numEvents += (nodePositionsInMS[j] - nodePositionsInMS[i]) * HexSettings::oneOverEnvResolution + 1;

        i = j;
        ++j;
    }

    ++m_numEvents; // this is for the last node (or first if there's only one)
}

void EnvelopeGenerator::generate()
{
    m_numEvents = 0;

    if (nodes->count() == 0)
        return;

    nodePositionsInMS = new double[nodes->count()];

    calculateNodePositionsInMS();
    calculateNumMidiEvents();

    sequencerEventArray = new SequencerEvent[m_numEvents];

    generateTheEnvelope();

    delete [] nodePositionsInMS;
}

// ###########################################################################
// ########################################################### DERIVED CLASSES

FloatEnvelopeGenerator::FloatEnvelopeGenerator(const SimpleMap<unsigned int, float> * const nodes,
                                               double millisecondsPerTick,
                                               SequencerEvent::Type eventType)
    : EnvelopeGenerator(nodes, millisecondsPerTick, eventType)
{
}

void FloatEnvelopeGenerator::generateTheEnvelope()
{
    const float *values = nodes->getValueArray();

    int eventCounter = 0, i = 0, j = 1;
    while (j < nodes->count())
    {
        if (values[i] != values[j])
        {
            double msPos = nodePositionsInMS[i]; // just initializing

            while (msPos <= nodePositionsInMS[j])
            {
                // linear interpolation (weighted average)
                double amountValuesJ = (msPos - nodePositionsInMS[i]) / (nodePositionsInMS[j] - nodePositionsInMS[i]);
                float interpolatedValue = amountValuesJ * values[j] + (1. - amountValuesJ) * values[i];

                sequencerEventArray[eventCounter].setData(eventType, msPos * ticksPerMillisecond, interpolatedValue * 1200.);

                ++eventCounter;
                msPos += HexSettings::envelopeResolutionMS;
            }
        }

        ++i;
        ++j;
    }

    sequencerEventArray[eventCounter].setData(eventType, nodes->getKeyArray()[i], values[i] * 1200.);
}

MIDICCEnvelopeGenerator::MIDICCEnvelopeGenerator(const SimpleMap<unsigned int, float> * const nodes,
                                                 double millisecondsPerTick,
                                                 unsigned char ccType,
                                                 unsigned char channel,
                                                 unsigned char track, SequencerEvent::Type eventType)
    : EnvelopeGenerator(nodes, millisecondsPerTick, eventType),
      ccType(ccType),
      channel(channel + 175),
      track(track)
{
}

void MIDICCEnvelopeGenerator::generateTheEnvelope()
{
    const float *values = nodes->getValueArray();

    int eventCounter = 0, i = 0, j = 1;
    while (j < nodes->count())
    {
        if (values[i] != values[j])
        {
            double msPos = nodePositionsInMS[i]; // just initializing

            while (msPos <= nodePositionsInMS[j])
            {
                // linear interpolation (weighted average)
                double amountValuesJ = (msPos - nodePositionsInMS[i]) / (nodePositionsInMS[j] - nodePositionsInMS[i]);
                double interpolatedValue = amountValuesJ * values[j] + (1. - amountValuesJ) * values[i];
                unsigned char finalValue = static_cast<unsigned char>(interpolatedValue * 127. + .5); // rounds

                if (finalValue != sequencerEventArray[eventCounter - 1].midiData.byte3) // don't send duplicate values
                {
                    sequencerEventArray[eventCounter].setData(eventType, msPos * ticksPerMillisecond, channel, ccType, finalValue, track);
                    ++eventCounter;
                }

                msPos += HexSettings::envelopeResolutionMS;
            }
        }

        ++i;
        ++j;
    }

    sequencerEventArray[eventCounter].setData(eventType, nodes->getKeyArray()[i], channel, ccType, static_cast<unsigned char>(values[i] * 127. + .5), track);
    ++eventCounter;

    // final number of events might be larger than predicted if duplicate events were removed
    m_numEvents = eventCounter;
}
