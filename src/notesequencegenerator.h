#ifndef NOTESEQUENCEGENERATOR_H
#define NOTESEQUENCEGENERATOR_H

template <class QGraphicsItem>
class SimpleVector;

class QGraphicsItem;
struct SequencerEvent;

class NoteSequenceGenerator
{
public:
    NoteSequenceGenerator(const SimpleVector<QGraphicsItem*> &notes);

    // inline methods
    SequencerEvent *eventArray() const {return m_eventArray;}
    int numEvents() const {return m_numEvents;}

private:
    SequencerEvent *m_eventArray;
    int m_numEvents;
};

#endif
