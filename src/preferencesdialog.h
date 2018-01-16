#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H
#include <QtWidgets/QDialog>

class MainWindow;
class QComboBox;
class QPushButton;
class QSettings;
class QSlider;

class PreferencesDialog : public QDialog
{
public:
    PreferencesDialog(MainWindow *mainWindow);
    void onButtonStyleChanged();
    void onColorPickerClicked(int id);
    void onInactiveTrackOpacityChanged(int opacity);
    void onMetronomeDelayBoxChanged(int index);
    void readSettings(QSettings *settings);
    void restoreDefaultColors();
    void writeSettings(QSettings *settings);

private:
    void setColor(int id, const QColor &color);

    void setLatticeBGColor(const QColor &color);
    void setSequencerBGColor(const QColor &color);
    void setPressedButtonColor(const QColor &color);
    void setDarkButtonColor(const QColor &color);
    void setLightButtonColor(const QColor &color);
    void setDarkNoteLaneColor(const QColor &color);
    void setLightNoteLaneColor(const QColor &color);
    void setPressedNoteLaneColor(const QColor &color);
    void setMinVelocityNoteColor(const QColor &color);
    void setMaxVelocityNoteColor(const QColor &color);
    void setUnselectedNoteColor(const QColor &color);
    void setSelectedNoteColor(const QColor &color);
    void setUnselectedNodeColor(const QColor &color);
    void setSelectedNodeColor(const QColor &color);
    void setEnvelopeColor(const QColor &color);
    void setGridLinesColor(const QColor &color);
    void setBeatLinesColor(const QColor &color);
    void setBarLinesColor(const QColor &color);
    void setCursorColor(const QColor &color);

    static const int numColorButtons = 19;

    MainWindow *mainWindow;
    QComboBox *buttonShapeBox;
    QSlider *buttonSizeSlider;
    QSlider *inactiveTrackOpacitySlider;
    QComboBox *metronomeLatencyBox;
    QPushButton *colorPickers[numColorButtons];
    QColor colors[numColorButtons];
};

#endif
