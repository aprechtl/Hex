#ifndef MAINWINDOWSTRINGS_H
#define MAINWINDOWSTRINGS_H
#include <QtCore/QString>

const QString helpLabelText(
        "<center>"
        "<b>Hex</b>"
        "<br>Version "
        HEX_VERSION_NAME
        "</center>"
        "<br>"
        "Hex is a MIDI sequencer that allows you to create microtonal music using Dynamic Tonality. You can find out more about Dynamic Tonality "
        "at <a href=\"http://www.dynamictonality.com\">dynamictonality.com</a>.<br>"
        "<br>"
        "The slider on the far left of Hex's main window controls the size of the tuning's generator, which is displayed below in cents. Dragging "
        "the slider will either shear or rotate the lattice, depending on the current setting (available in the View menu). The size of the "
        "tuning's period can be adjusted in the Project Settings dialog (File > Project Settings), as can the number of small and large steps, "
        "and other related settings.<br>"
        "<br>"
        "If you have any questions, or you would like to provide feedback, please send an email to aprechtl@gmail.com.");
#endif // MAINWINDOWSTRINGS_H
