#ifndef BARLINECALCULATOR_H
#define BARLINECALCULATOR_H

class BarLineCalculator
{
public:
    enum GridInterval {HalfNote, HalfNoteT, QuarterNote, QuarterNoteT,
                       EighthNote, EighthNoteT, SixteenthNote, SixteenthNoteT,
                       ThirtySecondNote, ThirtySecondNoteT, SixtyFourthNote};

    enum {ticksPerQuarterNote = 480,
          ticksPerWholeNote = ticksPerQuarterNote * 4,
          ticksPerHalfNote = ticksPerWholeNote / 2,
          ticksPerHalfNoteT = ticksPerWholeNote / 3,
          ticksPerQuarterNoteT = ticksPerWholeNote / 6,
          ticksPerEighthNote = ticksPerWholeNote / 8,
          ticksPerEighthNoteT = ticksPerWholeNote / 12,
          ticksPerSixteenthNote = ticksPerWholeNote / 16,
          ticksPerSixteenthNoteT = ticksPerWholeNote / 24,
          ticksPerThirtySecondNote = ticksPerWholeNote / 32,
          ticksPerThirtySecondNoteT = ticksPerWholeNote / 48,
          ticksPerSixtyFourthNote = ticksPerWholeNote / 64};

    BarLineCalculator();
    void run(unsigned int numMeasures,
             unsigned char timeSigNumerator,
             unsigned char timeSigDenominator,
             GridInterval gridInterval);

    // inline getters
    unsigned int getBeatLength() const {return beatLength;}
    unsigned int getGridLength() const {return gridLength;}
    unsigned int getMeasureLength() const {return measureLength;}
    unsigned int getSequencerLength() const {return sequencerLength;}

private:
    unsigned int beatLength;
    unsigned int gridLength;
    unsigned int measureLength;
    unsigned int sequencerLength;
};

#endif
