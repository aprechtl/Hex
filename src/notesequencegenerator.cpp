#include "notesequencegenerator.h"
#include "dynamictonality.h"
#include "hexsettings.h"
#include "note.h"
#include "sequencerevent.h"
#include "sortalgorithms.h"
#include "simplevector.h"

NoteSequenceGenerator::NoteSequenceGenerator(const SimpleVector<QGraphicsItem*> &notes)
    : m_eventArray(0), m_numEvents(0)
{
    if (notes.size() == 0)
        return;

    m_eventArray = new SequencerEvent[notes.size() + notes.size()];

    // convert notes to events
    for (int i = 0; i < notes.size(); ++i)
    {
        Note *note = static_cast<Note*>(notes[i]);

        short int j, k;
        HexSettings::convertNoteLaneIndexToJK(note->getLaneIndex(), j, k);

        unsigned char channel, number;
        if (!DynamicTonality::captureMIDIFromJK(j, k, channel, number))
            continue; // ignore notes that DT can't convert

        channel += 143;

        m_eventArray[m_numEvents].setData(SequencerEvent::MIDINoteOn, note->pos().x(), channel, number, note->getVelocity(), note->track());
        ++m_numEvents;

        m_eventArray[m_numEvents].setData(SequencerEvent::MIDINoteOff, note->pos().x() + note->boundingRect().width(), channel, number, 0, note->track());
        ++m_numEvents;
    }

    insertionSort(m_eventArray, m_numEvents, SequencerEvent::compareEvents);
}
