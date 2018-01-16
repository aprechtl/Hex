# Hex

Hex is a multi track MIDI sequencer designed for [Dynamic Tonality](http://www.dynamictonality.com). It was written by Anthony Prechtl and Andrew Milne using C++ with the Qt 5 library. This project has been successfully built for Windows and OS X, and would probably work on Linux as well.

The project has been made open source under the terms of the GNU General Public License v3.0. Feel free to contact us if you have other licensing requirements.

## Getting Started

Clone the repository and open the project (Hex.pro) with Qt Creator. Ensure you have a version of Qt 5 installed on your computer. The last version we have tested was 5.8.

### OS X deployment

* After building, open Terminal.
* cd to where Hex.app is located.
* Run `<path to macdeployqt>/macdeployqt Hex.app -verbose=2 -dmg`.
* To set app icon, copy info.plist from the icon folder into Hex.app's Contents folder, and copy icon.icns into Hex.app's Contents > Resources folder.
* Copy the support folder in its entirety to Contents > MacOS.

### Windows deployment

Instructions coming soon. Tested on Windows 7, 8, and 10. We followed the instructions on Qt's website and then copied the support folder to the root directory where Hex.exe is located.

## License

This project is licensed under the GNU General Public License v3.0; see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

Thank you to Gary Scavone for the wonderful [RtMidi](https://github.com/thestk/rtmidi) library.
