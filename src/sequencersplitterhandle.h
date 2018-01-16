#ifndef SEQUENCERSPLITTERHANDLE_H
#define SEQUENCERSPLITTERHANDLE_H
#include <QtWidgets/QWidget>

class QGraphicsView;

class SequencerSplitterHandle : public QWidget
{
    Q_OBJECT

public:
    // the handle needs a pointer to a scene in order to get the snap to grid information
    SequencerSplitterHandle(QGraphicsView *view, QWidget *parent = 0);
    void setMeasureLengthTicks(double length);

signals:
    void loopDisabled();
    void loopChanged(double, double);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    // these are initialized in the initializer list
    QGraphicsView *view;
    bool loopEnabled;
    double measureLengthTicks;

    bool mouseIsAdjustingLoop;
    double initialClickTickPos;
    double draggedClickTickPos;
};

#endif
