#ifndef ENVELOPEDATA_H
#define ENVELOPEDATA_H
#include "simplemap.h"

struct EnvelopeData
{
    unsigned char MIDIChannel;
    unsigned char MIDICCNumber;
    SimpleMap<unsigned int, float> envelope;
};

#endif // ENVELOPEDATA_H
