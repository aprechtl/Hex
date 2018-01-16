#ifndef TRACK_H
#define TRACK_H
#include "envelopedata.h"
#include "simplevector.h"
#include <QtCore/QString>

class MIDIOutput;
class Note;
class QActionGroup;
class QMenu;

struct Track
{
    Track() : id(0), outputPort(0), menu(0), trackTypeActionGroup(0) {} // default constructor

    int id; // each track has an ID that represents its index in the track manager's complete list of tracks (including removed tracks)
    MIDIOutput *outputPort;
    QMenu *menu; // the title of the menu is the name of the track (it's easier if the track has a pointer to the menu that to the list widget item)
    QActionGroup *trackTypeActionGroup;
    SimpleVector<Note*> notes;
    SimpleVector<EnvelopeData> envelopeDataVector;
};

#endif // TRACK_H
