#ifndef DRAGHANDLERS_H
#define DRAGHANDLERS_H
#include "simplemap.h"
#include "simplevector.h"
#include <QtCore/QPointF>

class EnvelopeScene;
class Note;
class QGraphicsItem;
class QGraphicsSceneMouseEvent;
class SequencerScene;
class TrackManagerDialog;

// ###########################################################################
// ###########################################################################

class DragHandler
{
public:
    void onMouseMove(QGraphicsSceneMouseEvent *event);
    void onMouseRelease();

protected:
    DragHandler();
    virtual ~DragHandler() {}

private:
    virtual void onMouseDragDerived(QGraphicsSceneMouseEvent *) = 0;
    virtual void onDraggedMouseReleaseDerived() = 0;
    virtual void onUndraggedMouseReleaseDerived() = 0;

    bool dragged;
};

class NoteDragHandler : public DragHandler
{
public:
    NoteDragHandler(QGraphicsItem *note);

protected:
    SequencerScene *scene;
    QGraphicsItem *draggedItem;

private:
    void onUndraggedMouseReleaseDerived();
};

// ###########################################################################
// ###########################################################################

class NodeMover : public DragHandler
{
public:
    NodeMover(int track, int envelopeIndex, int indexOfDraggedNode,
              SimpleMap<unsigned int, float> *envelope, const SimpleVector<int> &draggedNodeIndices, TrackManagerDialog *trackManager, EnvelopeScene *scene);

private:
    void onMouseDragDerived(QGraphicsSceneMouseEvent *event);
    void onDraggedMouseReleaseDerived();
    void onUndraggedMouseReleaseDerived();
    void updateLabel();

    int track;
    int envelopeIndex;
    int indexOfDraggedNode;
    SimpleMap<unsigned int, float> *envelope;
    TrackManagerDialog *trackManager;
    EnvelopeScene *scene;
    SimpleVector<int> draggedNodeIndices;
    SimpleVector<QPointF> draggedNodeInitialPositions;
    unsigned int minXPos, maxXPos;
    double minYPos, maxYPos;
};

// ###########################################################################
// ###########################################################################

class HorizontalNoteMover : public NoteDragHandler
{
public:
    HorizontalNoteMover(QGraphicsItem *thisNote, double initialClickXOffset);

private:
    void onMouseDragDerived(QGraphicsSceneMouseEvent *event);
    void onDraggedMouseReleaseDerived();

    double initialClickXOffset;
    SimpleVector<QGraphicsItem*> notesToMove;
    SimpleVector<float> initialPositions;
    double minXPos;
};

// ###########################################################################
// ###########################################################################

class NoteMover : public NoteDragHandler
{
public:
    NoteMover(Note *note, double initialClickXOffset);

private:
    void onMouseDragDerived(QGraphicsSceneMouseEvent *event);
    void onDraggedMouseReleaseDerived();

    double initialClickXOffset;
    unsigned short int initialNoteLaneIndex;
    SimpleVector<QGraphicsItem*> notesToMove;
    SimpleVector<float> initialPositions;
    double minXPos; // used to ensure note's aren't dragged too far left
};

// ###########################################################################
// ###########################################################################

class NoteResizer : public NoteDragHandler
{
public:
    NoteResizer(Note *note);

private:
    void onMouseDragDerived(QGraphicsSceneMouseEvent *event);
    void onDraggedMouseReleaseDerived();

    double draggedNoteOldEndPos;
    SimpleVector<QGraphicsItem*> notesToResize;
    SimpleVector<float> initialLengths;
    double minXPos;
};

// ###########################################################################
// ###########################################################################

class NoteVelocityAdjuster : public NoteDragHandler
{
public:
    NoteVelocityAdjuster(Note *note, double initialClickYOffset);

private:
    void onMouseDragDerived(QGraphicsSceneMouseEvent *event);
    void onDraggedMouseReleaseDerived();

    unsigned char clickedNoteInitialVelocity;
    double initialClickYOffset;
    SimpleVector<QGraphicsItem *> notes;
    SimpleVector<unsigned char> initialVelocities;
};

#endif
