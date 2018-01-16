#ifndef DYNAMICTONALITY_H
#define DYNAMICTONALITY_H
#include <math.h>
#include <vector>

struct Layout
{
    double omegaX, omegaY, psiY, snuggle;
    char transformCoeffs[4];
};

class DynamicTonality
{
public:
    DynamicTonality();

    // setters
    void setAPSLayout(int layoutID, bool flip = false);
    void setGenerator(double value);
    void setPeriod(double value);

    // getters
    void captureButtonCoordinates(short j, short k, double &x, double &y);
    void convertWickiJKToLayoutJK(short &j, short &k) const;
    double getRotationPeriodSize() const;
    double getRotationAngle() const;
    double getRotationRadians() const;
    double getShearFactor() const;
    double getShearPeriodSize() const;
    double getSnuggle() const;
    double getUntransformedRelativeBeta() const;

    // inline getters
    double generator() const {return m_generator;}
    double period() const {return m_period;}

    // static functions
    static void captureJKFromAxis(unsigned char pitch, short &j, short &k);
    static void captureJKFromPianoMIDI(unsigned char pitch, short &j, short &k);
    static void captureJKFromRelayer(unsigned char channel0To15, unsigned char pitch, short &j, short &k);
    static void captureMIDICCValues(double value0To12288, unsigned char &cc1, unsigned char &cc2, unsigned char &cc3);
    static bool captureMIDIFromJK(short j, short k, unsigned char &channel, unsigned char &pitch);
    static const std::vector<const char *> &getAPSLayoutNames();

    // convenience inline methods
    void captureAlphaMIDICCValues(unsigned char &cc50, unsigned char &cc51, unsigned char &cc52) const
      { captureMIDICCValues(m_period, cc50, cc51, cc52); } // call static version with current alpha value
    void captureBetaMIDICCValues(unsigned char &cc53, unsigned char &cc54, unsigned char &cc55) const
      { captureMIDICCValues(m_generator, cc53, cc54, cc55); } // call static version with current beta value

private:
    // these are initialized in the initializer list
    double m_period;
    double m_generator;

    Layout m_layout;
    double m_oneOverOmegaX;
    double m_psiYOverPeriod;
};

// inline methods
inline double DynamicTonality::getRotationPeriodSize() const { return m_layout.psiY * (cos(getRotationRadians()));}
inline double DynamicTonality::getRotationAngle() const { return getRotationRadians() * -57.295779513082320876798; } // (180/pi)
inline double DynamicTonality::getRotationRadians() const { return atan(m_oneOverOmegaX * (m_layout.omegaY - m_generator * m_psiYOverPeriod)); }
inline double DynamicTonality::getShearPeriodSize() const { return m_layout.psiY; }
inline double DynamicTonality::getShearFactor() const { return -m_oneOverOmegaX * (m_layout.omegaY - m_generator * m_psiYOverPeriod); }
inline double DynamicTonality::getSnuggle() const { return m_layout.snuggle; }
inline void DynamicTonality::setGenerator(double value) { m_generator = value; }
inline void DynamicTonality::setPeriod(double value) { m_period = value; m_psiYOverPeriod = m_layout.psiY / m_period; }

#endif
