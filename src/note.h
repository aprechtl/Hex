#ifndef NOTE_H
#define NOTE_H
#include <QtWidgets/QGraphicsItem>

class NoteDragHandler;

class Note : public QGraphicsItem
{
public:
    Note(float length, unsigned char velocity, unsigned short laneIndex, int track);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

    // inline methods
    QRectF boundingRect() const {return bRect;}
    unsigned short int getLaneIndex() const {return laneIndex;}
    unsigned char getVelocity() const {return velocity;}
    void setLaneIndex(short index) {laneIndex = index; setZValue(-index);}
    void setTrack(int track) {m_track = track;}
    void setVelocity(unsigned char vel) {velocity = vel; update();}
    void setWidth(double width) {prepareGeometryChange(); bRect.setWidth(width);}
    int track() const {return m_track;}

protected:
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    NoteDragHandler *createNoteDragger(QGraphicsSceneMouseEvent *event);
    bool hoveredOnRightOfNote(double xPos, double xScale) const;
    bool hoveredOnTopOfNote(double yPos, double yScale) const;
    bool selectedNotesAreInTheSameLane() const;

    QRectF bRect;
    unsigned short laneIndex;
    unsigned char velocity;
    NoteDragHandler *dragHandler;
    int m_track;
};

#endif
