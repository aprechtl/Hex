TEMPLATE  = app
TARGET    = Hex
CONFIG   += qt c++11
QT       += multimedia widgets svg # core / gui included by default with "qt" in config
RESOURCES = resources.qrc
DEFINES  += HEX_VERSION_NAME=\\\"2.1\\\"
SOURCES  += main.cpp\
            mainwindow.cpp \
            note.cpp \
            rtm/RtMidi.cpp \
            dynamictonality.cpp \
            preferencesdialog.cpp \
            envelopescene.cpp \
            draghandlers.cpp \
            sequencerscene.cpp \
            qdatastreamoperators.cpp \
            latticemanager.cpp \
            abstractsequencerscene.cpp \
            barlinecalculator.cpp \
            latticescene.cpp \
            barlinedrawer.cpp \
            buttonshapecalculator.cpp \
            sequencercommands.cpp \
            midieventplayer.cpp \
            envelopegenerator.cpp \
            notesequencegenerator.cpp \
            nodecommands.cpp \
            envelopecommands.cpp \
            zoomhandler.cpp \
            midifilebuilder.cpp \
            midioutput.cpp \
            midiinput.cpp \
            midieventhandler.cpp \
            projectsettingsdialog.cpp \
            midiport.cpp \
            sequencersplitterhandle.cpp \
            sequencereventcombiner.cpp \
            midiportmanager.cpp \
            trackmanagerdialog.cpp \
            trackcommands.cpp \
            lineeditdelegate.cpp
HEADERS  += mainwindow.h \
            note.h \
            rtm/RtMidi.h \
            rtm/RtError.h \
            dynamictonality.h \
            sequencerscene.h \
            sequencercommands.h \
            preferencesdialog.h \
            envelopescene.h \
            midioutput.h \
            draghandlers.h \
            qdatastreamoperators.h \
            latticemanager.h \
            abstractsequencerscene.h \
            barlinecalculator.h \
            latticescene.h \
            barlinedrawer.h \
            buttonshapecalculator.h \
            midieventplayer.h \
            highreselapsedtimer.h \
            envelopegenerator.h \
            notesequencegenerator.h \
            nodecommands.h \
            envelopecommands.h \
            zoomhandler.h \
            midifilebuilder.h \
            simplevector.h \
            hexsettings.h \
            midiinput.h \
            simplemap.h \
            midieventhandler.h \
            projectsettingscommands.h \
            projectsettingsdialog.h \
    envelopeview.h \
    midiport.h \
    sortalgorithms.h \
    sequencersplitterhandle.h \
    mainwindowstrings.h \
    notestruct.h \
    sequencerevent.h \
    sequencereventcombiner.h \
    midiportmanager.h \
    trackmanagerdialog.h \
    track.h \
    envelopedata.h \
    trackcommands.h \
    lineeditdelegate.h \
    latticedata.h

win32 {
    DESTDIR = build/win

    DEFINES += __WINDOWS_MM__ # for RtMidi
    LIBS += $$PWD/lib/windows/WinMM.lib
    RC_FILE = windowsiconfile.rc
}

macx {
    DESTDIR = build/mac

    DEFINES += __MACOSX_CORE__
    LIBS += -framework CoreMidi \
            -framework CoreAudio \
            -framework CoreFoundation
}

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/qrc
