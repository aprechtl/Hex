#ifndef SEQUENCERSCENE_H
#define SEQUENCERSCENE_H
#include "abstractsequencerscene.h"
#include <QtGui/QBrush>
#include <QtGui/QPen>

template <class QGraphicsItem>
class SimpleVector;

class Note;
class QDataStream;
struct LatticeData;
struct NoteStruct;

class SequencerScene : public AbstractSequencerScene
{
public:
    SequencerScene(LatticeData *latticeData, BarLineDrawer *barLineDrawer, QUndoStack *undoStack, QWidget *view);
    QGraphicsItem *addNote(const NoteStruct &note);
    int findClosestNoteLane(double yPos) const;
    void restoreNotes(QDataStream &in);
    void saveNotes(QDataStream &out) const;
    void selectAll();
    void setCurrentTrack(int track);
    void updateNotePositions();

    void insertTrack(int index, const SimpleVector<Note*> &itemsInTrack);
    SimpleVector<Note*> removeTrack(int index);

    // inline methods
    int currentTrack() const {return m_currentTrack;}
    unsigned char getDefaultVelocity() const {return defaultVelocity;}
    const QBrush &getActiveNoteBrush(int velocity) const {return activeNoteBrushes[velocity];}
    const QBrush &getInactiveNoteBrush(int velocity) const {return inactiveNoteBrushes[velocity];}
    const QPen& getSelectedNotePen() const {return selectedNotePen;}
    const QPen& getUnselectedNotePen() const {return unselectedNotePen;}
    void setDarkLaneColor(const QColor &color) {darkLaneBrush.setColor(color);}
    void setDefaultVelocity(unsigned char vel) {defaultVelocity = vel;}
    void setInactiveNoteBrushOpacity(int opacity = 255) {inactiveNoteBrushOpacity = opacity; updateNoteBrushColors();}
    void setLightLaneColor(const QColor &color) {lightLaneBrush.setColor(color);}
    void setMaxVelocityColor(const QColor &color) {activeNoteBrushes[127] = color; updateNoteBrushColors();}
    void setMinVelocityColor(const QColor &color) {activeNoteBrushes[0] = color; updateNoteBrushColors();}
    void setPressedLaneColor(const QColor &color) {pressedLaneBrush.setColor(color);}
    void setSelectedNoteColor(const QColor &color) {selectedNotePen.setColor(color);}
    void setUnselectedNoteColor(const QColor &color) {unselectedNotePen.setColor(color);}


protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    bool copyImplementation(QDataStream &stream);
    QUndoCommand *deleteCommand();
    QString mimeType() const {return "hex/notes";}
    QUndoCommand *pasteCommand(QDataStream &stream, int numItems);

    void drawLineSet(QPainter *painter, const QRectF &rect, const SimpleVector<unsigned int> &indices);
    int findTopmostNoteLane(double yPos) const;
    void updateNoteBrushColors();

    // these are initialized in the initializer list
    LatticeData *latticeData;
    Note *noteBeingCreated;
//    bool isCreatingANote;
    unsigned char defaultVelocity;
    QBrush darkLaneBrush;
    QBrush lightLaneBrush;
    QBrush pressedLaneBrush;
    int inactiveNoteBrushOpacity;
    int m_currentTrack;

    QBrush activeNoteBrushes[128];
    QBrush inactiveNoteBrushes[128];
    QPen unselectedNotePen;
    QPen selectedNotePen;
};

#endif
