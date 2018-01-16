#include "qdatastreamoperators.h"
#include "projectsettingsdialog.h"

QDataStream &operator<<(QDataStream &out, const LatticeSettings &settings)
{
    out << settings.numLargeSteps << settings.numSmallSteps << settings.numPeriods << settings.apsLayout << settings.flipped << settings.alpha << settings.betaSliderMin << settings.betaSliderMax;
    return out;
}

QDataStream &operator>>(QDataStream &in, LatticeSettings &settings)
{
    in >> settings.numLargeSteps >> settings.numSmallSteps >> settings.numPeriods >> settings.apsLayout >> settings.flipped >> settings.alpha >> settings.betaSliderMin >> settings.betaSliderMax;
    return in;
}

QDataStream &operator<<(QDataStream &out, const TimingSettings &settings)
{
    out << settings.timeSigNumerator << settings.timeSigDenominator << settings.numMeasures << settings.tempo;
    return out;
}

QDataStream &operator>>(QDataStream &in, TimingSettings &settings)
{
    in >> settings.timeSigNumerator >> settings.timeSigDenominator >> settings.numMeasures >> settings.tempo;
    return in;
}
