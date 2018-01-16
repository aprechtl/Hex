#include "preferencesdialog.h"
#include "barlinedrawer.h"
#include "envelopeview.h"
#include "latticemanager.h"
#include "latticescene.h"
#include "mainwindow.h"
#include "sequencerscene.h"
#include <QtCore/QSettings>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton>

const QString colorNames[] = {
    "latticebgcolor",
    "sequencerbgcolor",
    "pressedbuttoncolor",
    "darkbuttoncolor",
    "lightbuttoncolor",
    "darknotelanecolor",
    "lightnotelanecolor",
    "pressednotelanecolor",
    "minvelocitynotecolor",
    "maxvelocitynotecolor",
    "unselectednoteoutlinecolor",
    "selectednoteoutlinecolor",
    "unselectednodecolor",
    "selectednodecolor",
    "envelopecolor",
    "gridlinecolor",
    "beatlinecolor",
    "barlinecolor",
    "cursorcolor"
};

const QString colorLabels[] = {
    "Lattice background: ",
    "Sequencer background: ",
    "Pressed button: ",
    "Dark button (unpressed): ",
    "Light button (unpressed): ",
    "Dark note lane: ",
    "Light note lane: ",
    "Pressed note lane: ",
    "Minimum velocity note: ",
    "Maximum velocity note: ",
    "Unselected note outline: ",
    "Selected note outline: ",
    "Unselected node: ",
    "Selected node: ",
    "Envelope line: ",
    "Grid lines: ",
    "Beat lines: ",
    "Bar lines: ",
    "Cursor: "
};

const QColor defaultColors[] = {
    QColor(126, 126, 126),      // latticeBGColor
    QColor(245, 245, 245),      // sequencerBGColor
    QColor(170, 170, 127, 120), // pressedButtonColor
    QColor(192, 200, 204),      // darkButtonColor
    QColor(234, 234, 234),      // lightButtonColor
    QColor(160, 150, 136, 220), // darkNoteLaneColor
    QColor(185, 185, 185, 240), // lightNoteLaneColor
    QColor(255, 255, 255, 40),  // pressedNoteLaneColor
    QColor(255, 255, 255),      // minVelocNoteColor
    QColor(177, 17, 17),        // maxVelocNoteColor
    QColor(0, 0, 0),            // unselectedNoteColor
    QColor(255, 255, 255),      // selectedNoteColor
    QColor(0, 0, 0),            // unselectedNodeColor
    QColor(150, 150, 150),      // selectedNodeColor
    QColor(25, 25, 25, 200),    // envelopeColor
    QColor(175, 175, 175),      // gridLineColor
    QColor(130, 130, 130),      // beatLineColor
    QColor(50, 50, 50),         // barLineColor
    QColor(100, 100, 140)       // cursorColor
};

const QString buttonStyleSheet = "background-color: %1; border: 1px; border-color: black; border-style: outset;";
const QString graphicsViewStyleSheet = "QGraphicsView {background: qradialgradient(cx:0.5, cy:0.5, fx:0.5, fy:0.5, radius:0.6, stop:0 %1, stop:1 %2);}";

// ###########################################################################
// ###########################################################################

PreferencesDialog::PreferencesDialog(MainWindow *mainWindow)
    : QDialog(mainWindow, Qt::Tool), mainWindow(mainWindow)
{
    setWindowTitle(tr("Preferences"));
    setMinimumWidth(260);

    QTabWidget *tabWidget = new QTabWidget;
    setLayout(new QHBoxLayout);
    layout()->addWidget(tabWidget);

    // =========================================================== GENERAL TAB
    buttonShapeBox = new QComboBox;
    buttonShapeBox->addItem(tr("Hexagon"));
    buttonShapeBox->addItem(tr("Ellipse"));
    buttonShapeBox->addItem(tr("Rectangle"));

    buttonSizeSlider = new QSlider(Qt::Horizontal);
    buttonSizeSlider->setRange(250, 996);
    buttonSizeSlider->setMinimumHeight(30);

    inactiveTrackOpacitySlider = new QSlider(Qt::Horizontal);
    inactiveTrackOpacitySlider->setRange(0, 255);
    inactiveTrackOpacitySlider->setMinimumHeight(30);

    metronomeLatencyBox = new QComboBox;
    metronomeLatencyBox->addItem(tr("None"));
    metronomeLatencyBox->addItem(tr("50 ms"));
    metronomeLatencyBox->addItem(tr("100 ms"));
    metronomeLatencyBox->addItem(tr("150 ms"));
    metronomeLatencyBox->addItem(tr("200 ms"));

    // layout stuff
    QWidget *generalTab = new QWidget;
    QFormLayout *generalLayout = new QFormLayout(generalTab);
    generalLayout->addRow(tr("Button shape:"), buttonShapeBox);
    generalLayout->addRow(tr("Button size:"), buttonSizeSlider);
    generalLayout->addRow(tr("Inactive track opacity:"), inactiveTrackOpacitySlider);
    generalLayout->addRow(tr("Metronome click latency:"), metronomeLatencyBox);
    tabWidget->addTab(generalTab, tr("General"));

    connect(buttonShapeBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int){onButtonStyleChanged();});
    connect(buttonSizeSlider, &QSlider::valueChanged, [=](int){onButtonStyleChanged();});
    connect(inactiveTrackOpacitySlider, &QSlider::valueChanged, this, &PreferencesDialog::onInactiveTrackOpacityChanged);
    connect(metronomeLatencyBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index){onMetronomeDelayBoxChanged(index);});
    // =======================================================================

    // ============================================================= COLOR TAB
    // color buttons
    QWidget *colorTab = new QWidget;
    QFormLayout *colorLayout = new QFormLayout(colorTab);

    for (int i = 0; i < numColorButtons; ++i)
    {
        colorPickers[i] = new QPushButton;
        connect(colorPickers[i], &QPushButton::clicked, [=](){onColorPickerClicked(i);});
        colorLayout->addRow(colorLabels[i], colorPickers[i]);
    }

    QPushButton *restoreColorsButton = new QPushButton(tr("Restore Default Colors"));
    connect(restoreColorsButton, &QPushButton::clicked, this, &PreferencesDialog::restoreDefaultColors);

    QWidget *spacer = new QWidget;
    spacer->setMinimumHeight(6);
    colorLayout->addRow(spacer);
    colorLayout->addRow(restoreColorsButton);
    tabWidget->addTab(colorTab, tr("Colors"));
    // =======================================================================
}

void PreferencesDialog::onButtonStyleChanged()
{
    mainWindow->latticeManager->setButtonScaleAndType(buttonSizeSlider->value() * .001, buttonShapeBox->currentIndex());
}

void PreferencesDialog::onColorPickerClicked(int id)
{
    QColor color((defaultColors[id].alpha() < 255)
                 ? QColorDialog::getColor(colors[id], 0, tr("Select Color"), QColorDialog::ShowAlphaChannel)
                 : QColorDialog::getColor(colors[id], 0, tr("Select Color")));

    if (!color.isValid())
        return;

    setColor(id, color);
}

void PreferencesDialog::onInactiveTrackOpacityChanged(int opacity)
{
    mainWindow->sequencerScene->setInactiveNoteBrushOpacity(opacity);
}

void PreferencesDialog::onMetronomeDelayBoxChanged(int index)
{
    mainWindow->setMetronomeVersion(index);
}

void PreferencesDialog::readSettings(QSettings *settings)
{
    // restore button and lattice properties
    buttonShapeBox->blockSignals(true);
    buttonSizeSlider->blockSignals(true);
    buttonShapeBox->setCurrentIndex(settings->value("buttonshape", 0).toInt());
    buttonSizeSlider->setValue(settings->value("buttonsize", 988).toInt());
    buttonShapeBox->blockSignals(false);
    buttonSizeSlider->blockSignals(false);
    onButtonStyleChanged();

    inactiveTrackOpacitySlider->setValue(settings->value("inactivetrackopacity", 80).toInt());

#ifdef Q_OS_MAC
    metronomeLatencyBox->setCurrentIndex(settings->value("metronomedelay", 0).toInt());
#endif
#ifdef Q_OS_WIN32
    metronomeLatencyBox->setCurrentIndex(settings->value("metronomedelay", 4).toInt());
#endif

    // restore colors
    for (int i = 0; i < numColorButtons; ++i)
    {
        setColor(i, settings->value(colorNames[i], defaultColors[i]).value<QColor>());
    }
}

void PreferencesDialog::restoreDefaultColors()
{
    for (int i = 0; i < numColorButtons; ++i)
    {
        setColor(i, defaultColors[i]);
    }

    mainWindow->latticeScene->update();
    mainWindow->sequencerScene->update();
    mainWindow->envelopeScene->update();
}

void PreferencesDialog::setColor(int id, const QColor &color)
{
    colors[id] = color;
    colorPickers[id]->setStyleSheet(buttonStyleSheet.arg(color.name()));

    switch (id)
    {
    case 0:
        setLatticeBGColor(color);
        break;
    case 1:
        setSequencerBGColor(color);
        break;
    case 2:
        setPressedButtonColor(color);
        break;
    case 3:
        setDarkButtonColor(color);
        break;
    case 4:
        setLightButtonColor(color);
        break;
    case 5:
        setDarkNoteLaneColor(color);
        break;
    case 6:
        setLightNoteLaneColor(color);
        break;
    case 7:
        setPressedNoteLaneColor(color);
        break;
    case 8:
        setMinVelocityNoteColor(color);
        break;
    case 9:
        setMaxVelocityNoteColor(color);
        break;
    case 10:
        setUnselectedNoteColor(color);
        break;
    case 11:
        setSelectedNoteColor(color);
        break;
    case 12:
        setUnselectedNodeColor(color);
        break;
    case 13:
        setSelectedNodeColor(color);
        break;
    case 14:
        setEnvelopeColor(color);
        break;
    case 15:
        setGridLinesColor(color);
        break;
    case 16:
        setBeatLinesColor(color);
        break;
    case 17:
        setBarLinesColor(color);
        break;
    case 18:
        setCursorColor(color);
        break;
    }
}

void PreferencesDialog::writeSettings(QSettings *settings)
{
    // save lattice properties
    settings->setValue("buttonshape", buttonShapeBox->currentIndex());
    settings->setValue("buttonsize", buttonSizeSlider->value());
    settings->setValue("inactivetrackopacity", inactiveTrackOpacitySlider->value());
    settings->setValue("metronomedelay", metronomeLatencyBox->currentIndex());

    // save colors
    for (int i = 0; i < numColorButtons; ++i)
    {
        settings->setValue(colorNames[i], colors[i]);
    }
}

// ###########################################################################
// #################################################################### COLORS

void PreferencesDialog::setLatticeBGColor(const QColor &color)
{
    mainWindow->latticeView->setStyleSheet(graphicsViewStyleSheet.arg(color.lighter(120).name()).arg(color.name()));
}

void PreferencesDialog::setSequencerBGColor(const QColor &color)
{
//    mainWindow->sequencerView->setStyleSheet(graphicsViewStyleSheet.arg(color.lighter(120).name()).arg(color.name()));
//    mainWindow->envelopeView->setStyleSheet(graphicsViewStyleSheet.arg(color.lighter(120).name()).arg(color.name()));
    mainWindow->sequencerView->setStyleSheet(QString("QGraphicsView {background: %1;}").arg(color.name()));
    mainWindow->envelopeView->setStyleSheet(QString("QGraphicsView {background: %1;}").arg(color.name()));

}

void PreferencesDialog::setDarkButtonColor(const QColor &color)
{
    mainWindow->latticeScene->setDarkButtonColor(color);
    mainWindow->latticeScene->update();
}

void PreferencesDialog::setLightButtonColor(const QColor &color)
{
    mainWindow->latticeScene->setLightButtonColor(color);
    mainWindow->latticeScene->update();
}

void PreferencesDialog::setPressedButtonColor(const QColor &color)
{
    mainWindow->latticeScene->setPressedButtonColor(color);
}

void PreferencesDialog::setDarkNoteLaneColor(const QColor &color)
{
    mainWindow->latticeScene->setDarkLaneColor(color);
    mainWindow->latticeScene->update();

    QColor sequencerColor(color);
    sequencerColor.setAlpha(255);
    mainWindow->sequencerScene->setDarkLaneColor(sequencerColor);
    mainWindow->sequencerScene->update();
}

void PreferencesDialog::setLightNoteLaneColor(const QColor &color)
{
    mainWindow->latticeScene->setLightLaneColor(color);
    mainWindow->latticeScene->update();

    QColor sequencerColor(color);
    sequencerColor.setAlpha(255);
    mainWindow->sequencerScene->setLightLaneColor(sequencerColor);
    mainWindow->sequencerScene->update();
}

void PreferencesDialog::setPressedNoteLaneColor(const QColor &color)
{ mainWindow->sequencerScene->setPressedLaneColor(color); }

void PreferencesDialog::setMinVelocityNoteColor(const QColor &color)
{ mainWindow->sequencerScene->setMinVelocityColor(color); }

void PreferencesDialog::setMaxVelocityNoteColor(const QColor &color)
{ mainWindow->sequencerScene->setMaxVelocityColor(color); }

void PreferencesDialog::setUnselectedNoteColor(const QColor &color)
{ mainWindow->sequencerScene->setUnselectedNoteColor(color); }

void PreferencesDialog::setSelectedNoteColor(const QColor &color)
{ mainWindow->sequencerScene->setSelectedNoteColor(color); }

void PreferencesDialog::setUnselectedNodeColor(const QColor &color)
{
    QPen pen(color);
    pen.setWidth(9);
    pen.setCosmetic(true);
    pen.setCapStyle(Qt::RoundCap);
    mainWindow->envelopeScene->setUnselectedNodePen(pen);
}

void PreferencesDialog::setSelectedNodeColor(const QColor &color)
{
    QPen pen(color);
    pen.setWidth(10);
    pen.setCosmetic(true);
    pen.setCapStyle(Qt::RoundCap);
    mainWindow->envelopeScene->setSelectedNodePen(pen);
}

void PreferencesDialog::setEnvelopeColor(const QColor &color)
{
    QPen pen(color);
    pen.setWidth(2);
    pen.setCosmetic(true);
    mainWindow->envelopeScene->setEnvelopePen(pen);
}

void PreferencesDialog::setGridLinesColor(const QColor &color)
{
    mainWindow->barLineDrawer->setGridLineColor(color);
    mainWindow->sequencerScene->update();
    mainWindow->envelopeScene->update();
}

void PreferencesDialog::setBeatLinesColor(const QColor &color)
{
    mainWindow->barLineDrawer->setBeatLineColor(color);
    mainWindow->sequencerScene->update();
    mainWindow->envelopeScene->update();
}

void PreferencesDialog::setBarLinesColor(const QColor &color)
{
    mainWindow->barLineDrawer->setBarLineColor(color);
    mainWindow->sequencerScene->update();
    mainWindow->envelopeScene->update();
}

void PreferencesDialog::setCursorColor(const QColor &color)
{
    QPen cursorPen(color);
    cursorPen.setCosmetic(true);
    cursorPen.setWidth(3);
    mainWindow->sequencerScene->setCursorPen(cursorPen);
    mainWindow->envelopeScene->setCursorPen(cursorPen);
}

