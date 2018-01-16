#ifndef BUTTONSHAPECALCULATOR_H
#define BUTTONSHAPECALCULATOR_H
#include <QtGui/QPainterPath>

class ButtonShapeCalculator
{
public:
    enum Shape {Hexagon, Ellipse, Square};

    ButtonShapeCalculator(double size, Shape shape);
    QRectF buttonBoundingRect() const {return rect;}
    QPainterPath buttonPath() const {return path;}

private:
    void calculateEllipse();
    void calculateHexagon();
    void calculateSquare();

    // these are initialized in the initializer list
    double spacing;
    double rightCoord;

    QPainterPath path;
    QRectF rect;
};

#endif
