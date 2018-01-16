#ifndef LATTICEDATA_H
#define LATTICEDATA_H
#include "hexsettings.h"
#include "simplevector.h"
#include <QtCore/QPointF>

struct LatticeData
{
    LatticeData() : darkIndices(HexSettings::numButtons), lightIndices(HexSettings::numButtons), pressedIndices(HexSettings::maxPolyphony) {}

    SimpleVector<unsigned int> darkIndices;
    SimpleVector<unsigned int> lightIndices;
    SimpleVector<unsigned int> pressedIndices;
    QPointF buttonPositions[HexSettings::numButtons];
    double periodSize;
};

#endif // LATTICEDATA_H
