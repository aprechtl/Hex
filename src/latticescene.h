#ifndef LATTICESCENE_H
#define LATTICESCENE_H
#include <QtWidgets/QGraphicsScene>
#include "simplevector.h"
#include <QtGui/QBrush>

class MIDIEventHandler;
struct LatticeData;

class LatticeScene : public QGraphicsScene
{
public:
    LatticeScene(LatticeData *latticeData, QObject *parent = 0);

    // inline members
    bool showPeriodShading;

    // inline methods
    QRectF &getButtonRect() {return buttonRect;}
    QPainterPath &getButtonPath() {return buttonPath;}
    QBrush &getDarkButtonBrush() {return darkButtonBrush;}
    QBrush &getLightButtonBrush() {return lightButtonBrush;}
    QBrush &getPressedButtonBrush() {return pressedButtonBrush;}
    void setButtonPath(const QPainterPath &path) {buttonPath = path;}
    void setButtonRect(const QRectF &rect) {buttonRect = rect;}
    void setDarkButtonColor(const QColor &color) {darkButtonBrush.setColor(color);}
    void setDarkLaneColor(const QColor &color) {darkLaneBrush.setColor(color);}
    void setLightButtonColor(const QColor &color) {lightButtonBrush.setColor(color);}
    void setLightLaneColor(const QColor &color) {lightLaneBrush.setColor(color);}
    void setMIDIEventHandler(MIDIEventHandler *handler) {midiEventHandler = handler;}
    void setPressedButtonColor(const QColor &color) {pressedButtonBrush.setColor(color);}

protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void drawForeground(QPainter *painter, const QRectF &rect);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    void drawButtonSet(QPainter *painter, const QRectF &rect, const SimpleVector<unsigned int> &indices);
    void drawLineSet(QPainter *painter, const QRectF &rect, const SimpleVector<unsigned int> &indices);
    int visibleButtonAt(const QPointF &point);

    // these are initialized in the initializer list
    LatticeData *latticeData;
    MIDIEventHandler *midiEventHandler;
    QBrush darkButtonBrush;
    QBrush darkLaneBrush;
    QBrush lightButtonBrush;
    QBrush lightLaneBrush;
    QBrush pressedButtonBrush;

    QRectF buttonRect;
    QPainterPath buttonPath;
    SimpleVector<short> pressedButtonsJ;
    SimpleVector<short> pressedButtonsK;
};

#endif
