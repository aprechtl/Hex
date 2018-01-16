#ifndef SEQUENCEREVENTARRAYCOMBINER_H
#define SEQUENCEREVENTARRAYCOMBINER_H

struct SequencerEvent;

class SequencerEventArrayCombiner
{
public:
    SequencerEventArrayCombiner();
    void addSortedArray(SequencerEvent *eventArray, int numEvents); // takes ownership of combinedArray

    // inline methods
    SequencerEvent *eventArray() const {return m_eventArray;}
    int numEvents() const {return m_numEvents;}

private:
    SequencerEvent *m_eventArray;
    int m_numEvents;
};

#endif
