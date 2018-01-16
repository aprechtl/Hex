#include "latticemanager.h"
#include "buttonshapecalculator.h"
#include "latticedata.h"
#include "latticescene.h"
#include "midiportmanager.h"
#include "projectsettingsdialog.h"
#include "sequencerscene.h"

LatticeManager::LatticeManager(LatticeScene *latticeScene, SequencerScene *sequencerScene, LatticeData *latticeData, MIDIPortManager *manager)
    : latticeScene(latticeScene),
      sequencerScene(sequencerScene),
      latticeData(latticeData),
      midiPortManager(manager),
      transformMode(0),
      midiOutputEnabled(true)
{
}

LatticeManager::~LatticeManager()
{
}

void LatticeManager::adjustButtonShapeAccordingToLayout()
{
    QTransform layoutRotation;
    layoutRotation.rotateRadians(dt.getSnuggle());
    layoutAdjustedButtonPath = layoutRotation.map(untransformedButtonPath);
    layoutAdjustedButtonRect = layoutRotation.mapRect(untransformedButtonRect);
}

void LatticeManager::drawLattice(const LatticeSettings &latticeSettings)
{
    dt.setAPSLayout(latticeSettings.apsLayout, latticeSettings.flipped);

    int numLightColumns = latticeSettings.numLargeSteps + latticeSettings.numSmallSteps;
    int numDarkColumns = latticeSettings.numLargeSteps;
    float maxPeriods = latticeSettings.numPeriods * .5;

    latticeData->darkIndices.setSize(0);
    latticeData->lightIndices.setSize(0);

    // calculate min/max values for columns
    short minKLight = numLightColumns / -2;
    short maxKLight = minKLight + numLightColumns - 1;
    short minKVisible = (numDarkColumns + numLightColumns) / -2;
    short maxKVisible = minKVisible + numDarkColumns + numLightColumns - 1;

    short i = 0; // button counter
    short k = 0;

    drawLatticeHelper(k, minKLight, maxKLight, minKVisible, maxKVisible, maxPeriods, i);
    for (k = 1; k <= HexSettings::maxK; ++k)
    {
        drawLatticeHelper(k, minKLight, maxKLight, minKVisible, maxKVisible, maxPeriods, i);
        drawLatticeHelper(-k, minKLight, maxKLight, minKVisible, maxKVisible, maxPeriods, i);
    }

    adjustButtonShapeAccordingToLayout();

    transformLattice();
    updateSceneBoundingRects();
}

// iterates through the j values for the given k value
void LatticeManager::drawLatticeHelper(short k,
                                       short minKLight,
                                       short maxKLight,
                                       short minKVisible,
                                       short maxKVisible,
                                       float maxPeriods,
                                       short &i)
{
    // info for limiting the lattice height to a certain number of periods
    float basePitch = dt.getUntransformedRelativeBeta() * k;

    bool columnIsVisible = (k >= minKVisible && k <= maxKVisible);
    bool columnIsLight = false;

    if (columnIsVisible)
        columnIsLight = (k >= minKLight && k <= maxKLight);

    for (int j = HexSettings::minJ; j <= HexSettings::maxJ; ++j)
    {
        float pitch = basePitch + j; // expressed in terms of num periods above reference height

        if (columnIsVisible && pitch >= -maxPeriods && pitch <= maxPeriods)
        {
            if (columnIsLight)
                latticeData->lightIndices.appendByValue(i);
            else
                latticeData->darkIndices.appendByValue(i);
        }

        // this needs to be calculated whether drawn or not so that you can
        // view notes even if their lane isn't currently visible
        double x, y;
        dt.captureButtonCoordinates(j, k, x, y);
        untransformedPoints[i] = QPointF(x, y);

        ++i;
    }
}

void LatticeManager::sendAlphaMIDI()
{
    midiPortManager->sendPeriodMessages(dt.period());
}

void LatticeManager::sendBetaMIDI()
{
    midiPortManager->sendGeneratorMessages(dt.generator());
}

void LatticeManager::sendMIDIData()
{
    if (midiOutputEnabled)
    {
        sendAlphaMIDI();
        sendBetaMIDI();
    }
}

void LatticeManager::setAlpha(double alpha)
{
    dt.setPeriod(alpha);
    transformLattice();

    if (midiOutputEnabled)
        sendAlphaMIDI();
}

void LatticeManager::setBeta(double beta)
{
    dt.setGenerator(beta);
    transformLattice();

    if (midiOutputEnabled)
        sendBetaMIDI();
}

void LatticeManager::setButtonPressed(short index, bool pressed)
{
    if (index < 0 || index >= HexSettings::numButtons)
        return;

    if (pressed)
        latticeData->pressedIndices.appendSafely(index);
    else
        latticeData->pressedIndices.removeValue(index);

    latticeScene->update();
    sequencerScene->update();
}

void LatticeManager::setButtonScaleAndType(float scale, int type)
{
    ButtonShapeCalculator buttonShapeCalculator(scale, static_cast<ButtonShapeCalculator::Shape>(type));
    untransformedButtonPath = buttonShapeCalculator.buttonPath();
    untransformedButtonRect = buttonShapeCalculator.buttonBoundingRect();

    adjustButtonShapeAccordingToLayout();

    latticeScene->setButtonPath(latticeTransform.map(layoutAdjustedButtonPath));
    latticeScene->setButtonRect(latticeTransform.mapRect(layoutAdjustedButtonRect));
    updateSceneBoundingRects();

    latticeScene->update();
}

void LatticeManager::toTransformMode(int mode)
{
    transformMode = mode;
    transformLattice();
    updateSceneBoundingRects();
}

void LatticeManager::transformLattice()
{
    switch (transformMode)
    {
    case 0:
        latticeTransform = QTransform(1, dt.getShearFactor(), 0, 0, 1, 0, 0, 0, 1);
        latticeData->periodSize = dt.getShearPeriodSize();
        break;
    case 1:
        latticeTransform = QTransform().rotate(dt.getRotationAngle());
        latticeData->periodSize = dt.getRotationPeriodSize();
        break;
    }

    latticeScene->setButtonPath(latticeTransform.map(layoutAdjustedButtonPath));
    latticeScene->setButtonRect(latticeTransform.mapRect(layoutAdjustedButtonRect));

    for (int i = 0; i < HexSettings::numButtons; ++i)
    {
        latticeData->buttonPositions[i] = latticeTransform.map(untransformedPoints[i]);
    }

    latticeScene->update();
    sequencerScene->updateNotePositions();

    // need this in case there are no notes (i.e., nothing automatically updating sequencer)
    sequencerScene->update();
}

void LatticeManager::updateSceneBoundingRects()
{
    // =================================== CALCULATE THE LATTICE BOUNDING RECT
    QPolygonF latticeCenterPoints(latticeData->lightIndices.size() + latticeData->darkIndices.size());
    for (int i = 0; i < latticeData->lightIndices.size(); ++i)
        latticeCenterPoints.append(latticeData->buttonPositions[latticeData->lightIndices[i]]);
    for (int i = 0; i < latticeData->darkIndices.size(); ++i)
        latticeCenterPoints.append(latticeData->buttonPositions[latticeData->darkIndices[i]]);

    QPainterPath latticePath;
    latticePath.addPolygon(latticeCenterPoints);

    QRectF transformedNoteRect(latticeTransform.mapRect(layoutAdjustedButtonRect));

    QRectF latticeRect(latticePath.boundingRect().adjusted(transformedNoteRect.left(),
                                                           transformedNoteRect.top(),
                                                           transformedNoteRect.right(),
                                                           transformedNoteRect.bottom()));
    // ======================================================================

    latticeScene->setSceneRect(latticeRect);
    sequencerScene->setTopAndHeight(latticeRect.top(), latticeRect.height());
}
