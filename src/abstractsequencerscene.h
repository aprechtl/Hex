#ifndef ABSTRACTSEQUENCERSCENE_H
#define ABSTRACTSEQUENCERSCENE_H
#include <QtWidgets/QGraphicsScene>

class BarLineDrawer;
class QGraphicsOpacityEffect;
class QLabel;
class QPropertyAnimation;
class QUndoCommand;
class QUndoStack;

class AbstractSequencerScene : public QGraphicsScene
{
    Q_OBJECT

public:
    AbstractSequencerScene(BarLineDrawer *barLineDrawer, QUndoStack *undoStack, QWidget *view);
    void copy();
    void cut();
    void deleteSelectedItems();
    void fadeLabelIn();
    void fadeLabelOut();
    void paste();
    void pushUndoCommand(QUndoCommand *command);
    void setLabelText(const QString &string);
    void setLength(double length);
    void setTopAndHeight(double top, double height);

    virtual void selectAll() = 0;

    // inline methods
    double cursorPos() const {return m_cursorPos;}
    double getCurrentSnap() const {return (m_snapEnabled) ? m_snapSize : 1;}
    QLabel *label() const {return m_label;}
    unsigned int roundToLowerSnapPos(unsigned int xPos) const {return (m_snapEnabled) ? xPos / static_cast<unsigned int>(m_snapSize) * m_snapSize : xPos;}
    unsigned int roundToNearestSnapPos(unsigned int xPos) const {return (m_snapEnabled) ? ((xPos + m_snapSize / 2 ) / m_snapSize) * m_snapSize : xPos;}
    void setCursorPen(const QPen &pen) {cursorPen = pen; update();}
    void setCursorPos(double pos) {m_cursorPos = pos; update();}
    void setSnapSize(double size) {m_snapSize = size;}
    void setSnapToGrid(bool on) {m_snapEnabled = on;}
    double snapSize() const {return m_snapSize;}

signals:
    void cursorMoved(double pos);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect); // draws bar lines
    void drawForeground(QPainter *painter, const QRectF &rect); // draws cursor
    void roundSetAndEmitCursorPos(unsigned int pos);

private:
    virtual bool copyImplementation(QDataStream &stream) = 0;
    virtual QUndoCommand *deleteCommand() = 0;
    virtual QString mimeType() const = 0;
    virtual QUndoCommand *pasteCommand(QDataStream &stream, int numItems) = 0;

    // these are initialized in the initializer list
    BarLineDrawer *barLineDrawer;
    QUndoStack *undoStack;
    QLabel *m_label;
    double m_cursorPos;
    bool m_snapEnabled;
    unsigned int m_snapSize;
    QGraphicsOpacityEffect *labelFadeEffect;
    QPropertyAnimation *fadeAnimation;

    QPen cursorPen;
};

#endif
