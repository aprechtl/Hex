#ifndef ENVELOPESCENE_H
#define ENVELOPESCENE_H
#include "abstractsequencerscene.h"
#include "simplemap.h"
#include "simplevector.h"

class NodeMover;
class TrackManagerDialog;

class EnvelopeScene : public AbstractSequencerScene
{
public:
    EnvelopeScene(BarLineDrawer *barLineDrawer, QUndoStack *undoStack, QWidget *view);
    ~EnvelopeScene();
    void addNodeAtClickPos(const QPointF &scenePos);
    void clearSelectedNodes();
    void editNodeValue(int index);
    void evaluateSelection(const QRectF &rect);
    void setNodeSelected(int index);
    void selectAll();
    void setMIDICCEnvelope(SimpleMap<unsigned int, float> *envelope, int track, int envelopeIndex);

    // inline methods
    int currentEnvelopeIndex() const {return m_currentEnvelopeIndex;}
    int currentTrack() const {return m_currentTrack;}
    void setEnvelopePen(const QPen &pen) {m_envelopePen = pen; update();}
    void setSelectedNodePen(const QPen &pen) {m_selectedNodePen = pen; update();} // must be cosmetic
    void setUnselectedNodePen(const QPen &pen) {m_unselectedNodePen = pen; update();} // must be cosmetic

    void setTrackManagerDialog(TrackManagerDialog *dialog) {trackManager = dialog;}

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    // virtual
    bool copyImplementation(QDataStream &stream);
    QUndoCommand *deleteCommand();
    QString mimeType() const {return "hex/nodes";}
    QUndoCommand *pasteCommand(QDataStream &stream, int numItems);

    // helper methods
    double getValueFromUser(double currentVal0To1, double minVal, double maxVal, bool useFloatingPoint);
    int nodeAt(const QPointF &point, const QTransform &viewTransform); // returns index of node at the point, or -1 if there is no node there

    // these are initialized in the initializer list
    NodeMover *m_nodeMover;
    SimpleMap<unsigned int, float> *m_envelope;

    TrackManagerDialog *trackManager;
    int m_currentTrack, m_currentEnvelopeIndex;
    QPen m_envelopePen;
    QPen m_selectedNodePen;
    QPen m_unselectedNodePen;
    SimpleVector<int> m_selectedNodeIndices;
};

#endif
