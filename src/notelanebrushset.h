#ifndef NOTELANEBRUSHSET_H
#define NOTELANEBRUSHSET_H
#include <QBrush>

class NoteLaneBrushSet
{
public:
    NoteLaneBrushSet() {darkBrush.setStyle(Qt::SolidPattern); lightBrush.setStyle(Qt::SolidPattern);}
    QBrush &getDarkBrush() {return darkBrush;}
    QBrush &getLightBrush() {return lightBrush;}
    QColor getDarkColor() const {return darkBrush.color();}
    QColor getLightColor() const {return lightBrush.color();}
    void setDarkColor(const QColor &color) {darkBrush.setColor(color);}
    void setLightColor(const QColor &color) {lightBrush.setColor(color);}

private:
    QBrush darkBrush;
    QBrush lightBrush;
};

#endif
