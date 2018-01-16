#include "buttonshapecalculator.h"
#include <math.h>

const double sqrt3Over2 = sqrt(3.0) * .5;
const double spacingConstant = pow(2., .5) / pow(3., .75);

ButtonShapeCalculator::ButtonShapeCalculator(double size, Shape shape)
    : spacing (size * spacingConstant), rightCoord(spacing * sqrt3Over2)
{
    switch (shape)
    {
    case Hexagon:
        calculateHexagon();
        return;
    case Ellipse:
        calculateEllipse();
        return;
    case Square:
        calculateSquare();
        return;
    }
}

void ButtonShapeCalculator::calculateEllipse()
{
    rect = QRectF(-rightCoord, -rightCoord, rightCoord * 2, rightCoord * 2);
    path.addEllipse(rect);
}

void ButtonShapeCalculator::calculateHexagon()
{
    double topRightCoord = spacing * .5;

    rect = QRectF(-rightCoord, -spacing, rightCoord * 2, spacing * 2);
    path.addPolygon(QPolygonF()
                    << QPointF(0, spacing)
                    << QPointF(rightCoord, topRightCoord)
                    << QPointF(rightCoord, -topRightCoord)
                    << QPointF(0, -spacing)
                    << QPointF(-rightCoord, -topRightCoord)
                    << QPointF(-rightCoord, topRightCoord));
}

void ButtonShapeCalculator::calculateSquare()
{
    rect = QRectF(-rightCoord, -rightCoord * sqrt3Over2, rightCoord * 2, rightCoord * sqrt3Over2 * 2);
    path.addRect(rect);
}
