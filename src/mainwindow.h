#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtWidgets/QMainWindow>

class AbstractSequencerScene;
class BarLineDrawer;
class EnvelopeScene;
class EnvelopeView;
class LatticeManager;
class LatticeScene;
class MIDIEventPlayer;
class MIDIEventHandler;
class MIDIPortManager;
class PreferencesDialog;
class ProjectSettingsDialog;
class QAction;
class QActionGroup;
class QDoubleSpinBox;
class QGraphicsView;
class QSlider;
class QSoundEffect;
class QSplitter;
class QThread;
class QUndoStack;
class SequencerScene;
class SequencerSplitterHandle;
class TrackManagerDialog;
class ZoomHandler;
struct LatticeData;

class MainWindow : public QMainWindow
{
    friend class PreferencesDialog;

public:
    MainWindow(QWidget *parent = 0);
    void onInitialShow();
    ~MainWindow();
    void alignLatticeWithSequencer();
    void exportMIDIFile();
    void onActionNew();
    void onActionOpen();
    void onActionSave();
    void onActionSaveAs();
    void onBetaChangedWhilePlaying(double beta);
    void onPlayButtonClicked(bool on);
    void onRecordButtonClicked(bool recording);
    void onStopButtonClicked();
    void onTickPositionChangedWhilePlaying(double pos);
    void onUndoStackIndexChanged();
    void playBeatSound();
    void playMeasureSound();
    void setMetronomeVersion(int version);
    void updateProjectTiming();

    // inline methods
    QSize sizeHint() const {return QSize(800, 600);}

    // currently unused methods
    //    void setEnvelopeSelectionArea(int left, int right);
    //    void setSequencerSelectionArea(int left, int right);

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    void open(const QString &filePath);
    int showUnsavedFileWarning();
    void rewind();
    void save();
    AbstractSequencerScene *sequencerWithFocus() const;
    void updateWindowTitle();
    void writeSettings();

    // these are initialized in the initializer list
    QUndoStack *undoStack;
    QSoundEffect *beatSound;
    QSoundEffect *measureSound;

    // widgets and stuff
    QSlider *betaSlider;
    QDoubleSpinBox *betaSpinBox;
    QGraphicsView *latticeView;
    QGraphicsView *sequencerView;
    EnvelopeView *envelopeView;
    BarLineDrawer *barLineDrawer;
    SequencerSplitterHandle *sequencerSplitterHandle;

    // scenes
    LatticeScene *latticeScene;
    SequencerScene *sequencerScene;
    EnvelopeScene *envelopeScene;

    // dialogs
    TrackManagerDialog *trackManagerDialog;
    PreferencesDialog *preferencesDialog;
    ProjectSettingsDialog *projectSettingsDialog;

    // midi stuff
    MIDIPortManager *midiPortManager;
    MIDIEventHandler *midiEventHandler;
    MIDIEventPlayer *midiEventPlayer;

    // actions
    QAction *actionPlay;
    QAction *actionRecord;
    QAction *actionSave;
    QAction *actionSnapToGrid;
    QAction *actionToggleMetronomeEnabled;
    QActionGroup *gridGroup;

    // misc
    LatticeData *latticeData;
    QString currentFilePath;
    LatticeManager *latticeManager;
    QSplitter *latSeqSplitter;
    QSplitter *seqEnvSplitter;
    QThread *playbackThread;
    ZoomHandler *zoomHandler;
    QByteArray seqEnvSplitterState;
};

#endif
