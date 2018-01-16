#ifndef HEXSETTINGS_H
#define HEXSETTINGS_H
#include <Qt>
#include <QtCore/QEvent>

namespace HexSettings
{
    // note: maxK must equal -minK because of the way the lattice is drawn
    // in LatticeManager (see the helper function). Andy's MIDI mapping only
    // allows for -25 <= j <= 26 and -18 <= k <= 17

    const short int maxJ = 26;
    const short int maxK = 17;
    const short int minJ = -25;
    const short int minK = -17;
    const short int numJ = maxJ - minJ + 1;
    const short int numK = maxK - minK + 1;
    const short int numButtons = numJ * numK;

    const int latticeSettingsCommandID = 100;
    const int projectTimingSettingsCommandID = 101;
    const QEvent::Type doneRecordingNoteEventType = static_cast<QEvent::Type>(1001);

    const unsigned int maxPolyphony = 16;

    // timing variables
    const int sleepIntervalMilliseconds = 6;
    const unsigned int sleepIntervalMicroseconds = 1000 * sleepIntervalMilliseconds;
    const int numSleepIntervalsBetweenGUIUpdates = 8;
    const double envelopeResolutionMS = 24;
    const double oneOverEnvResolution = 1.0 / envelopeResolutionMS;

    inline void convertNoteLaneIndexToJK(int index, short int &j, short int &k)
    {
        int quotient = index / HexSettings::numJ;
        k = (quotient % 2 == 0) ? quotient / -2 : (quotient + 1) / 2;
        j = index - quotient * HexSettings::numJ + HexSettings::minJ;
    }

    inline int convertJKToNoteLaneIndex(short int j, short int k)
    {
        return ((k > 0) ? k + k - 1 : -k - k) * HexSettings::numJ + (j - HexSettings::minJ);
    }

    inline bool convertQWERTYKeyToWickiJK(int key, short int &j, short int &k) // returns true if successful
    {
        switch (key)
        {
        case Qt::Key_1: j =  7; k =-12; break;
        case Qt::Key_Q: j =  6; k =-11; break;
        case Qt::Key_2: j =  6; k =-10; break;
        case Qt::Key_A: j =  5; k =-10; break;
        case Qt::Key_W: j =  5; k = -9; break;
        case Qt::Key_3: j =  5; k = -8; break;
        case Qt::Key_Z: j =  4; k = -9; break;
        case Qt::Key_S: j =  4; k = -8; break;
        case Qt::Key_E: j =  4; k = -7; break;
        case Qt::Key_4: j =  4; k = -6; break;
        case Qt::Key_X: j =  3; k = -7; break;
        case Qt::Key_D: j =  3; k = -6; break;
        case Qt::Key_R: j =  3; k = -5; break;
        case Qt::Key_5: j =  3; k = -4; break;
        case Qt::Key_C: j =  2; k = -5; break;
        case Qt::Key_F: j =  2; k = -4; break;
        case Qt::Key_T: j =  2; k = -3; break;
        case Qt::Key_6: j =  2; k = -2; break;
        case Qt::Key_V: j =  1; k = -3; break;
        case Qt::Key_G: j =  1; k = -2; break;
        case Qt::Key_Y: j =  1; k = -1; break;
        case Qt::Key_7: j =  1; k =  0; break;
        case Qt::Key_B: j =  0; k = -1; break;
        case Qt::Key_H: j =  0; k =  0; break;
        case Qt::Key_U: j =  0; k =  1; break;
        case Qt::Key_8: j =  0; k =  2; break;
        case Qt::Key_N: j = -1; k =  1; break;
        case Qt::Key_J: j = -1; k =  2; break;
        case Qt::Key_I: j = -1; k =  3; break;
        case Qt::Key_9: j = -1; k =  4; break;
        case Qt::Key_M: j = -2; k =  3; break;
        case Qt::Key_K: j = -2; k =  4; break;
        case Qt::Key_O: j = -2; k =  5; break;
        case Qt::Key_0: j = -2; k =  6; break;
        case Qt::Key_Comma:        j = -3; k =  5; break;
        case Qt::Key_L:            j = -3; k =  6; break;
        case Qt::Key_P:            j = -3; k =  7; break;
        case Qt::Key_Minus:        j = -3; k =  8; break;
        case Qt::Key_Period:       j = -4; k =  7; break;
        case Qt::Key_Semicolon:    j = -4; k =  8; break;
        case Qt::Key_BracketLeft:  j = -4; k =  9; break;
        case Qt::Key_Equal:        j = -4; k = 10; break;
        case Qt::Key_Slash:        j = -5; k =  9; break;
        case Qt::Key_Apostrophe:   j = -5; k = 10; break;
        case Qt::Key_BracketRight: j = -5; k = 11; break;
        case Qt::Key_Backslash:    j = -6; k = 13; break;

        // f keys
        case Qt::Key_F1:  j = 8; k = -13; break;
        case Qt::Key_F2:  j = 7; k = -11; break;
        case Qt::Key_F3:  j = 6; k = -9; break;
        case Qt::Key_F4:  j = 5; k = -7; break;
        case Qt::Key_F5:  j = 4; k = -5; break;
        case Qt::Key_F6:  j = 3; k = -3; break;
        case Qt::Key_F7:  j = 2; k = -1; break;
        case Qt::Key_F8:  j = 1; k = 1; break;
        case Qt::Key_F9:  j = 0; k = 3; break;
        case Qt::Key_F10: j = -1; k = 5; break;
        case Qt::Key_F11: j = -2; k = 7; break;
        case Qt::Key_F12: j = -3; k = 9; break;

        default: return false;
        }

        return true;
    }
}

#endif
