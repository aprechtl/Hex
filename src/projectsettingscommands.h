#ifndef MAINWINDOWCOMMANDS_H
#define MAINWINDOWCOMMANDS_H
#include <QtWidgets/QUndoCommand>
#include "hexsettings.h"
#include "projectsettingsdialog.h"

class ChangeLatticeSettingsCommand : public QUndoCommand
{
public:
    ChangeLatticeSettingsCommand(ProjectSettingsDialog *dialog,
                                 const LatticeSettings &oldSettings,
                                 const LatticeSettings &newSettings)
        : projectSettingsDialog(dialog), undoSettings(oldSettings), redoSettings(newSettings)
    {
        setText(QObject::tr("Change Lattice Settings"));
    }

    void redo() {projectSettingsDialog->setLatticeSettings(redoSettings);}
    void undo() {projectSettingsDialog->setLatticeSettings(undoSettings);}
    int id() const {return HexSettings::latticeSettingsCommandID;}

    bool mergeWith(const QUndoCommand *other)
    {
        if (other->id() != id())
            return false;

        redoSettings = static_cast<const ChangeLatticeSettingsCommand *>(other)->redoSettings;
        return true;
    }

private:
    ProjectSettingsDialog *projectSettingsDialog;
    LatticeSettings undoSettings;
    LatticeSettings redoSettings;
};

// ###########################################################################
// ###########################################################################

class ChangeProjectTimingCommand : public QUndoCommand
{
public:
    ChangeProjectTimingCommand(ProjectSettingsDialog *dialog,
                               const TimingSettings &oldSettings,
                               const TimingSettings &newSettings)
        : projectSettingsDialog(dialog), undoSettings(oldSettings), redoSettings(newSettings)
    {
        setText(QObject::tr("Change Project Timing"));
    }

    void redo() {projectSettingsDialog->setTimingSettings(redoSettings);}
    void undo() {projectSettingsDialog->setTimingSettings(undoSettings);}
    int id() const {return HexSettings::projectTimingSettingsCommandID;}

    bool mergeWith(const QUndoCommand *other)
    {
        if (other->id() != id())
            return false;

        redoSettings = static_cast<const ChangeProjectTimingCommand *>(other)->redoSettings;
        return true;
    }

private:
    ProjectSettingsDialog *projectSettingsDialog;
    TimingSettings undoSettings;
    TimingSettings redoSettings;
};

#endif
