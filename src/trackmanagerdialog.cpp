#include "trackmanagerdialog.h"
#include "envelopecommands.h"
#include "envelopegenerator.h"
#include "envelopescene.h"
#include "lineeditdelegate.h"
#include "midiportmanager.h"
#include "notesequencegenerator.h"
#include "sequencereventcombiner.h"
#include "sequencerscene.h"
#include "trackcommands.h"
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGraphicsSceneContextMenuEvent>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>

// ===================================================== QDATASTREAM OPERATORS
QDataStream &operator>>(QDataStream &in, SimpleMap<unsigned int, float> &envelope)
{
    int numNodes;
    in >> numNodes;

    unsigned int *positions = new unsigned int[numNodes];
    float *values = new float[numNodes];

    for (int i = 0; i < numNodes; ++i)
        in >> positions[i] >> values[i];

    envelope = SimpleMap<unsigned int, float>(positions, values, numNodes);
    return in;
}

QDataStream &operator<<(QDataStream &out, const SimpleMap<unsigned int, float> &envelope)
{
    out << envelope.count();

    for (int i = 0; i < envelope.count(); ++i)
        out << envelope.keyAt(i) << envelope.valueAt(i);

    return out;
}

// ===========================================================================

TrackManagerDialog::TrackManagerDialog(MIDIPortManager *portManager, SequencerScene *sequencerScene, EnvelopeScene *envelopeScene, QWidget *parent)
    : QDialog(parent),
      m_numTotalTracks(0),
      m_portManager(portManager),
      m_sequencerScene(sequencerScene),
      m_envelopeScene(envelopeScene)
{
    setWindowTitle(tr("Track and Envelope Setup"));

    // track list widget
    m_trackListWidget = new QListWidget;
    m_trackListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    QPushButton *addTrackButton = new QPushButton(tr("Add"));
    QPushButton *removeTrackButton = new QPushButton(tr("Remove"));
    QHBoxLayout *trackButtonLayout = new QHBoxLayout;
    trackButtonLayout->addWidget(addTrackButton);
    trackButtonLayout->addWidget(removeTrackButton);
    connect(addTrackButton, &QPushButton::clicked, this, &TrackManagerDialog::onActionAddNewTrack);
    connect(removeTrackButton, &QPushButton::clicked, this, &TrackManagerDialog::onActionRemoveTrack);
    connect(m_trackListWidget, &QListWidget::currentRowChanged, this, &TrackManagerDialog::setCurrentTrack);
    connect(m_trackListWidget->model(), &QAbstractItemModel::rowsMoved, [=](const QModelIndex &, int sourceStart, int, const QModelIndex &, int destinationRow)
    { m_sequencerScene->pushUndoCommand(new ChangeTrackOrderCommand(sourceStart, (destinationRow > sourceStart) ? destinationRow - 1: destinationRow, this));});
    LineEditDelegate *delegate = new LineEditDelegate(m_trackListWidget); // use a custom delegate that emits the editingFinished signal
    connect(delegate, &LineEditDelegate::editingFinished, this, &TrackManagerDialog::onTrackNameEditingFinished);
    m_trackListWidget->setItemDelegate(delegate);

    // track name combo box
    m_trackNameComboBox = new QComboBox;
    m_trackNameComboBox->setMinimumWidth(120);
    m_trackNameComboBox->setEditable(true);
    m_trackNameComboBox->setInsertPolicy(QComboBox::InsertAtCurrent);
    m_trackNameComboBox->setContextMenuPolicy(Qt::NoContextMenu);
    m_trackNameComboBox->setModel(m_trackListWidget->model());
    connect(m_trackNameComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            m_trackListWidget, static_cast<void (QListWidget::*)(int)>(&QListWidget::setCurrentRow));
    connect(m_trackNameComboBox->lineEdit(), &QLineEdit::editingFinished, this, &TrackManagerDialog::onTrackNameEditingFinished);
#ifdef Q_OS_MAC
    m_trackNameComboBox->lineEdit()->setContentsMargins(2, 0, 0, 0);
#endif

    // midi cc list widget
    m_MIDICCListWidget = new QListWidget;
    m_MIDICCListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    QPushButton *addEnvelopeButton = new QPushButton(tr("Add"));
    QPushButton *editEnvelopeButton = new QPushButton(tr("Edit"));
    QPushButton *removeEnvelopeButton = new QPushButton(tr("Remove"));
    QHBoxLayout *envelopeButtonLayout = new QHBoxLayout;
    envelopeButtonLayout->addWidget(addEnvelopeButton);
    envelopeButtonLayout->addWidget(editEnvelopeButton);
    envelopeButtonLayout->addWidget(removeEnvelopeButton);
    connect(addEnvelopeButton, &QPushButton::clicked, this, &TrackManagerDialog::onActionAddNewEnvelope);
    connect(editEnvelopeButton, &QPushButton::clicked, this, &TrackManagerDialog::onActionEditEnvelope);
    connect(removeEnvelopeButton, &QPushButton::clicked, this, &TrackManagerDialog::onActionRemoveEnvelope);
    connect(m_MIDICCListWidget, &QListWidget::currentRowChanged, [=](int row)
    {
        if (row == -1) {m_envelopeScene->setMIDICCEnvelope(&m_globalEnvelopes[0], -1, 0);}
        else {m_envelopeScene->setMIDICCEnvelope(&m_currentTracks[currentTrack()]->envelopeDataVector[row].envelope, currentTrack(), row);}
    });
    connect(m_MIDICCListWidget->model(), &QAbstractItemModel::rowsMoved, [=](const QModelIndex &, int sourceStart, int, const QModelIndex &, int destinationRow)
    { m_sequencerScene->pushUndoCommand(new ChangeEnvelopeOrderCommand(this, currentTrack(), sourceStart, (destinationRow > sourceStart) ? destinationRow - 1: destinationRow)); });

    // layout
    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(new QLabel(tr("Tracks:")), 0, 0, 1, 1, Qt::AlignBottom);
    layout->addWidget(new QLabel(tr("MIDI CC Envelopes:")), 0, 1, 1, 1);
    layout->addWidget(m_trackListWidget, 1, 0, 1, 1);
    layout->addWidget(m_MIDICCListWidget, 1, 1, 1, 1);
    layout->addLayout(trackButtonLayout, 2, 0, 1, 1);
    layout->addLayout(envelopeButtonLayout, 2, 1, 1, 1);

    // menu
    m_menu = new QMenu(tr("Tracks"), this);
    connect(m_menu->addAction(tr("Add New Track")), &QAction::triggered, this, &TrackManagerDialog::onActionAddNewTrack);
    QAction *actionShowThis = new QAction(tr("Track/Envelope Setup..."), m_menu);
    actionShowThis->setShortcut(QKeySequence(tr("Ctrl+t")));
    m_menu->addAction(actionShowThis);
    connect(actionShowThis, &QAction::triggered, this, &QDialog::show);
    m_menu->addSeparator();
}

void TrackManagerDialog::addMIDICCEnvelope(int track, int index, const EnvelopeData &envelopeData)
{
    m_currentTracks[track]->envelopeDataVector.insertSafely(index, envelopeData);
    setCurrentTrack(track);
}

void TrackManagerDialog::addNewTrack()
{
    m_currentTracks.appendSafely(&m_allTracks[m_numTotalTracks]);
    m_allTracks[m_numTotalTracks].outputPort = m_portManager->createOutput();
    setUpTrackSubMenu(m_allTracks[m_numTotalTracks], tr("Track ") + QString::number(m_currentTracks.size()), 0);
    QListWidgetItem *item = new QListWidgetItem(m_allTracks[m_numTotalTracks].menu->title());
    item->setFlags (item->flags() | Qt::ItemIsEditable);
    m_trackListWidget->addItem(item);
    ++m_numTotalTracks;
    setCurrentTrack(m_currentTracks.size() - 1);
}

void TrackManagerDialog::addNode(int track, int envelope, unsigned int pos, float value)
{
    if (track == -1)
        m_globalEnvelopes[envelope].insert(pos, value);
    else
        m_currentTracks[track]->envelopeDataVector[envelope].envelope.insert(pos, value);

    m_envelopeScene->update();
}

void TrackManagerDialog::changeMIDICCEnvelopeData(int track, int index, unsigned char newChannel, unsigned char newMIDICCNumber)
{
    m_currentTracks[track]->envelopeDataVector[index].MIDIChannel = newChannel;
    m_currentTracks[track]->envelopeDataVector[index].MIDICCNumber = newMIDICCNumber;
    setCurrentTrack(track);
}

void TrackManagerDialog::clear()
{
    for (int i = 0; i < m_numTotalTracks; ++i)
    {
        m_allTracks[i].id = i;
        m_allTracks[i].trackTypeActionGroup = 0;
        m_allTracks[i].outputPort = NULL;
        m_allTracks[i].notes = SimpleVector<Note*>();
        m_allTracks[i].envelopeDataVector = SimpleVector<EnvelopeData>();
        delete m_allTracks[i].menu;
        m_allTracks[i].menu = NULL;
    }

    m_currentTracks = SimpleVector<Track*>(16);
    m_numTotalTracks = 0;

    m_portManager->clearOutputs();
    m_trackListWidget->clear();
    m_MIDICCListWidget->clear();
    m_sequencerScene->clear();
}

int TrackManagerDialog::currentTrack() const
{
    return m_trackListWidget->currentRow();
}

QString TrackManagerDialog::envelopeName(int track, int envelopeIndex) const
{
    return QString(tr("%1, MIDI Channel %2, MIDI CC %3")).arg(m_currentTracks[track]->menu->title(),
            QString::number(m_currentTracks[track]->envelopeDataVector[envelopeIndex].MIDIChannel),
            QString::number(m_currentTracks[track]->envelopeDataVector[envelopeIndex].MIDICCNumber));
}

void TrackManagerDialog::executeEnvelopeContextMenu(QGraphicsSceneContextMenuEvent *event, int indexOfClickedNode)
{
    QMenu menu;

    if (indexOfClickedNode == -1) // if no node at the click pos
        menu.addAction(tr("Add Node"))->setData(-1);
    else
    {
        menu.addAction(tr("Edit Node Value"))->setData(-2);
        menu.addAction(tr("Delete Node"))->setData(-3);
    }
    menu.addSeparator();

    QAction *globalEnvelopeActions[3];
    globalEnvelopeActions[0] = menu.addAction(tr("Generator Envelope"));
    globalEnvelopeActions[0]->setData(-5);
    globalEnvelopeActions[1] = menu.addAction(tr("Harmonicity Envelope"));
    globalEnvelopeActions[1]->setData(-6);
    globalEnvelopeActions[2] = menu.addAction(tr("Just Intonation Envelope"));
    globalEnvelopeActions[2]->setData(-7);
    menu.addSeparator();

    int numMIDICCEnvelopes = m_MIDICCListWidget->count();
    QAction **MIDICCEnvelopeActions = (numMIDICCEnvelopes > 0) ? new QAction *[numMIDICCEnvelopes] : 0;
    for (int i = 0; i < numMIDICCEnvelopes; ++i)
    {
        MIDICCEnvelopeActions[i] = menu.addAction(m_MIDICCListWidget->item(i)->text());
        MIDICCEnvelopeActions[i]->setData(i);
    }

    QAction *actionToCheck = (m_envelopeScene->currentTrack() == -1)
            ? globalEnvelopeActions[m_envelopeScene->currentEnvelopeIndex()]
            : MIDICCEnvelopeActions[m_envelopeScene->currentEnvelopeIndex()];

    actionToCheck->setCheckable(true);
    actionToCheck->setChecked(true);

    menu.addSeparator();

    menu.addAction(tr("Track/Envelope Setup..."))->setData(-4);

    // #######################################################################
    QAction *selectedAction = menu.exec(event->screenPos());
    // #######################################################################

    if (selectedAction != NULL)
    {
        int selectedActionID = selectedAction->data().toInt();

        switch (selectedActionID)
        {
        case -1: m_envelopeScene->addNodeAtClickPos(event->scenePos()); break;
        case -2: m_envelopeScene->editNodeValue(indexOfClickedNode); break;
        case -3: m_envelopeScene->deleteSelectedItems(); break;
        case -4: show(); break;
        case -5: m_envelopeScene->setMIDICCEnvelope(&m_globalEnvelopes[0], -1, 0); m_MIDICCListWidget->clearSelection(); break;
        case -6: m_envelopeScene->setMIDICCEnvelope(&m_globalEnvelopes[1], -1, 1); m_MIDICCListWidget->clearSelection(); break;
        case -7: m_envelopeScene->setMIDICCEnvelope(&m_globalEnvelopes[2], -1, 2); m_MIDICCListWidget->clearSelection(); break;
        default: m_MIDICCListWidget->setCurrentRow(selectedActionID);
        }
    }

    delete [] MIDICCEnvelopeActions;
}

SimpleVector<SequencerEvent> TrackManagerDialog::gatherSequencerEvents(double millisecondsPerTick) const
{
    SequencerEventArrayCombiner combiner;

    NoteSequenceGenerator noteSequenceGenerator(m_sequencerScene->items());
    combiner.addSortedArray(noteSequenceGenerator.eventArray(), noteSequenceGenerator.numEvents());

    FloatEnvelopeGenerator betaEnvelopeGenerator(&m_globalEnvelopes[0], millisecondsPerTick, SequencerEvent::Generator);
    betaEnvelopeGenerator.generate();
    combiner.addSortedArray(betaEnvelopeGenerator.eventArray(), betaEnvelopeGenerator.numEvents());

    MIDICCEnvelopeGenerator lambdaEnvelopeGenerator(&m_globalEnvelopes[1], millisecondsPerTick, 56, 1, 0, SequencerEvent::Harmonicity);
    lambdaEnvelopeGenerator.generate();
    combiner.addSortedArray(lambdaEnvelopeGenerator.eventArray(), lambdaEnvelopeGenerator.numEvents());

    MIDICCEnvelopeGenerator gammaEnvelopeGenerator(&m_globalEnvelopes[2], millisecondsPerTick, 57, 1, 0, SequencerEvent::JI);
    gammaEnvelopeGenerator.generate();
    combiner.addSortedArray(gammaEnvelopeGenerator.eventArray(), gammaEnvelopeGenerator.numEvents());

    // gather the MIDI CC envelopes for each track
    for (int i = 0; i < m_currentTracks.size(); ++i)
    {
        for (int j = 0; j < m_currentTracks[i]->envelopeDataVector.size(); ++j)
        {
            MIDICCEnvelopeGenerator envelopeGenerator(&m_currentTracks[i]->envelopeDataVector[j].envelope,
                                                      millisecondsPerTick,
                                                      m_currentTracks[i]->envelopeDataVector[j].MIDICCNumber,
                                                      m_currentTracks[i]->envelopeDataVector[j].MIDIChannel,
                                                      i,
                                                      SequencerEvent::MIDICC);
            envelopeGenerator.generate();
            combiner.addSortedArray(envelopeGenerator.eventArray(), envelopeGenerator.numEvents());
        }
    }

    return SimpleVector<SequencerEvent>(combiner.eventArray(), combiner.numEvents());
}

bool TrackManagerDialog::getChannelAndMIDICCNumber(unsigned char &channel, unsigned char &CCNumber,
                                                   unsigned char defaultChannel, unsigned char defaultCCNumber)
{
    QDialog dialog(this, Qt::WindowTitleHint);
    dialog.setWindowTitle(tr("Envelope Setup"));
    QSpinBox channelSpinBox;
    channelSpinBox.setRange(0, 127);
    channelSpinBox.setValue(defaultChannel);
    QSpinBox ccNumberSpinBox;
    ccNumberSpinBox.setRange(0, 127);
    ccNumberSpinBox.setValue(defaultCCNumber);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QFormLayout layout(&dialog);
    layout.addRow(tr("MIDI Channel: "), &channelSpinBox);
    layout.addRow(tr("MIDI CC: "), &ccNumberSpinBox);
    layout.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Rejected)
        return false;

    channel = channelSpinBox.value();
    CCNumber = ccNumberSpinBox.value();
    return true;
}

EnvelopeData TrackManagerDialog::getMIDICCEnvelope(int track, int index) const
{
    return m_currentTracks[track]->envelopeDataVector[index];
}

void TrackManagerDialog::moveMIDICCEnvelope(int track, int oldIndex, int newIndex)
{
    EnvelopeData data = m_currentTracks[track]->envelopeDataVector[oldIndex];
    m_currentTracks[track]->envelopeDataVector.removeIndex(oldIndex);
    m_currentTracks[track]->envelopeDataVector.insertSafely(newIndex, data);
    refreshMIDICCEnvelopes();
}

void TrackManagerDialog::moveNode(int track, int envelope, int nodeIndex, unsigned int newPos, float newValue)
{
    if (track == -1)
        m_globalEnvelopes[envelope].replaceKeyAndValueAt(nodeIndex, newPos, newValue);
    else
        m_currentTracks[track]->envelopeDataVector[envelope].envelope.replaceKeyAndValueAt(nodeIndex, newPos, newValue);
    m_envelopeScene->update();
}

void TrackManagerDialog::moveTrack(int oldIndex, int newIndex, bool moveListWidgetItem)
{
    m_menu->removeAction(m_currentTracks[oldIndex]->menu->menuAction());
    m_currentTracks[oldIndex]->notes = m_sequencerScene->removeTrack(oldIndex); // remove and save the notes
    Track *track = m_currentTracks[oldIndex];
    m_currentTracks.removeIndex(oldIndex);

    if (moveListWidgetItem)
        m_trackListWidget->insertItem(newIndex, m_trackListWidget->takeItem(oldIndex));

    m_currentTracks.insertSafely(newIndex, track);
    QAction *actionToBeAfterThisTrackMenu = (newIndex - 1 >= m_currentTracks.size()) ? 0 : m_currentTracks[newIndex + 1]->menu->menuAction();
    m_menu->insertMenu(actionToBeAfterThisTrackMenu, m_currentTracks[newIndex]->menu);
    m_sequencerScene->insertTrack(newIndex, m_currentTracks[newIndex]->notes); // insert the notes into the sequencer

    setCurrentTrack(newIndex);
}

void TrackManagerDialog::onActionAddNewEnvelope()
{
    unsigned char channel, ccNumber;
    if (!getChannelAndMIDICCNumber(channel, ccNumber))
            return;

    EnvelopeData data = {channel, ccNumber, SimpleMap<unsigned int, float>()};
    m_sequencerScene->pushUndoCommand(new AddEnvelopeCommand(this, currentTrack(), m_currentTracks[currentTrack()]->envelopeDataVector.size(), data));
}

void TrackManagerDialog::onActionAddNewTrack()
{
    if (m_numTotalTracks >= maxNumTracks)
    {
        QMessageBox::warning(this, tr("Error"), QString(tr("No more tracks can be added."
                                                           "The maximum number of tracks (including ones that have been deleted during this session) is %1. "
                                                           "Try saving and re-opening the project.")).arg(QString::number(maxNumTracks)));
        return;
    }
    m_sequencerScene->pushUndoCommand(new AddNewTrackCommand(this));
}

void TrackManagerDialog::onActionEditEnvelope()
{
    if (m_MIDICCListWidget->currentRow() == -1)
        return;

    const EnvelopeData &data = m_currentTracks[currentTrack()]->envelopeDataVector[m_MIDICCListWidget->currentRow()];

    unsigned char channel, ccNumber;
    if (!getChannelAndMIDICCNumber(channel, ccNumber, data.MIDIChannel, data.MIDICCNumber))
        return;

    m_sequencerScene->pushUndoCommand(new ChangeEnvelopeCommand(this, currentTrack(), m_MIDICCListWidget->currentRow(), data.MIDIChannel, channel, data.MIDICCNumber, ccNumber));
}

void TrackManagerDialog::onActionRemoveEnvelope()
{
    if (m_MIDICCListWidget->currentRow() == -1)
        return;
    m_sequencerScene->pushUndoCommand(new RemoveEnvelopeCommand(this, currentTrack(), m_MIDICCListWidget->currentRow()));
}

void TrackManagerDialog::onActionRemoveTrack()
{
    if (m_currentTracks.size() == 1)
    {
        QMessageBox::warning(this, tr("Error"), tr("There must be at least one track in the project."));
        return;
    }

    if (m_trackListWidget->currentRow() == -1)
        return;

    m_sequencerScene->pushUndoCommand(new RemoveTrackCommand(m_trackListWidget->currentRow(), this));
}

void TrackManagerDialog::onTrackNameEditingFinished()
{
    m_trackNameComboBox->clearFocus();
    m_currentTracks[currentTrack()]->menu->setTitle(m_trackListWidget->item(currentTrack())->text());
}

void TrackManagerDialog::refreshMIDICCEnvelopes()
{
    m_MIDICCListWidget->blockSignals(true);
    m_MIDICCListWidget->clear();
    for (int i = 0; i < m_currentTracks[currentTrack()]->envelopeDataVector.size(); ++i)
    {
        m_MIDICCListWidget->addItem(envelopeName(currentTrack(), i));
    }

    if (m_envelopeScene->currentTrack() != -1) // if currently viewing a MIDI CC envelope
        m_MIDICCListWidget->item(m_envelopeScene->currentEnvelopeIndex())->setSelected(true);
    m_MIDICCListWidget->blockSignals(false);
}

void TrackManagerDialog::removeMIDICCEnvelope(int track, int index)
{
    m_currentTracks[track]->envelopeDataVector.removeIndex(index);

    if (track == currentTrack())
        refreshMIDICCEnvelopes();
}

void TrackManagerDialog::removeNode(int track, int envelope, unsigned int pos)
{
    if (track == -1)
        m_globalEnvelopes[envelope].remove(pos);
    else
        m_currentTracks[track]->envelopeDataVector[envelope].envelope.remove(pos);
}

int TrackManagerDialog::removeTrack(int track)
{
    int trackID = m_currentTracks[track]->id;
    m_currentTracks.removeIndex(track);
    m_allTracks[trackID].outputPort->closePort();
    m_menu->removeAction(m_allTracks[trackID].menu->menuAction());
    m_allTracks[trackID].notes = m_sequencerScene->removeTrack(track); // remove and save the notes
    delete m_trackListWidget->takeItem(track);
    return trackID;
}

void TrackManagerDialog::restoreData(QDataStream &stream)
{
    clear();

    SimpleMap<unsigned int, float> unusedEnvelope; // placeholder for alpha envelope; currently unused
    bool alphaEnvelopeIsActive;
    stream >> unusedEnvelope >> alphaEnvelopeIsActive;

    for (int i = 0; i < numGlobalEnvelopes; ++i)
    {
        bool envelopeIsActive;
        stream >> m_globalEnvelopes[i]
               >> envelopeIsActive; // currently unused
    }

    stream >> m_numTotalTracks;

    m_currentTracks = SimpleVector<Track*>(m_numTotalTracks);

    for (int i = 0; i < m_numTotalTracks; ++i)
    {
        QString name;
        int outputPort, trackType, numMIDICCEnvelopes;
        bool trackIsActive; // currently unused

        stream >> name >> outputPort >> trackType >> trackIsActive >> numMIDICCEnvelopes;

        m_allTracks[i].outputPort = m_portManager->createOutput(outputPort);
        setUpTrackSubMenu(m_allTracks[i], name, trackType);

        m_allTracks[i].envelopeDataVector = SimpleVector<EnvelopeData>(numMIDICCEnvelopes);
        for (int j = 0 ; j < numMIDICCEnvelopes; ++j)
        {
            bool envelopeIsActive;
            stream >> m_allTracks[i].envelopeDataVector[j].MIDIChannel
                   >> m_allTracks[i].envelopeDataVector[j].MIDICCNumber
                   >> m_allTracks[i].envelopeDataVector[j].envelope
                   >> envelopeIsActive; // currently unused
        }
        m_currentTracks.append(&m_allTracks[i]);

        QListWidgetItem *item = new QListWidgetItem(m_allTracks[i].menu->title());
        item->setFlags (item->flags() | Qt::ItemIsEditable);
        m_trackListWidget->addItem(item);
    }

    m_sequencerScene->restoreNotes(stream);

    setCurrentTrack(0);
    return;
}

void TrackManagerDialog::restoreTrack(int trackID, int trackIndex)
{
    m_currentTracks.insertSafely(trackIndex, &m_allTracks[trackID]);
    m_allTracks[trackID].outputPort->openLastPort();
    QAction *actionToBeAfterThisTrackMenu = (trackIndex - 1 >= m_currentTracks.size()) ? 0 : m_currentTracks[trackIndex + 1]->menu->menuAction();
    m_menu->insertMenu(actionToBeAfterThisTrackMenu, m_allTracks[trackID].menu);
    m_sequencerScene->insertTrack(trackIndex, m_allTracks[trackID].notes); // insert the notes into the sequencer
    QListWidgetItem *item = new QListWidgetItem(m_allTracks[trackID].menu->title());
    item->setFlags (item->flags() | Qt::ItemIsEditable);
    m_trackListWidget->insertItem(trackIndex, item);
    setCurrentTrack(trackIndex);
}

void TrackManagerDialog::saveData(QDataStream &stream)
{
    stream << int(0) << true; // num nodes in alpha envelope, and whether active (unused)

    for (int i = 0; i < numGlobalEnvelopes; ++i)
    {
        stream << m_globalEnvelopes[i]
               << true; // envelope is active (currently unused)
    }

    stream << m_currentTracks.size();

    for (int i = 0; i < m_currentTracks.size(); ++i)
    {
        stream << m_currentTracks[i]->menu->title()
               << m_currentTracks[i]->outputPort->currentPortNumber()
               << m_currentTracks[i]->trackTypeActionGroup->checkedAction()->data().toInt()
               << true // track is active (currently unused)
               << m_currentTracks[i]->envelopeDataVector.size();

        for (int j = 0; j < m_currentTracks[i]->envelopeDataVector.size(); ++j)
        {
            stream << m_currentTracks[i]->envelopeDataVector[j].MIDIChannel
                   << m_currentTracks[i]->envelopeDataVector[j].MIDICCNumber
                   << m_currentTracks[i]->envelopeDataVector[j].envelope
                   << true; // envelope is active (currently unused)
        }
    }

    m_sequencerScene->saveNotes(stream);
}

void TrackManagerDialog::setCurrentTrack(int track)
{
    if (track == -1)
        return;

    m_trackNameComboBox->setCurrentIndex(track); // also updates m_trackListWidget
    m_portManager->setCurrentTrack(track);
    m_sequencerScene->setCurrentTrack(track);

    // only change the envelope if viewing a MIDI CC envelope
    if (m_envelopeScene->currentTrack() >= 0) // if viewing a MIDI CC envelope
    {
        if (m_currentTracks[track]->envelopeDataVector.size() > 0)
            m_envelopeScene->setMIDICCEnvelope(&m_currentTracks[track]->envelopeDataVector[0].envelope, track, 0);
        else
            m_envelopeScene->setMIDICCEnvelope(&m_globalEnvelopes[0], -1, 0);
    }

    refreshMIDICCEnvelopes();
}

void TrackManagerDialog::setUpTrackSubMenu(Track &track, const QString &title, int trackType)
{
    track.menu = new QMenu(title);
    QMenu *trackTypeMenu = track.menu->addMenu(tr("Track Type"));
    track.trackTypeActionGroup = new QActionGroup(trackTypeMenu);
    QAction *actionDTType = new QAction(tr("Dynamic Tonality (Send Tuning Messages)"), track.trackTypeActionGroup);
    QAction *actionNonDTType = new QAction(tr("Non-DT"), track.trackTypeActionGroup);
    trackTypeMenu->addAction(actionDTType);
    trackTypeMenu->addAction(actionNonDTType);
    actionDTType->setCheckable(true);
    actionNonDTType->setCheckable(true);
    actionDTType->setData(0);
    actionNonDTType->setData(1);
    switch (trackType)
    {
    case 0: actionDTType->setChecked(true); break;
    case 1: actionNonDTType->setChecked(true); break;
    }
    connect(track.trackTypeActionGroup, &QActionGroup::triggered, [=](QAction *action)
    {
        int type = action->data().toInt();
        switch (type)
        {
        case 0: track.outputPort->sendsDTMessages = true;
        case 1: track.outputPort->sendsDTMessages = false;
        }
    });
    track.outputPort->menu()->setTitle(tr("MIDI Output Port"));
    track.menu->addMenu(track.outputPort->menu());
    m_menu->addMenu(track.menu);
}
