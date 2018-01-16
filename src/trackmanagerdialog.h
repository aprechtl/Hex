#ifndef TRACKMANAGERDIALOG_H
#define TRACKMANAGERDIALOG_H
#include "envelopedata.h"
#include "sequencerevent.h"
#include "track.h"
#include <QtWidgets/QDialog>

class EnvelopeScene;
class MIDIPortManager;
class QComboBox;
class QDataStream;
class QGraphicsSceneContextMenuEvent;
class QListWidget;
class QMenu;
class SequencerScene;

class TrackManagerDialog : public QDialog
{
public:
    TrackManagerDialog(MIDIPortManager *portManager, SequencerScene *sequencerScene, EnvelopeScene *envelopeScene, QWidget *parent = 0);
    int currentTrack() const;
    QMenu *menu() const {return m_menu;}
    void setCurrentTrack(int track);
    QComboBox *trackNameComboBox() const {return m_trackNameComboBox;}

    // meta methods
    void clear();
    SimpleVector<SequencerEvent> gatherSequencerEvents(double millisecondsPerTick) const;
    void restoreData(QDataStream &stream);
    void saveData(QDataStream &stream);

    // node methods (note: global envelopes are tagged with track = -1)
    void addNode(int track, int envelope, unsigned int pos, float value);
    void moveNode(int track, int envelope, int nodeIndex, unsigned int newPos, float newValue); // must not cross over any other node
    void removeNode(int track, int envelope, unsigned int pos);

    // envelope methods
    void addMIDICCEnvelope(int track, int index, const EnvelopeData &envelopeData);
    void changeMIDICCEnvelopeData(int track, int index, unsigned char newChannel, unsigned char newMIDICCNumber);
    EnvelopeData getMIDICCEnvelope(int track, int index) const;
    void moveMIDICCEnvelope(int track, int oldIndex, int newIndex);
    void removeMIDICCEnvelope(int track, int index);

    // track methods
    void addNewTrack();
    void moveTrack (int oldIndex, int newIndex, bool moveListWidgetItem);
    int removeTrack(int track); // returns the track's ID for easy restoration later
    void restoreTrack(int trackID, int trackIndex);

    // action slots
    void onActionAddNewEnvelope();
    void onActionAddNewTrack();
    void onActionEditEnvelope();
    void onActionRemoveEnvelope();
    void onActionRemoveTrack();
    void onTrackNameEditingFinished();

    void executeEnvelopeContextMenu(QGraphicsSceneContextMenuEvent *event, int indexOfClickedNode);

    static const int maxNumTracks = 32;
    static const int numGlobalEnvelopes = 3;

private:
    QString envelopeName(int track, int envelopeIndex) const;
    bool getChannelAndMIDICCNumber(unsigned char &channel, unsigned char &CCNumber, unsigned char defaultChannel = 0, unsigned char defaultCCNumber = 0);
    void refreshMIDICCEnvelopes();
    void setUpTrackSubMenu(Track &track, const QString &title, int trackType);

    // data
    int m_numTotalTracks;
    Track m_allTracks[maxNumTracks]; // includes deleted tracks (so that deleting can be undone)
    SimpleVector<Track*> m_currentTracks;
    SimpleMap<unsigned int, float> m_globalEnvelopes[numGlobalEnvelopes];

    // widgets
    QListWidget *m_trackListWidget;
    QListWidget *m_MIDICCListWidget;
    QComboBox *m_trackNameComboBox;
    QMenu *m_menu;

    // other
    MIDIPortManager *m_portManager;
    SequencerScene *m_sequencerScene;
    EnvelopeScene *m_envelopeScene;
};

#endif // TRACKMANAGERDIALOG_H
