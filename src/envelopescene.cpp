#include "envelopescene.h"
#include "draghandlers.h"
#include "envelopeview.h"
#include "nodecommands.h"
#include "trackmanagerdialog.h"
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QInputDialog>

EnvelopeScene::EnvelopeScene(BarLineDrawer *barLineDrawer, QUndoStack *undoStack, QWidget *view)
    : AbstractSequencerScene(barLineDrawer, undoStack, view), m_nodeMover(0), m_envelope(0)
{
    setSceneRect(0, 0, 100, 1);
}

EnvelopeScene::~EnvelopeScene()
{
    if (m_nodeMover != NULL)
        delete m_nodeMover;
}

void EnvelopeScene::setMIDICCEnvelope(SimpleMap<unsigned int, float> *envelope, int track, int envelopeIndex)
{
    clearSelectedNodes();
    m_envelope = envelope;
    m_currentTrack = track;
    m_currentEnvelopeIndex = envelopeIndex;
    update();
}

void EnvelopeScene::addNodeAtClickPos(const QPointF &scenePos)
{
    unsigned int xPos = roundToNearestSnapPos(scenePos.x());

    if (m_envelope->contains(xPos))
        return;

    SimpleVector<QPointF> nodeVector(1);
    nodeVector.append(QPointF(xPos, scenePos.y()));

    pushUndoCommand(new AddNodesCommand(m_currentTrack, m_currentEnvelopeIndex, nodeVector, trackManager));

    // select node...
}

void EnvelopeScene::clearSelectedNodes()
{
    m_selectedNodeIndices.setSize(0);
}

void EnvelopeScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    trackManager->executeEnvelopeContextMenu(event, nodeAt(event->scenePos(), static_cast<QGraphicsView*>(event->widget()->parent())->transform()));
}

bool EnvelopeScene::copyImplementation(QDataStream &stream)
{
    if (m_selectedNodeIndices.size() == 0)
        return false;

    stream << m_selectedNodeIndices.size();

    const unsigned int *xPositions = m_envelope->getKeyArray();
    const float *yPositions = m_envelope->getValueArray();

    // find leftmost node's x position
    unsigned int leftmostNodeXPos = xPositions[m_selectedNodeIndices[0]]; // default value...
    for (int i = 0; i < m_selectedNodeIndices.size(); ++i)
    {
        if (xPositions[m_selectedNodeIndices[i]] < leftmostNodeXPos)
            leftmostNodeXPos = xPositions[m_selectedNodeIndices[i]];
    }
    stream << leftmostNodeXPos;

    // serialize the nodes
    for (int i = 0; i < m_selectedNodeIndices.size(); ++i)
    {
        stream << xPositions[m_selectedNodeIndices[i]] << yPositions[m_selectedNodeIndices[i]];
    }

    return true;
}

QUndoCommand *EnvelopeScene::deleteCommand()
{
    if (m_selectedNodeIndices.size() == 0)
        return 0;

    SimpleVector<QPointF> selectedNodes(m_selectedNodeIndices.size());
    for (int i = 0; i < m_selectedNodeIndices.size(); ++i)
    {
        selectedNodes.append(QPointF(m_envelope->keyAt(m_selectedNodeIndices[i]),
                                     m_envelope->valueAt(m_selectedNodeIndices[i])));
    }

    clearSelectedNodes();

    return new DeleteNodesCommand(m_currentTrack, m_currentEnvelopeIndex, selectedNodes, trackManager);
}

void EnvelopeScene::drawBackground(QPainter *painter, const QRectF &rect) // draws bar lines, then the envelope
{
    AbstractSequencerScene::drawBackground(painter, rect); // draw the bar lines

    const unsigned int *xPositions = m_envelope->getKeyArray();
    const float *yPositions = m_envelope->getValueArray();
    double rectRight = rect.right();

    if (m_envelope->count() == 0 || xPositions[0] > rectRight) // if nothing to draw
        return;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(m_envelopePen);

    int lastIndex = m_envelope->count() - 1;

    // find the first index to draw
    int firstIndexToDraw = 0;
    while (xPositions[firstIndexToDraw] < rect.left())
        ++firstIndexToDraw;
    if (firstIndexToDraw > 0)
        --firstIndexToDraw;

    // find the last index to draw
    int lastIndexToDraw = lastIndex;
    while (xPositions[lastIndexToDraw] > rectRight)
        --lastIndexToDraw;
    if (lastIndexToDraw < lastIndex)
        ++lastIndexToDraw;

    int i = firstIndexToDraw;

    // ==================================================== DRAW THE LINES
    QPointF fromPoint;
    QPointF toPoint(xPositions[i], yPositions[i]);
    while (i < lastIndexToDraw)
    {
        fromPoint = toPoint;
        ++i;
        toPoint = QPointF(xPositions[i], yPositions[i]);
        painter->drawLine(fromPoint, toPoint);
    }
    painter->drawLine(toPoint, QPointF(rectRight, toPoint.y()));
    // ===================================================================

    // adjust to draw partially offscreen nodes
    double rectLeft = rect.left() - 1;
    rectRight += 1;

    // ========================================= DRAW THE UNSELECTED NODES
    ++lastIndexToDraw;
    painter->setPen(m_unselectedNodePen);
    for (i = firstIndexToDraw; i < lastIndexToDraw; ++i)
    {
        QPointF point(xPositions[i], yPositions[i]);

        if (point.x() > rectLeft && point.x() < rectRight)
            painter->drawPoint(point);
    }
    // ===================================================================

    // =============================================== DRAW THE SELECTED NODES
    painter->setPen(m_selectedNodePen);
    for (int i = 0; i < m_selectedNodeIndices.size(); ++i)
    {
        QPointF point(xPositions[m_selectedNodeIndices[i]],
                      yPositions[m_selectedNodeIndices[i]]);

        if (point.x() > rectLeft && point.x() < rectRight)
            painter->drawPoint(point);
    }
    // =======================================================================
}

void EnvelopeScene::editNodeValue(int index)
{
    float value = m_envelope->valueAt(index);

    if (m_currentTrack == -1)
    {
        switch (m_currentEnvelopeIndex)
        {
        case 0: // beta
            value = getValueFromUser(value, 0, 1200, true);
            break;
        case 1: // lambda
            value = getValueFromUser(value, 0, 1, true);
            break;
        case 2: // gamma
            value = getValueFromUser(value, -1, 1, true);
            break;
        }
    }
    else
    {
        value = getValueFromUser(value, 0, 127, false);
    }

    SimpleVector<int> indexVector(1);
    indexVector.append(index);
    SimpleVector<QPointF> nodeInitialPosition(1);
    nodeInitialPosition.append(QPointF(m_envelope->keyAt(index), m_envelope->valueAt(index)));
    SimpleVector<QPointF> nodeNewPosition(1);
    nodeNewPosition.append(QPointF(m_envelope->keyAt(index), value));

    pushUndoCommand(new MoveNodesCommand(m_currentTrack, m_currentEnvelopeIndex, indexVector, nodeInitialPosition, nodeNewPosition, trackManager));
}

void EnvelopeScene::evaluateSelection(const QRectF &rect)
{
    m_selectedNodeIndices.setSize(0);

    if (rect.isNull())
        return;

    const unsigned int *xPositions = m_envelope->getKeyArray();
    const float *yPositions = m_envelope->getValueArray();

    for (int i = 0; i < m_envelope->count(); ++i)
    {
        if (rect.contains(QPointF(xPositions[i], yPositions[i])))
            m_selectedNodeIndices.appendSafely(i);
    }
}

double EnvelopeScene::getValueFromUser(double currentVal0To1, double minVal, double maxVal, bool useFloatingPoint)
{
    double newValue;
    QString title(tr("Edit Node Value"));
    QString label(tr("Value (") + QString::number(minVal) + tr(" to ") + QString::number(maxVal) + "):");
    double range = maxVal - minVal;

    if (useFloatingPoint)
        newValue = QInputDialog::getDouble(static_cast<QWidget*>(parent()), title, label, currentVal0To1 * range + minVal, minVal, maxVal, 3, 0, Qt::WindowTitleHint);
    else
        newValue = QInputDialog::getInt(static_cast<QWidget*>(parent()), title, label, currentVal0To1 * range + minVal, minVal, maxVal, 1, 0, Qt::WindowTitleHint);

    return (newValue - minVal) / range; // convert back to 0-1 range
}

void EnvelopeScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // find out which node is at click position
    int nodeIndexAtClickPos = nodeAt(event->scenePos(), static_cast<QGraphicsView*>(event->widget()->parent())->transform());

    if (nodeIndexAtClickPos == -1) // if no node there, add one
    {
        addNodeAtClickPos(event->scenePos());
    }
    else // change node's value
    {
        fadeLabelOut();

        editNodeValue(nodeIndexAtClickPos);
    }
}

void EnvelopeScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_nodeMover != NULL)
        m_nodeMover->onMouseMove(event);
}

void EnvelopeScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();

    int indexOfClickedNode = nodeAt(event->scenePos(), static_cast<QGraphicsView*>(event->widget()->parent())->transform());
    if (indexOfClickedNode == -1) // if no node
    {
        if (event->button() == Qt::LeftButton)
        {
            m_selectedNodeIndices.setSize(0);
            static_cast<EnvelopeView*>(event->widget()->parent())->startRubberBandDrag();
        }

        return; // start a double-click and/or rubber band drag
    }

    // find out if the node is already selected or not (note: this is the index of the item in the selected nodes vector, not in the envelope)
    int indexOfItemIfSelected = m_selectedNodeIndices.find(indexOfClickedNode);

    // if holding down control or shift, just toggle selected
    if (event->modifiers().testFlag(Qt::ControlModifier) || event->modifiers().testFlag(Qt::ShiftModifier)) // toggle selected
    {
        if (indexOfItemIfSelected == -1) // if not already selected
            m_selectedNodeIndices.appendSafely(indexOfClickedNode);
        else
            m_selectedNodeIndices.removeIndex(indexOfItemIfSelected);

        static_cast<EnvelopeView*>(event->widget()->parent())->startRubberBandDrag();
        return; // event is ignored... rubber band allowed
    }

    if (indexOfItemIfSelected == -1) // if not already selected
    {
        m_selectedNodeIndices.setSize(0);
        m_selectedNodeIndices.appendSafely(indexOfClickedNode);
    }

    event->accept(); // prevent a rubber band drag; want mouse move events

    if (event->button() != Qt::LeftButton) // make sure node mover isn't created if context menu is requested
        return;

    fadeLabelIn();
    m_nodeMover = new NodeMover(m_currentTrack, m_currentEnvelopeIndex, indexOfClickedNode, m_envelope, m_selectedNodeIndices, trackManager, this);
}

void EnvelopeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_nodeMover == 0)
    {
        if (event->screenPos() == event->buttonDownScreenPos(event->button()))
            roundSetAndEmitCursorPos(event->scenePos().x());
    }
    else
    {
        m_nodeMover->onMouseRelease();
        delete m_nodeMover;
        m_nodeMover = 0;
        fadeLabelOut();
    }
}

int EnvelopeScene::nodeAt(const QPointF &point, const QTransform &viewTransform) // this function could easily be sped up
{
    if (m_envelope->count() == 0)
        return -1;

    float nodeRadius = m_unselectedNodePen.width() * .5;

    double horzRadius = nodeRadius / viewTransform.m11();
    double vertRadius = nodeRadius / viewTransform.m22();

    QRectF clickRect(point.x() - horzRadius, point.y() - vertRadius, horzRadius + horzRadius, vertRadius + vertRadius);

    const unsigned int *xPositions = m_envelope->getKeyArray();
    const float *yPositions = m_envelope->getValueArray();

    for (int i = 0; i < m_envelope->count(); ++i)
    {
        if (clickRect.contains(xPositions[i], yPositions[i]))
            return i;
    }

    return -1;
}

QUndoCommand *EnvelopeScene::pasteCommand(QDataStream &stream, int numItems)
{
    m_selectedNodeIndices.setSize(0);
    double leftmostCopiedNodeXPos;
    stream >> leftmostCopiedNodeXPos;

    double pastedNodeOffset = cursorPos() - leftmostCopiedNodeXPos;

    SimpleVector<QPointF> nodesToPaste(numItems);

    for (int i = 0; i < numItems; ++i)
    {
        unsigned int pos;
        float value;
        stream >> pos >> value;
        pos += pastedNodeOffset;

        while (m_envelope->contains(pos))
            pos += snapSize();

        nodesToPaste.append(QPointF(pos, value));
    }

    return new AddNodesCommand(m_currentTrack, m_currentEnvelopeIndex, nodesToPaste, trackManager);
}

void EnvelopeScene::selectAll()
{
    m_selectedNodeIndices = SimpleVector<int>(m_envelope->count());

    for (int i = 0; i < m_envelope->count(); ++i)
    {
        m_selectedNodeIndices.append(i);
    }

    update();
}

void EnvelopeScene::setNodeSelected(int index)
{
    m_selectedNodeIndices.appendSafely(index);
}
