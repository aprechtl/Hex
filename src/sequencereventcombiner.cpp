#include "sequencereventcombiner.h"
#include "sequencerevent.h"
#include "sortalgorithms.h"

SequencerEventArrayCombiner::SequencerEventArrayCombiner()
    : m_eventArray(0), m_numEvents(0)
{
}

void SequencerEventArrayCombiner::addSortedArray(SequencerEvent *array, int count)
{
    if (count == 0)
        return;

    if (m_numEvents == 0) // if current array is empty, no need to sort anything
    {
        m_numEvents = count;
        m_eventArray = array;
        return;
    }

    SequencerEvent *newArray = mergeSort(m_eventArray, m_numEvents, array, count, SequencerEvent::compareEvents);

    delete [] m_eventArray;
    delete [] array;

    m_eventArray = newArray;
    m_numEvents += count;
}
