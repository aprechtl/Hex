#include <QtWidgets/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    w.onInitialShow();

    return a.exec();
}

// comment out the above code, and run this code to create the default Hex project:

//#include <QtCore>
//#include "projectsettingsdialog.h"
//#include "qdatastreamoperators.h"

//const LatticeSettings latticeSettings = {5, 2, 10, 12, false, 1200., 685.7143, 720.}; // 5 large steps, 2 small steps, 10 periods, 12 = APSLayout, not flipped
//const TimingSettings timingSettings = {4, 4, 16, 100.0}; // 4/4 time sig, 16 measures, 100 BPM

//int main(int argc, char *argv[])
//{
//    QCoreApplication a(argc, argv);

//    QFile file("New Project--Generated.hxp");

//    file.open(QIODevice::WriteOnly);

//    QDataStream out(&file);
//    out.setVersion(13);
//    out << int(1); // file format
//    out << latticeSettings;
//    out << int(6); // grid step
//    out << true; // snap to grid
//    out << timingSettings;
//    out << double(1200.) << double(700.); // alpha, beta
//    out << int(0) << true // num nodes in alpha envelope, and whether active
//        << int(0) << true // ditto for beta
//        << int(0) << true // ditto for gamma
//        << int(0) << true; // ditto for lambda
//    out << int(1); // number of tracks
//    out << QString("Track 1"); // track name
//    out << int(0); // MIDI port of track 1
//    out << int(0); // port type (DT)
//    out << true; // track 1 is active
//    out << int(0); // number of MIDI CC envelopes in track 1
//    out << int(0); // number of notes in track 1

//    file.close();

//    return 0;
//}
