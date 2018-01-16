#include "mainwindow.h"
#include "mainwindowstrings.h"
#include "barlinecalculator.h"
#include "barlinedrawer.h"
#include "envelopeview.h"
#include "latticedata.h"
#include "latticemanager.h"
#include "latticescene.h"
#include "midieventhandler.h"
#include "midieventplayer.h"
#include "midifilebuilder.h"
#include "midiportmanager.h"
#include "preferencesdialog.h"
#include "projectsettingsdialog.h"
#include "qdatastreamoperators.h"
#include "sequencerscene.h"
#include "sequencersplitterhandle.h"
#include "trackmanagerdialog.h"
#include "zoomhandler.h"
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QThread>
#include <QtGui/QCloseEvent>
#include <QtMultimedia/QSoundEffect>
#include <QtWidgets/QApplication>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QUndoStack>

#ifdef Q_OS_WIN32
#include <QtWidgets/QStyleFactory>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      undoStack(new QUndoStack), // deleted in destuctor
      beatSound(new QSoundEffect(this)),
      measureSound(new QSoundEffect(this))
{
    setFocusPolicy(Qt::ClickFocus); // makes it so that you can click on the window to clear focus
    setFocus();

    betaSlider = new QSlider;
    betaSlider->setToolTip(tr("Generator"));
    betaSlider->setFixedWidth(40);
#ifdef Q_OS_WIN32
    betaSlider->setPageStep(0); // don't allow clicking above/below the slider because it won't emit the "slider pressed" signal (only relevant on Windows)
#endif
    // range is set later when the default project is loaded

    betaSpinBox = new QDoubleSpinBox;
    betaSpinBox->setToolTip(tr("Generator value (cents)"));
    betaSpinBox->setFixedWidth(45);
    betaSpinBox->setKeyboardTracking(false);
    betaSpinBox->setAlignment(Qt::AlignCenter);
    betaSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    betaSpinBox->setRange(0, 12288);

    latticeView = new QGraphicsView;
    latticeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    latticeView->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::DontSavePainterState);
    latticeView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    latticeView->setRenderHint(QPainter::Antialiasing, true);
    latticeView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    latticeView->setFocusPolicy(Qt::NoFocus);

    sequencerView = new QGraphicsView;
    sequencerView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    sequencerView->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::DontSavePainterState);
    sequencerView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    sequencerView->setDragMode(QGraphicsView::RubberBandDrag);
    sequencerView->setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);

    envelopeView = new EnvelopeView;
    envelopeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    envelopeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    envelopeView->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::DontSavePainterState);
    envelopeView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    envelopeView->setDragMode(QGraphicsView::RubberBandDrag);
    envelopeView->setResizeAnchor(QGraphicsView::AnchorViewCenter); // prevents randomly moving left when resizing the window vertically

    sequencerSplitterHandle = new SequencerSplitterHandle(sequencerView);
    QLayout *splitterHandleLayout = new QVBoxLayout;
    splitterHandleLayout->setMargin(0);
    splitterHandleLayout->addWidget(sequencerSplitterHandle);
    seqEnvSplitter = new QSplitter(Qt::Vertical);
    seqEnvSplitter->addWidget(sequencerView);
    seqEnvSplitter->addWidget(envelopeView);
    seqEnvSplitter->handle(1)->setLayout(splitterHandleLayout); // handle width is set below

    zoomHandler = new ZoomHandler(latticeView, sequencerView, envelopeView, sequencerSplitterHandle, this);

    barLineDrawer = new BarLineDrawer; // not a QObject; deleted in the destructor
    latticeData = new LatticeData; // not a QObject; deleted in the destructor

    latticeScene = new LatticeScene(latticeData, latticeView);
    sequencerScene = new SequencerScene(latticeData, barLineDrawer, undoStack, sequencerView);
    envelopeScene = new EnvelopeScene(barLineDrawer, undoStack, envelopeView);

    // random stuff
    midiPortManager = new MIDIPortManager; // not a QObject; deleted in destructor
    latticeManager = new LatticeManager(latticeScene, sequencerScene, latticeData, midiPortManager); // deleted in destructor
    midiEventHandler = new MIDIEventHandler(latticeManager, sequencerScene, midiPortManager); // not a QObject; deleted in destructor
    MIDIInput *midiInput = midiPortManager->createInput(midiEventHandler);
    midiInput->menu()->setTitle(tr("MIDI Port"));
    latticeScene->setMIDIEventHandler(midiEventHandler);

    // playback thread and MIDI player
    playbackThread = new QThread; // deleted in destructor
    midiEventPlayer = new MIDIEventPlayer(midiPortManager);
    midiEventPlayer->moveToThread(playbackThread);
    // =======================================================================

    // =============================================================== DIALOGS
    preferencesDialog = new PreferencesDialog(this);
    projectSettingsDialog = new ProjectSettingsDialog(latticeManager, undoStack, this);
    trackManagerDialog = new TrackManagerDialog(midiPortManager, sequencerScene, envelopeScene, this);
    envelopeScene->setTrackManagerDialog(trackManagerDialog);

    QDialog *helpDialog = new QDialog(this, Qt::Tool);
    helpDialog->setWindowTitle(tr("Help"));
    helpDialog->setMinimumWidth(100);
    QLabel *helpDialogLabel = new QLabel;
    helpDialogLabel->setWordWrap(true);
    helpDialogLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    helpDialogLabel->setOpenExternalLinks(true);
    helpDialogLabel->setText(helpLabelText);
    QVBoxLayout *helpDialogLayout = new QVBoxLayout(helpDialog);
    helpDialogLayout->addWidget(helpDialogLabel);
    // =======================================================================

    // ================================= SET UP TRANSPORT TOOLBAR AND MENU BAR
    QToolBar *transportToolBar = new QToolBar;
    transportToolBar->setIconSize(QSize(25, 25));

#ifdef Q_OS_MAC // right-aligns the toolbar on Mac
    transportToolBar->setContentsMargins(0, 0, 6, 0);
    QWidget *leftSpacerWidget = new QWidget;
    leftSpacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    transportToolBar->addWidget(leftSpacerWidget); // right-align the transport tool bar
#endif

    transportToolBar->addWidget(trackManagerDialog->trackNameComboBox());
    transportToolBar->addSeparator();
    actionRecord = transportToolBar->addAction(tr("Record"));
    actionRecord->setCheckable(true);
    actionRecord->setIcon(QIcon(":/resources/record.svg"));
    actionPlay = transportToolBar->addAction(tr("Play"));
    actionPlay->setCheckable(true);
    actionPlay->setIcon(QIcon(":/resources/play.svg"));
    actionPlay->setShortcut(Qt::Key_Space);
    QAction *actionStop = transportToolBar->addAction(tr("Stop"));
    actionStop->setIcon(QIcon(":/resources/stop.svg"));
    transportToolBar->addSeparator();
    QAction *actionShowHideEnvelope = transportToolBar->addAction(tr("Show/Hide Envelope"));
    actionShowHideEnvelope->setIcon(QIcon(":resources/envelope.svg"));
    actionShowHideEnvelope->setCheckable(true);
    actionToggleMetronomeEnabled = transportToolBar->addAction(tr("Metronome"));
    actionToggleMetronomeEnabled->setCheckable(true);
    actionToggleMetronomeEnabled->setIcon(QIcon(":/resources/metronome.svg"));
    actionSnapToGrid = transportToolBar->addAction(tr("Snap to Grid"));
    actionSnapToGrid->setCheckable(true);
    actionSnapToGrid->setIcon(QIcon(":/resources/snap.svg"));
    gridGroup = new QActionGroup(this);
    const QString gridGroupNames[11] = {"1/2", "1/2T", "1/4", "1/4T", "1/8", "1/8T", "1/16", "1/16T", "1/32", "1/32T", "1/64"};
    QMenu *gridMenu = new QMenu(tr("Grid"), this); // parent should be "this" so that the menu doesn't inherit the toolbutton's style
    for (int i = 0; i < 11; ++i)
    {
        QAction *gridAction = new QAction(gridGroupNames[i], gridGroup);
        gridAction->setCheckable(true);
        gridAction->setData(i);
        gridMenu->addAction(gridAction);
    }
    static_cast<QToolButton*>(transportToolBar->widgetForAction(actionSnapToGrid))->setMenu(gridMenu);
#ifdef Q_OS_WIN32
    static_cast<QToolButton*>(transportToolBar->widgetForAction(actionSnapToGrid))->setPopupMode(QToolButton::MenuButtonPopup);
#endif
#ifdef Q_OS_MAC
    static_cast<QToolButton*>(transportToolBar->widgetForAction(actionSnapToGrid))->setPopupMode(QToolButton::DelayedPopup);
#endif
    // =======================================================================

    // ============================================= CREATE AND SET UP LAYOUTS
    QGridLayout *betaLayout = new QGridLayout;
    betaLayout->addWidget(betaSlider, 0, 1, 1, 1);
    betaLayout->addWidget(betaSpinBox, 1, 0, 1, 3);
    latSeqSplitter = new QSplitter(Qt::Horizontal);
    latSeqSplitter->addWidget(latticeView);
    latSeqSplitter->addWidget(seqEnvSplitter);
    QWidget *mainWidget = new QWidget;
    mainWidget->setContentsMargins(6, 0, 6, 6);
    QHBoxLayout *mainHLayout = new QHBoxLayout(mainWidget);
    mainHLayout->setMargin(0);
    mainHLayout->setSpacing(6);
    mainHLayout->addLayout(betaLayout);
    mainHLayout->addWidget(latSeqSplitter);

#ifdef Q_OS_WIN32 // make the central widget the main widget, and prepare the custom menu bar
    setCentralWidget(mainWidget);

    QWidget *menuBarWidget = new QWidget;
    QMenuBar *mainMenuBar = new QMenuBar;
    QHBoxLayout *menuBarLayout = new QHBoxLayout(menuBarWidget);
    menuBarLayout->addWidget(mainMenuBar, 0, Qt::AlignVCenter);
    menuBarLayout->addWidget(transportToolBar);
    menuBarLayout->setContentsMargins(6, 0, 2, 0);
    setMenuWidget(menuBarWidget);
#endif
#ifdef Q_OS_MAC // make the central widget a new widget that contains the toolbar and the main widget
    setCentralWidget(new QWidget);
    QVBoxLayout *mainVLayout = new QVBoxLayout(centralWidget());
    mainVLayout->setMargin(0);
    mainVLayout->setSpacing(6);
    mainVLayout->addWidget(transportToolBar);
    mainVLayout->addWidget(mainWidget);

    QMenuBar *mainMenuBar = menuBar();
#endif
    // =======================================================================

    // ======================================================== SET UP ACTIONS
    // file menu
    QAction *actionNew = new QAction(tr("New Project"), this);
    actionNew->setShortcut(QKeySequence::New);
    QAction *actionOpen = new QAction(tr("Open Project"), this);
    actionOpen->setShortcut(QKeySequence::Open);
    actionSave = new QAction(tr("Save Project"), this);
    actionSave->setShortcut(QKeySequence::Save);
    QAction *actionSaveAs = new QAction(tr("Save Project As"), this);
    actionSaveAs->setShortcut(QKeySequence::SaveAs);
    QAction *actionExportMIDI = new QAction(tr("Export MIDI File"), this);
    QAction *actionOpenProjectSettingsDialog = new QAction(tr("Project Settings..."), this);
    actionOpenProjectSettingsDialog->setShortcut(QKeySequence("Ctrl+p"));

    // edit menu
    QAction *actionUndo = undoStack->createUndoAction(this);
    actionUndo->setShortcut(QKeySequence::Undo);
    QAction *actionRedo = undoStack->createRedoAction(this);
    actionRedo->setShortcut(QKeySequence::Redo);
    QAction *actionCut = new QAction(tr("Cut"), this);
    actionCut->setShortcut(QKeySequence::Cut);
    QAction *actionCopy = new QAction(tr("Copy"), this);
    actionCopy->setShortcut(QKeySequence::Copy);
    QAction *actionPaste = new QAction(tr("Paste"), this);
    actionPaste->setShortcut(QKeySequence::Paste);
    QAction *actionDelete = new QAction(tr("Delete"), this);
    actionDelete->setShortcuts(QList<QKeySequence>()
        << QKeySequence::Delete
        << QKeySequence(Qt::Key_Backspace));
    QAction *actionSelectAll = new QAction(tr("Select All"), this);
    actionSelectAll->setShortcut(QKeySequence::SelectAll);
    QAction *actionPreferences = new QAction(tr("Preferences..."), this);

    // view menu
    QActionGroup *transformGroup = new QActionGroup(this);
    QAction *actionShear = new QAction(tr("Shear"), transformGroup);
    actionShear->setCheckable(true);
    actionShear->setData(0);
    QAction *actionRotate = new QAction(tr("Rotate"), transformGroup);
    actionRotate->setCheckable(true);
    actionRotate->setData(1);
    QAction *actionShowPeriodShading = new QAction(tr("Period Shading"), this);
    actionShowPeriodShading->setCheckable(true);
    QAction *actionVZoomIn = new QAction(tr("Vert. Zoom In"), this);
    actionVZoomIn->setShortcuts(QList<QKeySequence>()
        << QKeySequence(tr("Ctrl++"))
        << QKeySequence(tr("Ctrl+=")));
    QAction *actionVZoomOut = new QAction(tr("Vert. Zoom Out"), this);
    actionVZoomOut->setShortcut(QKeySequence(tr("Ctrl+-")));
    // =======================================================================
    // somehow using Option (i.e., Alt) seems to mess up on Mac, so I have to change the shortcuts  a bit
    // also note that on Mac, multiple shortcuts don't seem to work, so I have to put the "=" shortcuts before "+"
    QAction *actionHZoomInLattice = new QAction(tr("Hor. Zoom In (Lattice)"), this);
    QAction *actionHZoomOutLattice = new QAction(tr("Hor. Zoom Out (Lattice)"), this);
    QAction *actionHZoomInSequencer = new QAction(tr("Hor. Zoom In (Sequencer)"), this);
    QAction *actionHZoomOutSequencer = new QAction(tr("Hor. Zoom Out (Sequencer)"), this);
#ifdef Q_OS_WIN32
    actionHZoomInLattice->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Ctrl+Alt++")) << QKeySequence(tr("Ctrl+Alt+=")));
    actionHZoomOutLattice->setShortcut(QKeySequence(tr("Ctrl+Alt+-")));
    actionHZoomInSequencer->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Alt++")) << QKeySequence(tr("Alt+=")));
    actionHZoomOutSequencer->setShortcut(QKeySequence(tr("Alt+-")));
#endif
#ifdef Q_OS_MAC
    actionHZoomInLattice->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Ctrl+Alt+=")) << QKeySequence(tr("Ctrl+Alt++")));
    actionHZoomOutLattice->setShortcut(QKeySequence(tr("Ctrl+Alt+-")));
    actionHZoomInSequencer->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Ctrl+Meta+=")) << QKeySequence(tr("Ctrl+Meta++")));
    actionHZoomOutSequencer->setShortcut(QKeySequence(tr("Ctrl+Meta+-")));
#endif
    // =======================================================================

    // midi menu
    QActionGroup *midiInputTypeActionGroup = new QActionGroup(this);
    QAction *actionStandardMIDIInput = new QAction(tr("Standard"), midiInputTypeActionGroup);
    actionStandardMIDIInput->setCheckable(true);
    actionStandardMIDIInput->setData(0);
    QAction *actionAxisMIDIInput = new QAction(tr("AXiS-49 (Selfless Mode)"), midiInputTypeActionGroup);
    actionAxisMIDIInput->setCheckable(true);
    actionAxisMIDIInput->setData(1);
    QAction *actionRelayerMIDIInput = new QAction(tr("Relayer"), midiInputTypeActionGroup);
    actionRelayerMIDIInput->setCheckable(true);
    actionRelayerMIDIInput->setData(2);

    // help menu
    QAction *actionOpenHelpDialog = new QAction(tr("Help"), this);
    // =======================================================================

    // ========================================================== SET UP MENUS
    QMenu *fileMenu = mainMenuBar->addMenu(tr("File"));
        fileMenu->addAction(actionNew);
        fileMenu->addAction(actionOpen);
        fileMenu->addAction(actionSave);
        fileMenu->addAction(actionSaveAs);
        fileMenu->addAction(actionExportMIDI);
        fileMenu->addSeparator();
        fileMenu->addAction(actionOpenProjectSettingsDialog);
    QMenu *editMenu = mainMenuBar->addMenu(tr("Edit"));
        editMenu->addAction(actionUndo);
        editMenu->addAction(actionRedo);
        editMenu->addSeparator();
        editMenu->addAction(actionCut);
        editMenu->addAction(actionCopy);
        editMenu->addAction(actionPaste);
        editMenu->addAction(actionDelete);
        editMenu->addSeparator();
        editMenu->addAction(actionSelectAll);
        editMenu->addSeparator();
        editMenu->addSeparator();
        editMenu->addAction(actionPreferences);
    QMenu *viewMenu = mainMenuBar->addMenu(tr("View"));
        viewMenu->addAction(actionShear);
        viewMenu->addAction(actionRotate);
        viewMenu->addSeparator();
        viewMenu->addAction(actionShowPeriodShading);
        viewMenu->addSeparator();
        viewMenu->addAction(actionVZoomIn);
        viewMenu->addAction(actionVZoomOut);
        viewMenu->addAction(actionHZoomInLattice);
        viewMenu->addAction(actionHZoomOutLattice);
        viewMenu->addAction(actionHZoomInSequencer);
        viewMenu->addAction(actionHZoomOutSequencer);
    mainMenuBar->addMenu(trackManagerDialog->menu());
    QMenu *inputMenu = mainMenuBar->addMenu(tr("Input"));
        inputMenu->addMenu(midiInput->menu());
        QMenu *midiInputMenu = new QMenu(tr("MIDI Input Type"));
            midiInputMenu->addAction(actionStandardMIDIInput);
            midiInputMenu->addAction(actionAxisMIDIInput);
            midiInputMenu->addAction(actionRelayerMIDIInput);
        inputMenu->addMenu(midiInputMenu);

    QMenu *helpMenu = mainMenuBar->addMenu(tr("Help"));
        helpMenu->addAction(actionOpenHelpDialog);
    // =======================================================================

    // ============================================================ SET STYLES
    betaSpinBox->setStyleSheet("QAbstractSpinBox {background:transparent; border:none;}");

    const QString sequencerLabelStyleSheet("QFrame {margin-left:6px; margin-top:6px; padding:2px; border:1px solid gray; background:rgba(255, 255, 255, 165);}");
    sequencerScene->label()->setStyleSheet(sequencerLabelStyleSheet);
    envelopeScene->label()->setStyleSheet(sequencerLabelStyleSheet);

#ifdef Q_OS_WIN32
    static const QString scrollBarBGColor("rgb(240,240,240)");
    static const QString scrollBarHandleColor("rgb(180,180,180)");
    centralWidget()->setStyleSheet(QString(
                "QAbstractScrollArea {border: 1px solid rgb(150, 150, 150);}"
                "QAbstractScrollArea::corner {background: transparent}"
                "QScrollBar::sub-line, QScrollBar::add-line {width: 0px; height: 0px;}" // remove buttons
                "QScrollBar::horizontal {height: 16px; padding: 0 5px 0 5px; background: %1;}"
                "QScrollBar::vertical {width: 16px; padding: 5px 0 5px 0; background: %1;}"
                "QScrollBar::sub-page, QScrollBar::add-page {background: %1;}"
                "QScrollBar::handle:horizontal {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 %1, stop: 0.28 %1, stop: 0.29 %2, stop: 0.71 %2, stop: 0.72 %1, stop: 1 %1);}"
                "QScrollBar::handle:vertical {background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 %1, stop: 0.28 %1, stop: 0.29 %2, stop: 0.71 %2, stop: 0.72 %1, stop: 1 %1);}"
                ).arg(scrollBarBGColor, scrollBarHandleColor));

    menuBarWidget->setStyleSheet(
                "QMenuBar, QMenuBar::item {background-color: transparent;}"
                "QMenuBar::item:selected {color: rgb(90, 90, 90);}"
                "QToolBar::separator {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 transparent, stop: 0.5 gray, stop: 1 transparent); margin: 5px; width: 1px;}"
                );

    transportToolBar->setStyle(QStyleFactory::create("fusion"));
#endif // Q_OS_WIN32

#ifdef Q_OS_MAC
    QFont font;
    font.setPixelSize(10);
    betaSpinBox->setFont(font);
#endif // Q_OS_MAC
    // =======================================================================

    latticeView->setScene(latticeScene);
    sequencerView->setScene(sequencerScene);
    envelopeView->setScene(envelopeScene);

    // =============================================== CONNECT SIGNALS & SLOTS
    // file menu actions
    connect(actionNew, &QAction::triggered, this, &MainWindow::onActionNew);
    connect(actionSave, &QAction::triggered, this, &MainWindow::onActionSave);
    connect(actionSaveAs, &QAction::triggered, this, &MainWindow::onActionSaveAs);
    connect(actionOpen, &QAction::triggered, this, &MainWindow::onActionOpen);
    connect(actionExportMIDI, &QAction::triggered, this, &MainWindow::exportMIDIFile);
    connect(actionOpenProjectSettingsDialog, &QAction::triggered, projectSettingsDialog, &QDialog::show);

    // edit menu actions
    connect(actionCut, &QAction::triggered, [=]{sequencerWithFocus()->cut();});
    connect(actionCopy, &QAction::triggered, [=]{sequencerWithFocus()->copy();});
    connect(actionPaste, &QAction::triggered, [=]{sequencerWithFocus()->paste();});
    connect(actionDelete, &QAction::triggered, [=]{sequencerWithFocus()->deleteSelectedItems();});
    connect(actionSelectAll, &QAction::triggered, [=]{sequencerWithFocus()->selectAll();});
    connect(actionPreferences, &QAction::triggered, preferencesDialog, &QDialog::show);

    // view menu actions
    connect(transformGroup, &QActionGroup::triggered, [=](QAction *action){latticeManager->toTransformMode(action->data().toInt());});
    connect(actionShowPeriodShading, &QAction::toggled, [=](bool show){
        latticeScene->showPeriodShading = show;
        latticeScene->update();
    });
    connect(actionVZoomIn, &QAction::triggered, zoomHandler, &ZoomHandler::zoomInLatticeVertically);
    connect(actionVZoomOut, &QAction::triggered, zoomHandler, &ZoomHandler::zoomOutLatticeVertically);
    connect(actionHZoomInLattice, &QAction::triggered, zoomHandler, &ZoomHandler::zoomInLatticeHorizontally);
    connect(actionHZoomOutLattice, &QAction::triggered, zoomHandler, &ZoomHandler::zoomOutLatticeHorizontally);
    connect(actionHZoomInSequencer, &QAction::triggered, zoomHandler, &ZoomHandler::zoomInSequencersHorizontally);
    connect(actionHZoomOutSequencer, &QAction::triggered, zoomHandler, &ZoomHandler::zoomOutSequencersHorizontally);

    // midi menu actions
    connect(midiInputTypeActionGroup, &QActionGroup::triggered, [=](QAction *action){midiEventHandler->setMIDIInputType(static_cast<MIDIEventHandler::MIDIInputType>(action->data().toInt()));});

    // grid menu actions
    connect(gridGroup, &QActionGroup::triggered, [=](QAction *){updateProjectTiming();}); // signal is overloaded

    // help menu action
    connect(actionOpenHelpDialog, &QAction::triggered, helpDialog, &QDialog::show);

    // widget signals and slots
    // note, this can't be valueChanged because changing the spinbox to a value outside of the slider's range will cause it to emit the valueChanged signal, which will set the spinbox to the wrong value
    connect(betaSlider, &QSlider::sliderMoved, [=](int value){betaSpinBox->setValue(value * .01);});
    connect(betaSlider, &QAbstractSlider::sliderPressed, midiEventPlayer, &MIDIEventPlayer::temporarilyDisableBetaOutput, Qt::DirectConnection);
    connect(betaSlider, &QAbstractSlider::sliderReleased, latticeManager, &LatticeManager::updateSceneBoundingRects);
    connect(betaSpinBox, &QAbstractSpinBox::editingFinished, [=](){betaSpinBox->clearFocus();});
    connect(betaSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=] (double value) {
        latticeManager->setBeta(value);
        betaSlider->setValue(static_cast<int>(value * 100 + .5));
    });
#ifdef Q_OS_MAC
    connect(betaSlider, &QSlider::sliderPressed, [=](){betaSpinBox->setValue(betaSlider->value() * .01);}); // in case clicked on the slider track, causing the slider to jump (does not happen on Windows)
#endif

    // scrollbar synchronization
    connect(latticeView->verticalScrollBar(), &QAbstractSlider::valueChanged, sequencerView->verticalScrollBar(), &QAbstractSlider::setValue);
    connect(sequencerView->verticalScrollBar(), &QAbstractSlider::valueChanged, latticeView->verticalScrollBar(), &QAbstractSlider::setValue);
    connect(envelopeView->horizontalScrollBar(), &QAbstractSlider::valueChanged, sequencerView->horizontalScrollBar(), &QAbstractSlider::setValue);
    connect(sequencerView->verticalScrollBar(), &QAbstractSlider::rangeChanged, [=](int, int){alignLatticeWithSequencer();});
    connect(sequencerView->horizontalScrollBar(), &QAbstractSlider::valueChanged, [=] (int value) {
        envelopeView->horizontalScrollBar()->setValue(value);
        sequencerSplitterHandle->update();
    });

    // transport controls
    connect(actionRecord, &QAction::toggled, this, &MainWindow::onRecordButtonClicked);
    connect(actionPlay, &QAction::toggled, this, &MainWindow::onPlayButtonClicked);
    connect(actionStop, &QAction::triggered, this, &MainWindow::onStopButtonClicked);

    // threads, midi event player, cursor
    connect(playbackThread, &QThread::started, midiEventPlayer, &MIDIEventPlayer::start);
    connect(midiEventPlayer, &MIDIEventPlayer::finished, playbackThread, &QThread::quit);
    connect(midiEventPlayer, &MIDIEventPlayer::betaChanged, this, &MainWindow::onBetaChangedWhilePlaying); // TODO: ideally this would be a lambda, but it seems lambdas can only be direct connection (note: this only was a problem when there was a note in the sequencer, AND a beta envelope)

    connect(midiEventPlayer, &MIDIEventPlayer::beatChanged, this, &MainWindow::playBeatSound);
    connect(midiEventPlayer, &MIDIEventPlayer::measureChanged, this, &MainWindow::playMeasureSound);
    connect(midiEventPlayer, &MIDIEventPlayer::tickPositionChanged, this, &MainWindow::onTickPositionChangedWhilePlaying);
    connect(sequencerScene, &AbstractSequencerScene::cursorMoved, midiEventPlayer, &MIDIEventPlayer::setTickPosition, Qt::DirectConnection);
    connect(envelopeScene, &AbstractSequencerScene::cursorMoved, midiEventPlayer, &MIDIEventPlayer::setTickPosition, Qt::DirectConnection);
    connect(sequencerScene, &AbstractSequencerScene::cursorMoved, envelopeScene, &AbstractSequencerScene::setCursorPos);
    connect(envelopeScene, &AbstractSequencerScene::cursorMoved, sequencerScene, &AbstractSequencerScene::setCursorPos);

    // misc
    connect(undoStack, &QUndoStack::indexChanged, [=](int){onUndoStackIndexChanged();});
    connect(projectSettingsDialog, &ProjectSettingsDialog::betaChanged, betaSpinBox, &QDoubleSpinBox::setValue);
    connect(projectSettingsDialog, &ProjectSettingsDialog::betaSliderRangeChanged, [=](double min, double max){betaSlider->setRange(min * 100, max * 100);});
    connect(projectSettingsDialog, &ProjectSettingsDialog::projectTimingUpdated, this, &MainWindow::updateProjectTiming);
    connect(sequencerSplitterHandle, &SequencerSplitterHandle::loopChanged, midiEventPlayer, &MIDIEventPlayer::setLoopBounds, Qt::DirectConnection);
    connect(sequencerSplitterHandle, &SequencerSplitterHandle::loopDisabled, midiEventPlayer, &MIDIEventPlayer::disableLoop, Qt::DirectConnection);
    connect(actionShowHideEnvelope, &QAction::toggled, [=] (bool show) {
        if (show) {
            seqEnvSplitter->restoreState(seqEnvSplitterState);
            sequencerView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            sequencerView->setCornerWidget(0);
        } else if (envelopeView->height() != 0) {
            seqEnvSplitterState = seqEnvSplitter->saveState();
            seqEnvSplitter->setSizes(QList<int>() << 1 << 0);
            sequencerView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        }
    });
    connect(seqEnvSplitter, &QSplitter::splitterMoved, [=](int, int) {
        sequencerView->setHorizontalScrollBarPolicy(envelopeView->height() == 0 ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
    });
    connect(actionSnapToGrid, &QAction::toggled, [=](bool toggled) {
        sequencerScene->setSnapToGrid(toggled);
        envelopeScene->setSnapToGrid(toggled);
    });
    // =======================================================================

    latticeManager->setMIDIOutputEnabled(false);

    // ========================================================= READ SETTINGS
    QSettings settings(QCoreApplication::applicationDirPath() + "/support/settings.ini", QSettings::IniFormat);

    if (settings.value("maximized", false).toBool())
        setWindowState(Qt::WindowMaximized);

    latSeqSplitter->restoreState(settings.value("latsplittersizes").toByteArray());
    seqEnvSplitter->restoreState(settings.value("seqsplittersizes").toByteArray()); // NOTE: Also restores the width of the handle, on Mac
    seqEnvSplitter->setHandleWidth(18);
    zoomHandler->restoreSettings(&settings);

    // restore midi input settings
    midiInputTypeActionGroup->actions().at(settings.value("midiintype", 0).toInt())->trigger();
    midiInput->openPort(settings.value("midiinport", -1).toInt()); // default is to not open a port

    // restore metronome state
    if (settings.value("metronomeenabled", false).toBool() && !actionToggleMetronomeEnabled->isChecked())
        actionToggleMetronomeEnabled->trigger();

    preferencesDialog->readSettings(&settings);
    actionShowHideEnvelope->trigger();

    transformGroup->actions().at(settings.value("transformmode", 0).toInt())->trigger();
    latticeScene->showPeriodShading = settings.value("periodshading", true).toBool();
    actionShowPeriodShading->setChecked(latticeScene->showPeriodShading);
    // =======================================================================

#ifdef Q_OS_WIN32
    QDir::setCurrent(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
#endif
#ifdef Q_OS_MAC
    QDir::setCurrent(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
#endif

    onActionNew();

    latticeManager->setMIDIOutputEnabled(true);
}

void MainWindow::onInitialShow()
{
    latticeView->centerOn(0, 0);
}

MainWindow::~MainWindow()
{
    delete latticeManager; // must be deleted before sequencerScene because the destructor deletes buttons that may be in the scene
    delete undoStack;
    delete midiEventHandler;
    delete midiPortManager;
    delete latticeData;
    delete barLineDrawer;

    midiEventPlayer->stop();
    playbackThread->wait();
    delete midiEventPlayer;
    delete playbackThread;
}

// ###########################################################################
// ################################################################### METHODS

void MainWindow::alignLatticeWithSequencer()
{
    // this function ensures that the lattice and sequencer line up, at least when they both have scroll bars
    bool sequencerHasVerticalScrollBar = (sequencerView->verticalScrollBar()->minimum() != sequencerView->verticalScrollBar()->maximum());
    bool latticeHasVerticalScrollBar = (latticeView->verticalScrollBar()->minimum() != latticeView->verticalScrollBar()->maximum());
    if (latticeHasVerticalScrollBar && sequencerHasVerticalScrollBar)
        latticeView->verticalScrollBar()->setRange(sequencerView->verticalScrollBar()->minimum(), sequencerView->verticalScrollBar()->maximum());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    switch (showUnsavedFileWarning())
    {
        case QMessageBox::Cancel:
           event->setAccepted(false);
           return;
        case QMessageBox::Save:
            onActionSaveAs();
        // default is to descard
    }

    writeSettings();
}

void MainWindow::exportMIDIFile()
{
    QString exportPath(QFileDialog::getSaveFileName(0, tr("Export MIDI File"), QDir::currentPath(), tr("MIDI File (*.mid)")));

    if (exportPath.isEmpty())
        return;

    if (!exportPath.endsWith(".mid", Qt::CaseInsensitive))
        exportPath.append(".mid");

    MIDIFileBuilder builder;
    builder.setTimeSignature(projectSettingsDialog->timeSigNum(), projectSettingsDialog->timeSigDen());
    builder.setTempo(projectSettingsDialog->tempoBPM());
    builder.setResolution(BarLineCalculator::ticksPerQuarterNote);
    builder.setEvents(trackManagerDialog->gatherSequencerEvents(1. / projectSettingsDialog->tempoTicksPerMS()));
    builder.writeToFile(exportPath);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat() || event->modifiers() != Qt::NoModifier)
        return;

    midiEventHandler->onKeyEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->modifiers() != Qt::NoModifier)
        return;

    midiEventHandler->onKeyEvent(event);
}

void MainWindow::onActionNew()
{
    switch (showUnsavedFileWarning())
    {
    case QMessageBox::Cancel:
        return;
    case QMessageBox::Save:
        onActionSaveAs();
    // default discards (continues function without saving)
    }

    // "DummyFolder" ensures that QFile::exists(currentFilePath) returns false
    // when you save the project
    currentFilePath = QString(tr("DummyFolder/New Project.hxp"));

    open(QCoreApplication::applicationDirPath() + "/support/New Project.hxp");
}

void MainWindow::onActionOpen()
{
    switch (showUnsavedFileWarning())
    {
    case QMessageBox::Cancel: return;
    case QMessageBox::Save: onActionSaveAs();
    // default is discard, which continues without saving
    }

    QString openPath(QFileDialog::getOpenFileName(this, tr("Open Hex Project"), QDir::currentPath(), tr("Hex Project File (*.hxp)")));

    // do not proceed if the user clicks cancel
    if (openPath.isNull())
        return;

    // do not proceed if user tries to open non-existing file (could happen when Hex is initially loaded if it can't find the default project)
    if (!QFile::exists(openPath))
    {
        QMessageBox::warning(this,
                             tr("File Not Found"),
                             tr("File not found or unavailable: ") + openPath);
        return;
    }

    currentFilePath = openPath;
    open(currentFilePath);
}

void MainWindow::onActionSave()
{
    if (!QFile::exists(currentFilePath)) // if currently using the default blank project
        currentFilePath = QFileDialog::getSaveFileName(this, tr("Save Hex Project"), QDir::currentPath(), tr("Hex Project File (*.hxp)"));

    save();
}

void MainWindow::onActionSaveAs()
{
    currentFilePath = QFileDialog::getSaveFileName(this, tr("Save Hex Project As"), QDir::currentPath(), tr("Hex Project File (*.hxp)"));

    save();
}

void MainWindow::onBetaChangedWhilePlaying(double beta)
{
    latticeManager->setMIDIOutputEnabled(false);
    betaSpinBox->setValue(beta);
    latticeManager->setMIDIOutputEnabled(true);
}

void MainWindow::onPlayButtonClicked(bool on)
{
    if (!on)
    {
        midiEventPlayer->stop();
        actionRecord->setChecked(false);
    }
    else
    {
        midiEventPlayer->setTempo(projectSettingsDialog->tempoTicksPerMS());
        midiEventPlayer->setEvents(trackManagerDialog->gatherSequencerEvents(1. / projectSettingsDialog->tempoTicksPerMS()));
        latticeManager->sendMIDIData();
        playbackThread->start(QThread::HighPriority);
    }
}

void MainWindow::onRecordButtonClicked(bool recording)
{
    if (recording)
        midiEventHandler->startRecording(sequencerScene);
    else
        midiEventHandler->stopRecording();

    actionPlay->setChecked(recording);
}

void MainWindow::onStopButtonClicked()
{
    if (actionPlay->isChecked()) // stop
    {
        midiEventPlayer->stop();
        actionPlay->setChecked(false);
        actionRecord->setChecked(false);
    }
    else rewind();
}

void MainWindow::onTickPositionChangedWhilePlaying(double pos)
{
    // auto-scroll right if needed
    if ((pos * sequencerView->transform().m11()) - sequencerView->horizontalScrollBar()->value() > sequencerView->viewport()->width())
        sequencerView->horizontalScrollBar()->setValue(sequencerView->horizontalScrollBar()->value() + sequencerView->viewport()->width());

    sequencerScene->setCursorPos(pos);
    envelopeScene->setCursorPos(pos);
}

void MainWindow::onUndoStackIndexChanged()
{
    setWindowModified(!undoStack->isClean());
    actionSave->setEnabled(!undoStack->isClean());

    if (actionPlay->isChecked())
    {
        midiEventPlayer->setTempo(projectSettingsDialog->tempoTicksPerMS());
        midiEventPlayer->setEvents(trackManagerDialog->gatherSequencerEvents(1. / projectSettingsDialog->tempoTicksPerMS()));
    }
}

void MainWindow::open(const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,
                             tr("File Not Found"),
                             tr("File not found or unavailable: ") + filePath);
        return;
    }

    undoStack->clear(); // this must be up here, before the scene is cleared

    QDataStream in(&file);
    in.setVersion(13);

    int fileFormat;
    in >> fileFormat;
    if (fileFormat > 1)
    {
        QMessageBox::warning(this,
                             tr("Version Problem"),
                             tr("This file was created with a newer version of Hex and can't be opened."
                                "Please go to the Dynamic Tonality website and download the latest version of Hex."));
        return;
    }

    // ========================================== RESTORE LATTICE SETUP DIALOG
    LatticeSettings savedLatticeSettings;
    in >> savedLatticeSettings;
    projectSettingsDialog->setLatticeSettings(savedLatticeSettings);
    // =======================================================================

    // ======================================= RESTORE PROJECT TIMING SETTINGS
    int gridInterval;
    bool snapToGrid;
    TimingSettings savedProjectTimingSettings;
    in >> gridInterval >> snapToGrid >> savedProjectTimingSettings;
    gridGroup->actions().at(gridInterval)->setChecked(true);
    actionSnapToGrid->setChecked(snapToGrid);
    projectSettingsDialog->setTimingSettings(savedProjectTimingSettings);
    // =======================================================================

    // ================================================ RESTORE ALPHA AND BETA
    double alpha, beta; // alpha won't get used in current versions of Hex
    in >> alpha >> beta;
    latticeManager->setAlpha(alpha);
    betaSpinBox->setValue(beta);
    // =======================================================================

    trackManagerDialog->restoreData(in);

    rewind();
    updateWindowTitle();
}

void MainWindow::playBeatSound()
{
    if (actionToggleMetronomeEnabled->isChecked())
        beatSound->play();
}

void MainWindow::playMeasureSound()
{
    if (actionToggleMetronomeEnabled->isChecked())
        measureSound->play();
}

void MainWindow::rewind()
{
    sequencerScene->setCursorPos(0);
    sequencerView->horizontalScrollBar()->setValue(0);
    envelopeScene->setCursorPos(0);
    envelopeView->horizontalScrollBar()->setValue(0);
    midiEventPlayer->setTickPosition(0);
}

void MainWindow::save()
{
    if (currentFilePath.isEmpty())
        return;

    if (!currentFilePath.endsWith(".hxp", Qt::CaseInsensitive))
        currentFilePath.append(".hxp");

    QFile file(currentFilePath);
    if (!file.open(QIODevice::WriteOnly)) // opens file for writing
        return;

    QDataStream out(&file);
    out.setVersion(13);

    out << int(1); // file format
    out << projectSettingsDialog->getCurrentLatticeSettings();
    out << gridGroup->checkedAction()->data().toInt();
    out << actionSnapToGrid->isChecked();
    out << projectSettingsDialog->getCurrentTimingSettings();
    out << latticeManager->getDT()->period() << latticeManager->getDT()->generator();

    trackManagerDialog->saveData(out);

    undoStack->setClean();
    setWindowModified(false);
    updateWindowTitle();

    QFileInfo currentFileInfo(currentFilePath);
    QDir::setCurrent(currentFileInfo.path());
}

AbstractSequencerScene *MainWindow::sequencerWithFocus() const
{
    if (envelopeView->hasFocus())
        return envelopeScene;

    return sequencerScene;
}

//void MainWindow::setEnvelopeSelectionArea(int left, int right)
//{
//    QPainterPath path;
//    double oneOverHorZoom = 1. / envelopeView->transform().m11();
//    double sceneLeft = left * oneOverHorZoom;
//    double sceneRight = right * oneOverHorZoom;
//    path.addRect(sceneLeft, -1, sceneRight - sceneLeft, 2);
//    envelopeScene->setSelectionArea(path);
//}

//void MainWindow::setSequencerSelectionArea(int left, int right)
//{
//    QPainterPath path;
//    double oneOverHorZoom = 1. / sequencerView->transform().m11();
//    double sceneLeft = left * oneOverHorZoom;
//    double sceneRight = right * oneOverHorZoom;
//    path.addRect(sceneLeft,
//                 sequencerScene->sceneRect().top(),
//                 sceneRight - sceneLeft,
//                 sequencerScene->sceneRect().height());
//    sequencerScene->setSelectionArea(path);
//}

void MainWindow::setMetronomeVersion(int version)
{
    beatSound->setSource(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/support/beat" + QString::number(version) + ".wav"));
    measureSound->setSource(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/support/measure" + QString::number(version) + ".wav"));
}

int MainWindow::showUnsavedFileWarning()
{
    if (!isWindowModified())
        return QMessageBox::Discard;

    return QMessageBox::warning(this, "Hex", tr("Do you want to save your changes?"),
                                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                QMessageBox::Save);
}

void MainWindow::updateProjectTiming()
{
    BarLineCalculator calculator;
    calculator.run(projectSettingsDialog->numMeasures(),
                   projectSettingsDialog->timeSigNum(),
                   projectSettingsDialog->timeSigDen(),
                   static_cast<BarLineCalculator::GridInterval>(gridGroup->checkedAction()->data().toInt()));

    // update snap sizes
    sequencerScene->setSnapSize(calculator.getGridLength());
    envelopeScene->setSnapSize(calculator.getGridLength());

    // update bar line spacings
    barLineDrawer->setLineSpacings(calculator.getMeasureLength(), calculator.getBeatLength(), calculator.getGridLength());
    sequencerSplitterHandle->setMeasureLengthTicks(calculator.getMeasureLength());
    midiEventPlayer->setBeatAndMeasureLength(calculator.getBeatLength(), calculator.getMeasureLength());

    sequencerScene->setLength(calculator.getSequencerLength());
    envelopeScene->setLength(calculator.getSequencerLength());
}

void MainWindow::updateWindowTitle()
{
    QString projectTitle(currentFilePath.section('/', -1)); // remove path
    projectTitle.chop(4); // remove .hxp extension
    setWindowTitle(QString("Hex - %1[*]").arg(projectTitle));
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/support/settings.ini", QSettings::IniFormat);

    settings.setValue("maximized", isMaximized());
    settings.setValue("latsplittersizes", latSeqSplitter->saveState());
    settings.setValue("seqsplittersizes", seqEnvSplitter->saveState());
    zoomHandler->captureSettings(&settings);

    // save midi settings
    settings.setValue("midiintype", static_cast<int>(midiEventHandler->getMIDIInputType()));
    settings.setValue("midiinport", midiPortManager->input(0)->currentPortNumber());

    preferencesDialog->writeSettings(&settings);

    // save lattice properties
    settings.setValue("transformmode", latticeManager->getTransformMode());
    settings.setValue("periodshading", latticeScene->showPeriodShading);

    // save metronome state
    settings.setValue("metronomeenabled", actionToggleMetronomeEnabled->isChecked());
}
