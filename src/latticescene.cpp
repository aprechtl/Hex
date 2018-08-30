#include "latticescene.h"
#include "latticedata.h"
#include "midieventhandler.h"
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>

LatticeScene::LatticeScene(LatticeData *latticeData, QObject *parent)
    : QGraphicsScene(parent),
      showPeriodShading(true),
      latticeData(latticeData),
      midiEventHandler(0),
      darkButtonBrush(Qt::SolidPattern),
      darkLaneBrush(Qt::SolidPattern),
      lightButtonBrush(Qt::SolidPattern),
      lightLaneBrush(Qt::SolidPattern),
      pressedButtonBrush(Qt::SolidPattern),
      pressedButtonsJ(16),
      pressedButtonsK(16)
{
}

void LatticeScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Since the optimization flag DontSavePainterState is set, I call this
    // here so that the buttons don't need to call it in their paint() method.
    painter->setPen(Qt::NoPen);

    double halfButtonWidth = buttonRect.width() * .5;
    double halfButtonHeight = buttonRect.height() * .5;
    QRectF adjustedRect(rect.adjusted(-halfButtonWidth, -halfButtonHeight, halfButtonWidth, halfButtonHeight));

    painter->setBrush(darkButtonBrush);
    drawButtonSet(painter, adjustedRect, latticeData->darkIndices);
    painter->setBrush(lightButtonBrush);
    drawButtonSet(painter, adjustedRect, latticeData->lightIndices);
    painter->setBrush(pressedButtonBrush);
    drawButtonSet(painter, adjustedRect, latticeData->pressedIndices);

    // highlight center button
    painter->setBrush(QBrush(QColor(255, 255, 255, 80), Qt::SolidPattern));
    painter->drawPath(buttonPath);
}

void LatticeScene::drawButtonSet(QPainter *painter, const QRectF &rect, const SimpleVector<unsigned int> &indices)
{
    for (int i = 0; i < indices.size(); ++i)
    {
        if (rect.contains(latticeData->buttonPositions[indices[i]]))
            painter->drawPath(buttonPath.translated(latticeData->buttonPositions[indices[i]]));
    }
}

void LatticeScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    painter->setPen(Qt::NoPen);

    if (showPeriodShading)
    {
        painter->setBrush(QBrush(QColor(0, 0, 0, 18))); // for period shading
        // figure out which octaves are in sight and need to be shaded
        int octave = rect.bottom() / latticeData->periodSize - 1;
        if (octave % 2 == 0)
            ++octave;
        int targetOctave = rect.top() / latticeData->periodSize + 1;
        while (octave < targetOctave)
        {
            painter->drawRect(QRectF(rect.left(), octave * latticeData->periodSize, rect.width(), latticeData->periodSize));
            octave += 2;
        }
    }

    painter->setBrush(darkLaneBrush);
    drawLineSet(painter, rect, latticeData->darkIndices);
    painter->setBrush(lightLaneBrush);
    drawLineSet(painter, rect, latticeData->lightIndices);
}

void LatticeScene::drawLineSet(QPainter *painter, const QRectF &rect, const SimpleVector<unsigned int> &indices)
{
    // QRectF is implemented in terms of left/top/width/height, so it's faster to calculate these just once
    double rectRight = rect.right();
    double rectBottom = rect.bottom();

    // there is no benefit to using painter->drawRects instead of drawing them individually as I'm doing here
    for (int i = 0; i < indices.size(); ++i)
    {
        if (latticeData->buttonPositions[indices[i]].y() > rect.top() &&
            latticeData->buttonPositions[indices[i]].y() < rectBottom &&
            latticeData->buttonPositions[indices[i]].x() < rectRight)
        {
            painter->drawRect(QRectF(latticeData->buttonPositions[indices[i]].x(),             // left
                                     latticeData->buttonPositions[indices[i]].y() - .0125,     // top
                                     rectRight - latticeData->buttonPositions[indices[i]].x(), // width
                                     .025));                                                   // height
        }
    }
}

void LatticeScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    int buttonIndex = visibleButtonAt(event->scenePos());

    if (buttonIndex == -1)
        return;

    short pressedButtonJ, pressedButtonK;
    HexSettings::convertNoteLaneIndexToJK(buttonIndex, pressedButtonJ, pressedButtonK);

    // ensure we don't already have this button press
    for (int i = 0; i < pressedButtonsJ.size(); ++i)
    {
        if (pressedButtonsJ.at(i) == pressedButtonJ && pressedButtonsK.at(i) == pressedButtonK)
            return;
    }

    pressedButtonsJ.appendSafely(pressedButtonJ);
    pressedButtonsK.appendSafely(pressedButtonK);

    // this will light up the button
    midiEventHandler->sendAndRecordNoteOn(pressedButtonJ, pressedButtonK, 90);

    event->accept();
}

void LatticeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    int buttonIndex = visibleButtonAt(event->scenePos());

    if (buttonIndex == -1)
        return;

    short pressedButtonJ, pressedButtonK;
    HexSettings::convertNoteLaneIndexToJK(buttonIndex, pressedButtonJ, pressedButtonK);

    // find the button press and remove it
    for (int i = 0; i < pressedButtonsJ.size(); ++i)
    {
        if (pressedButtonsJ.at(i) == pressedButtonJ && pressedButtonsK.at(i) == pressedButtonK)
        {
            pressedButtonsJ.removeIndex(i);
            pressedButtonsK.removeIndex(i);

            // this will unlight the button
            midiEventHandler->sendAndRecordNoteOff(pressedButtonJ, pressedButtonK);

            event->accept();
            return;
        }
    }
}

int LatticeScene::visibleButtonAt(const QPointF &point)
{
    for (int i = 0; i < latticeData->lightIndices.size(); ++i)
    {
        if (buttonRect.translated(latticeData->buttonPositions[latticeData->lightIndices[i]]).contains(point)
                && buttonPath.translated(latticeData->buttonPositions[latticeData->lightIndices[i]]).contains(point))
            return latticeData->lightIndices[i];
    }

    for (int i = 0; i < latticeData->darkIndices.size(); ++i)
    {
        if (buttonRect.translated(latticeData->buttonPositions[latticeData->darkIndices[i]]).contains(point)
                && buttonPath.translated(latticeData->buttonPositions[latticeData->darkIndices[i]]).contains(point))
            return latticeData->darkIndices[i];
    }

    return -1;
}
