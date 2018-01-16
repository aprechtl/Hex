#include "barlinecalculator.h"

BarLineCalculator::BarLineCalculator()
{
}

void BarLineCalculator::run(unsigned int numMeasures,
                            unsigned char timeSigNumerator,
                            unsigned char timeSigDenominator,
                            GridInterval gridInterval)
{
    measureLength = ticksPerWholeNote * timeSigNumerator / timeSigDenominator;
    sequencerLength = numMeasures * measureLength;

    switch (gridInterval)
    {
    case HalfNote:
        gridLength = ticksPerHalfNote;
        break;
    case HalfNoteT:
        gridLength = ticksPerHalfNoteT;
        break;
    case QuarterNote:
        gridLength = ticksPerQuarterNote;
        break;
    case QuarterNoteT:
        gridLength = ticksPerQuarterNoteT;
        break;
    case EighthNote:
        gridLength = ticksPerEighthNote;
        break;
    case EighthNoteT:
        gridLength = ticksPerEighthNoteT;
       break;
    case SixteenthNote:
        gridLength = ticksPerSixteenthNote;
        break;
    case SixteenthNoteT:
        gridLength = ticksPerSixteenthNoteT;
        break;
    case ThirtySecondNote:
        gridLength = ticksPerThirtySecondNote;
        break;
    case ThirtySecondNoteT:
        gridLength = ticksPerThirtySecondNoteT;
        break;
    case SixtyFourthNote:
        gridLength = ticksPerSixtyFourthNote;
    }

    bool timeSigIsCompound = (timeSigNumerator != 3 && timeSigNumerator % 3 == 0);
    beatLength = measureLength / ((timeSigIsCompound) ? timeSigNumerator / 3 : timeSigNumerator);
}
