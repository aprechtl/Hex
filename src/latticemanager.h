#ifndef LATTICEMANAGER_H
#define LATTICEMANAGER_H
#include "dynamictonality.h"
#include "hexsettings.h"
#include <QtGui/QPainterPath>
#include <QtGui/QTransform>

class LatticeScene;
class MIDIPortManager;
class SequencerScene;
struct LatticeData;
struct LatticeSettings;

class LatticeManager : public QObject
{
public:
    LatticeManager(LatticeScene *latticeScene, SequencerScene *sequencerScene, LatticeData *latticeData, MIDIPortManager *manager);
    ~LatticeManager();
    void drawLattice(const LatticeSettings &latticeSettings);
    void sendMIDIData();
    void setAlpha(double alpha);
    void setBeta(double beta);
    void setButtonPressed(short index, bool pressed);
    void setButtonScaleAndType(float scale, int type);
    void toTransformMode(int mode);
    void transformLattice();
    void updateSceneBoundingRects();

    // inline methods
    const DynamicTonality *getDT() const {return &dt;}
    int getTransformMode() const {return transformMode;}
    void setMIDIOutputEnabled(bool on) {midiOutputEnabled = on;}

private:
    void adjustButtonShapeAccordingToLayout();
    void constructorHelper(short k, short &i);
    void drawLatticeHelper(short k, short minKLight, short maxKLight, short minKVisible, short maxKVisible, float maxPeriods, short &i);
    void sendAlphaMIDI();
    void sendBetaMIDI();

    // these are initialized in the initializer list
    LatticeScene *latticeScene;
    SequencerScene *sequencerScene;
    LatticeData *latticeData;
    MIDIPortManager *midiPortManager;
    int transformMode;
    bool midiOutputEnabled;

    DynamicTonality dt;
    QTransform latticeTransform;
    QPainterPath layoutAdjustedButtonPath;
    QRectF layoutAdjustedButtonRect;
    QPainterPath untransformedButtonPath;
    QRectF untransformedButtonRect;
    QPointF untransformedPoints[HexSettings::numButtons];
};

#endif
