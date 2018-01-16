#ifndef QDATASTREAMOPERATORS_H
#define QDATASTREAMOPERATORS_H

struct LatticeSettings;
struct TimingSettings;
class QDataStream;

QDataStream &operator<<(QDataStream &out, const LatticeSettings &settings);

QDataStream &operator>>(QDataStream &in, LatticeSettings &settings);

QDataStream &operator<<(QDataStream &out, const TimingSettings &settings);

QDataStream &operator>>(QDataStream &in, TimingSettings &settings);

#endif
