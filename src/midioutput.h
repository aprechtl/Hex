#ifndef MIDIHANDLER_H
#define MIDIHANDLER_H
#include "midiport.h"
#include "dynamictonality.h"
#include "sequencerevent.h"
#include "rtm/RtMidi.h"
#include <vector>

class MIDIOutput : public MIDIPort
{
    friend class MIDIPortManager;

public:
    // inline methods (below)
    void sendMessage(unsigned char a, unsigned char b);
    void sendMessage(unsigned char a, unsigned char b, unsigned char c);
    void sendMessageIfOpen(unsigned char a, unsigned char b);
    void sendMessageIfOpen(unsigned char a, unsigned char b, unsigned char c);

    bool sendsDTMessages; // public member

private:
    MIDIOutput(MIDIPortManager *manager);
    ~MIDIOutput();
    bool testPortNameForFeedback(const std::string &name) const;

    std::vector<unsigned char> midi2;
    std::vector<unsigned char> midi3;
};

inline void MIDIOutput::sendMessage(unsigned char a, unsigned char b)
{
    midi2[0] = a;
    midi2[1] = b;
    static_cast<RtMidiOut*>(rtMidi)->sendMessage(&midi2);
}

inline void MIDIOutput::sendMessage(unsigned char a, unsigned char b, unsigned char c)
{
    midi3[0] = a;
    midi3[1] = b;
    midi3[2] = c;
    static_cast<RtMidiOut*>(rtMidi)->sendMessage(&midi3);
}

inline void MIDIOutput::sendMessageIfOpen(unsigned char a, unsigned char b)
{
    if (!isOpen())
        return;

    sendMessage(a, b);
}

inline void MIDIOutput::sendMessageIfOpen(unsigned char a, unsigned char b, unsigned char c)
{
    if (!isOpen())
        return;

    sendMessage(a, b, c);
}

#endif
