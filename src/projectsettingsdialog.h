#ifndef PROJECTSETTINGSDIALOG_H
#define PROJECTSETTINGSDIALOG_H
#include <QtWidgets/QDialog>

struct LatticeSettings
{
    unsigned char numLargeSteps, numSmallSteps, numPeriods, apsLayout;
    bool flipped;
    double alpha, betaSliderMin, betaSliderMax;
};

struct TimingSettings
{
    int timeSigNumerator, timeSigDenominator, numMeasures;
    double tempo;
};

class LatticeManager;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QUndoStack;

class ProjectSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    ProjectSettingsDialog(LatticeManager *manager, QUndoStack *stack, QWidget *parent = 0);
    LatticeSettings getCurrentLatticeSettings();
    TimingSettings getCurrentTimingSettings();
    int numMeasures() const;
    void setLatticePreset(int preset);
    void setLatticeSettings(const LatticeSettings &settings);
    void setTimingSettings(const TimingSettings &settings);
    double tempoBPM() const;
    double tempoTicksPerMS() const;
    int timeSigDen() const;
    int timeSigNum() const;

signals:
    void betaChanged(double); // for when a preset is selected
    void betaSliderRangeChanged(double, double);
    void projectTimingUpdated();

private slots:
    void createNewChangeLatticeCommand();
    void createNewChangeProjectTimingCommand();

private:
    void updateStepBoxMaxima();
    void updateTempoTicksPerMS();

    // these are initialized in the initializer list
    LatticeManager *latticeManager;
    QUndoStack *undoStack;

    QComboBox *APSComboBox;
    QDoubleSpinBox *betaSliderMaxBox;
    QDoubleSpinBox *betaSliderMinBox;
    QSpinBox *largeStepBox;
    QCheckBox *flipLatticeCheckBox;
    LatticeSettings latticeSettings;
    QSpinBox *numMeasuresBox;
    QSpinBox *numPeriodsBox;
    QDoubleSpinBox *periodBox;
    QSpinBox *smallStepBox;
    bool suppressChangeLatticeCommands;
    bool suppressProjectTimingCommands;
    QDoubleSpinBox *tempoBox;
    QComboBox *timeSigDenomBox;
    QComboBox *timeSigNumerBox;
    TimingSettings timingSettings;
};

#endif
