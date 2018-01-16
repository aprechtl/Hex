#include "projectsettingsdialog.h"
#include "barlinecalculator.h"
#include "latticemanager.h"
#include "projectsettingscommands.h"
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTreeWidget>

struct LatticeSettingsPreset
{
    const char *name;
    LatticeSettings settings;
    double beta;
};

const short int numPresets = 204;
const short int numPresetCategories = 10;
const short int numPresetsInCategories[] = {16, 14, 21, 23, 22, 30, 22, 20, 17, 19};
const char *presetCategoryTitles[] = {"Popular", "Diatonic Tunings", "5-limit Temperaments", "7-limit temperaments", "MOS Scales (5-9 tones)", "MOS Scales (10-13 tones)", "MOS Scales (14-16 tones)", "Equal Tunings (5-12)", "Equal Tunings (13-16)", "Equal Tunings (17-19)"};

const LatticeSettingsPreset presets[] = {
    // popular
    {"12-TET Diatonic", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 700},
    {"5-TET Diatonic", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 720},
    {"7-TET Diatonic", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 685.7143},
    {"3L, 4s (10-TET)", {3, 4, 10, 7, false, 1200, 342.8572, 400}, 360},
    {"4L, 3s (11-TET)", {4, 3, 10, 7, false, 1200, 300, 342.8572}, 327.2727},
    {"4L, 7s (15-TET)", {4, 7, 10, 5, true, 1200, 300, 327.2727}, 320},
    {"17-TET Diatonic", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 705.8824},
    {"19-TET Diatonic", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 694.7368},
    {"Meantone", {5, 2, 10, 12, false, 1201.70, 686.6857, 721.02}, 697.57},
    {"Porcupine", {1, 6, 10, 11, false, 1196.91, 0, 170.9871}, 162.32},
    {"Magic", {3, 7, 10, 7, false, 1201.28, 360.384, 400.4267}, 380.80},
    {"Pajara", {1, 4, 20, 11, false, 598.45, 0, 149.6125}, 106.57},
    {"Blacksmith", {1, 1, 50, 11, false, 239.18, 0, 119.59}, 83.83},
    {"Keemun", {4, 7, 10, 5, true, 1203.19, 300.7975, 328.1427}, 317.84},
    {"Bohlen-Pierce", {4, 5, 6, 5, false, 1903.37, 422.9711, 475.8425}, 440.43},
    {"Harmonic Table", {1, 13, 14, 11, false, 700, 380, 410}, 400},

    // diatonic tunings
    {"5-TET Diatonic", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 720},
    {"22-TET Diatonic", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 709.0909},
    {"17-TET Diatonic", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 705.8824},
    {"Pythagorean", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 701.9550},
    {"12-TET Diatonic", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 700},
    {"1/6-comma Meantone", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 698.3706},
    {"1/5-comma Meantone", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 697.6537},
    {"TOP-Meantone", {5, 2, 10, 12, false, 1201.70, 686.69, 721.02}, 697.57},
    {"1/4-comma Meantone", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 696.5784},
    {"2/7-comma Meantone", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 695.8103},
    {"1/3-comma Meantone", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 694.7862},
    {"19-TET Diatonic", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 694.7368},
    {"26-TET Diatonic", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 692.3077},
    {"7-TET Diatonic", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 685.7143},

    // 5-limit temperaments
    {"Dicot", {1, 2, 10, 11, false, 1207.66, 0, 402.5533}, 353.22},
    {"Father", {2, 1, 10, 11, false, 1185.90, 395.3, 592.95}, 447.40},
    {"Bug", {1, 3, 10, 11, false, 1207.66, 0, 301.915}, 260.30},
    {"Augmented", {1, 1, 30, 11, false, 399.02, 0, 199.51}, 93.15},
    {"Meantone", {5, 2, 10, 12, false, 1201.70, 686.6857, 721.02}, 697.57},
    {"Porcupine", {1, 6, 10, 11, false, 1196.91, 0, 170.9871}, 162.32},
    {"Mavila", {2, 5, 10, 12, false, 1206.55, 603.275, 689.4571}, 685.03},
    {"Srutal", {1, 4, 20, 11, false, 599.56, 0, 119.912}, 104.70},
    {"Dimipent", {1, 1, 40, 11, false, 299.16, 0, 149.58}, 101.67},
    {"Magic", {3, 7, 10, 7, false, 1201.28, 360.384, 400.4267}, 380.80},
    {"Negripent", {1, 9, 10, 11, false, 1201.82, 0, 120.182}, 126.14},
    {"Blackwood", {1, 1, 50, 11, false, 238.87, 0, 119.435}, 80.09},
    {"Hanson", {4, 7, 10, 5, true, 1200.29, 300.0725, 327.3518}, 317.07},
    {"Ripple", {1, 10, 10, 11, false, 1203.32, 0, 109.3927}, 101.99},
    {"Tetracot", {7, 6, 10, 3, false, 1199.03, 171.2900, 184.4662}, 176.11},
    {"Passion", {12, 1, 10, 11, false, 1198.31, 92.1777, 99.8592}, 98.40},
    {"Würschmidt", {3, 13, 10, 7, false, 1199.69, 374.9031, 399.8967}, 387.64},
    {"Helmholtz", {12, 5, 10, 14, true, 1200.07, 700.0408, 705.9235}, 701.79},
    {"Superpyth", {5, 12, 10, 14, true, 1197.60, 704.4706, 718.56}, 708.17},
    {"Amity", {7, 11, 10, 6, false, 1199.85, 333.2917, 342.8143}, 339.47},
    {"Sensipent", {8, 11, 10, 8, false, 1199.59, 441.8032, 449.6926}, 442.99},

    // 7-limit temperaments
    {"Pajara", {1, 4, 20, 11, false, 598.45, 0, 149.6125}, 106.57},
    {"Dimisept", {1, 1, 40, 11, false, 298.53, 0, 149.2650}, 101.45},
    {"Negrisept", {1, 8, 10, 11, false, 1203.19, 0, 133.6878}, 124.84},
    {"Blacksmith", {1, 1, 50, 11, false, 239.18, 0, 119.59}, 83.83},
    {"Keemun", {4, 7, 10, 5, true, 1203.19, 300.7975, 328.1427}, 317.84},
    {"Augene", {1, 3, 30, 11, false, 399.02, 0, 99.7550}, 92.46},
    {"Injera", {1, 5, 20, 11, false, 600.89, 0, 100.1483}, 93.61},
    {"Dominant", {5, 7, 10, 14, true, 1195.23, 697.2175, 717.138}, 699.35},
    {"August", {1, 2, 30, 11, false, 399.99, 0, 133.3300}, 107.31},
    {"Nautilus", {1, 12, 10, 11, false, 1202.66, 0, 92.5123}, 82.67},
    {"Hedgehog", {4, 3, 20, 7, false, 598.45, 149.6125, 170.9857}, 162.32},
    {"Doublewide", {2, 5, 20, 12, false, 599.28, 299.64, 342.4457}, 326.96},
    {"Semaphore", {5, 9, 10, 4, true, 1203.67, 240.734, 257.9293}, 252.48},
    {"Myna", {4, 11, 10, 5, true, 1198.83, 299.7075, 319.688}, 309.89},
    {"Porcupine", {7, 8, 10, 2, false, 1196.91, 159.588, 170.9871}, 162.32},
    {"Magic", {3, 13, 10, 7, false, 1201.28, 375.4000, 400.4267}, 380.80},
    {"Lemba", {5, 3, 20, 7, true, 601.7, 225.6375, 240.6800}, 230.87},
    {"Superpyth", {5, 12, 10, 14, true, 1197.60, 704.4706, 718.5600}, 708.17},
    {"Beatles", {10, 7, 10, 6, true, 1197.1, 352.0882, 359.1300}, 354.72},
    {"Sensisept", {8, 11, 10, 8, false, 1198.39, 441.5121, 449.3962}, 443.16},
    {"Meantone", {12, 7, 10, 13, false, 1201.70, 695.7211, 700.9917}, 697.57},
    {"Flattone", {7, 12, 10, 13, false, 1202.54, 687.1657, 696.2074}, 695.4},
    {"Liese", {17, 2, 10, 12, true, 1202.62, 565.9388, 569.6622}, 569.05},

    // MOS scales (5-9 tones)
    {"1L, 4s (6-TET)", {1, 4, 10, 11, false, 1200, 0, 240}, 200},
    {"2L, 3s (7-TET)", {2, 3, 10, 12, true, 1200, 480, 600}, 514.29},
    {"3L, 2s (8-TET)", {3, 2, 10, 12, true, 1200, 400, 480}, 450},
    {"4L, 1s (9-TET)", {4, 1, 10, 11, false, 1200, 240, 300}, 266.6667},
    {"1L, 5s (7-TET)", {1, 5, 10, 11, false, 1200, 0, 200}, 171.43},
    {"5L, 1s (11-TET)", {5, 1, 10, 11, false, 1200, 200, 240}, 218.18},
    {"1L, 6s (8-TET)", {1, 6, 10, 11, false, 1200, 0, 171.4286}, 150},
    {"2L, 5s (9-TET)", {2, 5, 10, 12, false, 1200, 600, 685.7143}, 666.6667},
    {"3L, 4s (10-TET)", {3, 4, 10, 7, false, 1200, 342.8572, 400}, 360},
    {"4L, 3s (11-TET)", {4, 3, 10, 7, false, 1200, 300, 342.8572}, 327.2727},
    {"5L, 2s (12-TET)", {5, 2, 10, 12, false, 1200, 685.7143, 720}, 700},
    {"6L, 1s (13-TET)", {6, 1, 10, 11, false, 1200, 171.4286, 200}, 184.6154},
    {"1L, 7s (9-TET)", {1, 7, 10, 11, false, 1200, 0, 150}, 133.3333},
    {"3L, 5s (11-TET)", {3, 5, 10, 7, true, 1200, 400, 450}, 436.36},
    {"5L, 3s (13-TET)", {5, 3, 10, 7, true, 1200, 450, 480}, 461.54},
    {"7L, 1s (15-TET)", {7, 1, 10, 11, false, 1200, 150, 171.4286}, 160},
    {"1L, 8s (10-TET)", {1, 8, 10, 11, false, 1200, 0, 133.3333}, 120},
    {"2L, 7s (11-TET)", {2, 7, 10, 12, true, 1200, 533.3333, 600}, 545.45},
    {"4L, 5s (13-TET)", {4, 5, 10, 5, false, 1200, 266.6667, 300}, 276.92},
    {"5L, 4s (14-TET)", {5, 4, 10, 5, false, 1200, 240, 266.6667}, 257.14},
    {"7L, 2s (16-TET)", {7, 2, 10, 12, true, 1200, 514.2857, 533.3333}, 525},
    {"8L, 1s (17-TET)", {8, 1, 10, 11, false, 1200, 133.3333, 150}, 141.1765},

    // MOS scales (10-13 tones)
    {"1L, 9s (11-TET)", {1, 9, 10, 11, false, 1200, 0, 120}, 109.09},
    {"3L, 7s (13-TET)", {3, 7, 10, 7, false, 1200, 360, 400}, 369.23},
    {"7L, 3s (17-TET)", {7, 3, 10, 7, false, 1200, 342.8572, 360}, 352.94},
    {"9L, 1s (19-TET)", {9, 1, 10, 11, false, 1200, 120, 133.3333}, 126.32},
    {"1L, 10s (12-TET)", {1, 10, 10, 11, false, 1200, 0, 109.0909}, 100},
    {"2L, 9s (13-TET)", {2, 9, 10, 12, true, 1200, 545.4545, 600}, 553.85},
    {"3L, 8s (14-TET)", {3, 8, 10, 7, true, 1200, 400, 436.3636}, 428.57},
    {"4L, 7s (15-TET)", {4, 7, 10, 5, true, 1200, 300, 327.2727}, 320},
    {"5L, 6s (16-TET)", {5, 6, 10, 4, false, 1200, 218.1818, 240}, 225},
    {"6L, 5s (17-TET)", {6, 5, 10, 4, false, 1200, 200, 218.1818}, 211.76},
    {"7L, 4s (18-TET)", {7, 4, 10, 5, true, 1200, 327.2727, 342.8572}, 333.3333},
    {"8L, 3s (19-TET)", {8, 3, 10, 7, true, 1200, 442.11, 436.3636}, 450},
    {"9L, 2s (20-TET)", {9, 2, 10, 12, true, 1200, 533.3333, 545.4545}, 540},
    {"10L, 1s (21-TET)", {10, 1, 10, 11, false, 1200, 109.0909, 120}, 114.29},
    {"1L, 11s (13-TET)", {1, 11, 10, 11, false, 1200, 0, 100}, 92.31},
    {"5L, 7s (17-TET)", {5, 7, 10, 14, true, 1200, 700, 720}, 705.88},
    {"7L, 5s (19-TET)", {7, 5, 10, 14, true, 1200, 685.7143, 700}, 694.74},
    {"11L, 1s (23-TET)", {11, 1, 10, 11, false, 1200, 100, 109.0909}, 104.35},
    {"1L, 12s (14-TET)", {1, 12, 10, 11, false, 1200, 0, 92.3077}, 85.71},
    {"2L, 11s (15-TET)", {2, 11, 10, 12, true, 1200, 553.8461, 600}, 560},
    {"3L, 10s (16-TET)", {3, 10, 10, 7, false, 1200, 369.2308, 400}, 375},
    {"4L, 9s (17-TET)", {4, 9, 10, 5, false, 1200, 276.9231, 300}, 282.35},
    {"5L, 8s (18-TET)", {5, 8, 10, 9, false, 1200, 461.5385, 480}, 466.6667},
    {"6L, 7s (19-TET)", {6, 7, 10, 3, false, 1200, 184.6154, 200}, 189.47},
    {"7L, 6s (20-TET)", {7, 6, 10, 3, false, 1200, 171.4286, 184.6154}, 180},
    {"8L, 5s (21-TET)", {8, 5, 10, 9, false, 1200, 450, 461.5385}, 457.14},
    {"9L, 4s (22-TET)", {9, 4, 10, 5, false, 1200, 266.6667, 276.9231}, 272.73},
    {"10L, 3s (23-TET)", {10, 3, 10, 7, false, 1200, 360, 369.2308}, 365.22},
    {"11L, 2s (24-TET)", {11, 2, 10, 12, true, 1200, 545.4545, 553.8461}, 550},
    {"12L, 1s (25-TET)", {12, 1, 10, 11, false, 1200, 92.3077, 100}, 96},

    // MOS scales (14-16 tones)
    {"1L, 13s (15-TET)", {1, 13, 10, 11, false, 1200, 0, 85.71429}, 80},
    {"3L, 11s (17-TET)", {3, 11, 10, 7, true, 1200, 400, 428.5714}, 423.53},
    {"5L, 9s (19-TET)", {5, 9, 10, 4, true, 1200, 240, 257.1429}, 252.63},
    {"9L, 5s (23-TET)", {9, 5, 10, 4, true, 1200, 257.1429, 266.6667}, 260.87},
    {"11L, 3s (25-TET)", {11, 3, 10, 7, true, 1200, 428.5714, 436.3636}, 432},
    {"13L, 1s (27-TET)", {13, 1, 10, 11, false, 1200, 85.7143, 92.3077}, 88.89},
    {"1L, 14s (16-TET)", {1, 14, 10, 11, false, 1200, 0, 80}, 75},
    {"2L, 13s (17-TET)", {2, 13, 10, 12, true, 1200, 560, 600}, 564.71},
    {"4L, 11s (19-TET)", {4, 11, 10, 5, true, 1200, 300, 320}, 315.79},
    {"7L, 8s (22-TET)", {7, 8, 10, 2, false, 1200, 160, 171.4286}, 163.64},
    {"8L, 7s (23-TET)", {8, 7, 10, 2, false, 1200, 150, 160}, 156.52},
    {"11L, 4s (26-TET)", {11, 4, 10, 5, true, 1200, 320, 327.2727}, 323.08},
    {"13L, 2s (28-TET)", {13, 2, 10, 12, true, 1200, 553.8461, 560}, 557.14},
    {"14L, 1s (29-TET)", {14, 1, 10, 11, false, 1200, 80, 85.7143}, 82.76},
    {"1L, 15s (17-TET)", {1, 15, 10, 11, false, 1200, 0, 75}, 70.59},
    {"3L, 13s (19-TET)", {3, 13, 10, 7, false, 1200, 375, 400}, 378.95},
    {"5L, 11s (21-TET)", {5, 11, 10, 4, false, 1200, 225, 240}, 228.57},
    {"7L, 9s (23-TET)", {7, 9, 10, 10, true, 1200, 514.2857, 525}, 521.74},
    {"9L, 7s (25-TET)", {9, 7, 10, 10, true, 1200, 525, 533.3333}, 528},
    {"11L, 5s (27-TET)", {11, 5, 10, 4, false, 1200, 218.1818, 225}, 222.22},
    {"13L, 3s (29-TET)", {13, 3, 10, 7, false, 1200, 369.2308, 375}, 372.41},
    {"15L, 1s (31-TET)", {15, 1, 10, 11, false, 1200, 75, 80}, 77.42},

    // equal tunings (5-12)
    {"5-TET (1+4)", {1, 4, 10, 11, false, 1200, 0, 300}, 240},
    {"5-TET (2+3)", {2, 3, 10, 12, true, 1200, 400, 600}, 480},
    {"6-TET (1+5)", {1, 5, 10, 11, false, 1200, 0, 240}, 200},
    {"7-TET (1+6)", {1, 6, 10, 11, false, 1200, 0, 200}, 171.4286},
    {"7-TET (2+5)", {2, 5, 10, 12, true, 1200, 600, 720}, 514.2857},
    {"7-TET (3+4)", {3, 4, 10, 7, false, 1200, 300, 400}, 342.8571},
    {"8-TET (1+7)", {1, 7, 10, 11, false, 1200, 0, 171.4286}, 150},
    {"8-TET (3+5)", {3, 5, 10, 7, true, 1200, 400, 480}, 450},
    {"9-TET (1+8)", {1, 8, 10, 11, false, 1200, 0, 150}, 133.3333},
    {"9-TET (2+7)", {2, 7, 10, 12, true, 1200, 514.2857, 600}, 533.3333},
    {"9-TET (4+5)", {4, 5, 10, 5, false, 1200, 240, 300}, 266.6667},
    {"10-TET (1+9)", {1, 9, 10, 11, false, 1200, 0, 133.3333}, 120},
    {"10-TET (3+7)", {3, 7, 10, 7, false, 1200, 342.8572, 400}, 360},
    {"11-TET (1+10)", {1, 10, 10, 11, false, 1200, 0, 120}, 109.0909},
    {"11-TET (2+9)", {2, 9, 10, 12, true, 1200, 533.3333, 600}, 545.4545},
    {"11-TET (3+8)", {3, 8, 10, 7, true, 1200, 400, 450}, 436.3636},
    {"11-TET (4+7)", {4, 7, 10, 5, true, 1200, 300, 342.8572}, 327.2727},
    {"11-TET (5+6)", {5, 6, 10, 4, false, 1200, 200, 240}, 218.1818},
    {"12-TET (1+11)", {1, 11, 10, 11, false, 1200, 0, 109.0909}, 100},
    {"12-TET (5+7)", {5, 7, 10, 14, true, 1200, 685.7143, 720}, 700},

    // equal tunings (13-16)
    {"13-TET (1+12)", {1, 12, 10, 11, false, 1200, 0, 100}, 92.3077},
    {"13-TET (2+11)", {2, 11, 10, 12, true, 1200, 545.4545, 600}, 553.8462},
    {"13-TET (3+10)", {3, 10, 10, 7, false, 1200, 360, 400}, 369.2308},
    {"13-TET (4+9)", {4, 9, 10, 5, false, 1200, 266.6667, 300}, 276.9231},
    {"13-TET (5+8)", {5, 8, 10, 9, false, 1200, 450, 480}, 461.5385},
    {"13-TET (6+7)", {6, 7, 10, 3, false, 1200, 171.4286, 200}, 184.6154},
    {"14-TET (1+13)", {1, 13, 10, 11, false, 1200, 0, 92.3077}, 85.7143},
    {"14-TET (3+11)", {3, 11, 10, 7, true, 1200, 400, 436.3636}, 428.5714},
    {"14-TET (5+9)", {5, 9, 10, 4, true, 1200, 240, 266.6667}, 257.1429},
    {"15-TET (1+14)", {1, 14, 10, 11, false, 1200, 0, 85.7143}, 80},
    {"15-TET (2+13)", {2, 13, 10, 12, true, 1200, 553.8461, 600}, 560},
    {"15-TET (4+11)", {4, 11, 10, 5, true, 1200, 300, 327.2727}, 320},
    {"15-TET (7+8)", {7, 8, 10, 2, false, 1200, 150, 171.4286}, 160},
    {"16-TET (1+15)", {1, 15, 10, 11, false, 1200, 0, 80}, 75},
    {"16-TET (3+13)", {3, 13, 10, 7, false, 1200, 368.2308, 400}, 375},
    {"16-TET (5+11)", {5, 11, 10, 4, false, 1200, 218.1818, 240}, 225},
    {"16-TET (7+9)", {7, 9, 10, 10, true, 1200, 514.2857, 533.3333}, 525},

    // equal tunings (17-19)
    {"17-TET (1+16)", {1, 16, 10, 11, false, 1200, 0, 75}, 70.5882},
    {"17-TET (2+15)", {2, 15, 10, 12, true, 1200, 560, 600}, 564.7059},
    {"17-TET (3+14)", {3, 14, 10, 7, true, 1200, 400, 428.5714}, 423.5294},
    {"17-TET (4+13)", {4, 13, 10, 5, false, 1200, 276.9231, 300}, 282.3529},
    {"17-TET (5+12)", {5, 12, 10, 14, true, 1200, 700, 720}, 705.8824},
    {"17-TET (6+11)", {6, 11, 10, 3, true, 1200, 200, 218.1818}, 211.7647},
    {"17-TET (7+10)", {7, 10, 10, 6, true, 1200, 342.8572, 360}, 352.9412},
    {"17-TET (8+9)", {8, 9, 10, 1, false, 1200, 133.3333, 150}, 141.1765},
    {"18-TET (1+17)", {1, 17, 10, 11, false, 1200, 0, 70.5882}, 66.6667},
    {"18-TET (5+13)", {5, 13, 10, 9, false, 1200, 461.5385, 480}, 466.6667},
    {"18-TET (7+11)", {7, 11, 10, 6, false, 1200, 327.2727, 342.8572}, 333.3333},
    {"19-TET (1+18)", {1, 18, 10, 11, false, 1200, 0, 66.6667}, 63.1579},
    {"19-TET (2+17)", {2, 17, 10, 12, true, 1200, 564.7059, 600}, 568.4211},
    {"19-TET (3+16)", {3, 16, 10, 7, false, 1200, 375, 400}, 378.9474},
    {"19-TET (4+15)", {4, 15, 10, 5, true, 1200, 300, 320}, 315.7895},
    {"19-TET (5+14)", {5, 14, 10, 4, true, 1200, 240, 257.1429}, 252.6316},
    {"19-TET (6+13)", {6, 13, 10, 3, false, 1200, 184.6154, 200}, 189.4737},
    {"19-TET (7+12)", {7, 12, 10, 13, false, 1200, 685.7143, 700}, 694.7368},
    {"19-TET (8+11)", {8, 11, 10, 8, false, 1200, 436.3636, 450}, 442.1053},
    {"19-TET (9+10)", {9, 10, 10, 0, false, 1200, 120, 133.3333}, 126.3158}
};

ProjectSettingsDialog::ProjectSettingsDialog(LatticeManager *manager, QUndoStack *stack, QWidget *parent)
    : QDialog(parent, Qt::Tool), latticeManager(manager), undoStack(stack)
{
    setWindowTitle(tr("Project Settings"));

    setFocusPolicy(Qt::ClickFocus); // makes it so that you can clear focus from widgets by clicking anywhere in the dialog
    setFocus();

    // ====================================================== GENERAL SETTINGS
    timeSigNumerBox = new QComboBox;
    timeSigDenomBox = new QComboBox;
    const QStringList timeSigNumeratorQStrings = QStringList()
            << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10" << "11" << "12";
    const QStringList timeSigDenominatorQStrings = QStringList()
            << "2" << "4" << "8" << "16";
    timeSigNumerBox->addItems(timeSigNumeratorQStrings);
    timeSigDenomBox->addItems(timeSigDenominatorQStrings);

    numMeasuresBox = new QSpinBox;
    numMeasuresBox->setRange(16, 1000);
    numMeasuresBox->setKeyboardTracking(false);

    tempoBox = new QDoubleSpinBox;
    tempoBox->setRange(20, 400);
    tempoBox->setKeyboardTracking(false);

    QHBoxLayout *timeSignatureLayout = new QHBoxLayout;
    timeSignatureLayout->addWidget(timeSigNumerBox);
    timeSignatureLayout->addWidget(new QLabel("/"));
    timeSignatureLayout->addWidget(timeSigDenomBox);
    timeSignatureLayout->addStretch();

    QGroupBox *generalSettingsGroup = new QGroupBox(tr("General Settings"));
    QFormLayout *generalSettingsLayout = new QFormLayout(generalSettingsGroup);
    generalSettingsLayout->addRow(tr("Time signature:"), timeSignatureLayout);
    generalSettingsLayout->addRow(tr("Num measures:"), numMeasuresBox);
    generalSettingsLayout->addRow(tr("Tempo (BPM):"), tempoBox);
    // =======================================================================

    // =========================================================== DT SETTINGS
    // preset popup dialog
    QTreeWidget *presetTreeWidget = new QTreeWidget(this);
    presetTreeWidget->header()->hide();

    // TODO: this seems to be the only way to make presetTreeWidget expand vertically
    QSizePolicy policy = presetTreeWidget->sizePolicy();
    policy.setVerticalStretch(1);
    presetTreeWidget->setSizePolicy(policy);

    int presetIndex = 0;

    for (int i = 0; i < numPresetCategories; ++i)
    {
        QTreeWidgetItem *groupItem = new QTreeWidgetItem(presetTreeWidget);
        groupItem->setText(0, presetCategoryTitles[i]);
        groupItem->setData(0, Qt::UserRole, -1); // data is -1, which indicates it's a category and not a preset

        for (int j = 0; j < numPresetsInCategories[i]; ++j)
        {
            QTreeWidgetItem *presetItem = new QTreeWidgetItem(groupItem);
            presetItem->setText(0, presets[presetIndex].name);
            presetItem->setData(0, Qt::UserRole, presetIndex);
            ++presetIndex;
        }
    }

    largeStepBox = new QSpinBox;
    smallStepBox = new QSpinBox;
    largeStepBox->setMinimum(1);
    smallStepBox->setMinimum(1);
    largeStepBox->setKeyboardTracking(false);
    smallStepBox->setKeyboardTracking(false);

    APSComboBox = new QComboBox;
    for (size_t i = 0; i < DynamicTonality::getAPSLayoutNames().size(); ++i)
    {
        APSComboBox->addItem(DynamicTonality::getAPSLayoutNames()[i]);
    }

    flipLatticeCheckBox = new QCheckBox(tr("Flip"));

    QHBoxLayout *APSLayout = new QHBoxLayout;
    APSLayout->addWidget(APSComboBox, 1);
    APSLayout->addWidget(flipLatticeCheckBox);

    periodBox = new QDoubleSpinBox;
    periodBox->setRange(0, 12288);
    periodBox->setKeyboardTracking(false);
    numPeriodsBox = new QSpinBox;
    numPeriodsBox->setRange(1, HexSettings::numJ);

    betaSliderMinBox = new QDoubleSpinBox;
    betaSliderMaxBox = new QDoubleSpinBox;
    betaSliderMinBox->setRange(0, 12288);
    betaSliderMaxBox->setRange(0, 12288);
    betaSliderMinBox->setKeyboardTracking(false);
    betaSliderMaxBox->setKeyboardTracking(false);

    QGroupBox *latticeSettingsGroup = new QGroupBox(tr("Lattice Settings"));
    QFormLayout *latticeSettingsLayout = new QFormLayout(latticeSettingsGroup);
    latticeSettingsLayout->addRow(new QLabel(tr("Presets:")));
    latticeSettingsLayout->addRow(presetTreeWidget);
    latticeSettingsLayout->addRow(tr("Num large steps:"), largeStepBox);
    latticeSettingsLayout->addRow(tr("Num small steps:"), smallStepBox);
    latticeSettingsLayout->addRow(tr("Layouts:"), APSLayout);
    latticeSettingsLayout->addRow(tr("Period:"), periodBox);
    latticeSettingsLayout->addRow(tr("Num periods:"), numPeriodsBox);
    latticeSettingsLayout->addRow(tr("Tuning slider min:"), betaSliderMinBox);
    latticeSettingsLayout->addRow(tr("Tuning slider max:"), betaSliderMaxBox);
    // =======================================================================

    QVBoxLayout *dialogMainLayout = new QVBoxLayout(this);
    dialogMainLayout->addWidget(generalSettingsGroup);
    dialogMainLayout->addWidget(latticeSettingsGroup);

    connect(presetTreeWidget, &QTreeWidget::itemClicked, [=](QTreeWidgetItem *item, int){setLatticePreset(item->data(0, Qt::UserRole).toInt());});
    connect(timeSigNumerBox,SIGNAL(currentIndexChanged(int)),this,SLOT(createNewChangeProjectTimingCommand()));
    connect(timeSigDenomBox,SIGNAL(currentIndexChanged(int)),this,SLOT(createNewChangeProjectTimingCommand()));
    connect(numMeasuresBox,SIGNAL(valueChanged(int)),this,SLOT(createNewChangeProjectTimingCommand()));
    connect(largeStepBox,SIGNAL(valueChanged(int)),this,SLOT(createNewChangeLatticeCommand()));
    connect(smallStepBox,SIGNAL(valueChanged(int)),this,SLOT(createNewChangeLatticeCommand()));
    connect(numPeriodsBox,SIGNAL(valueChanged(int)),this,SLOT(createNewChangeLatticeCommand()));
    connect(APSComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(createNewChangeLatticeCommand()));
    connect(flipLatticeCheckBox,SIGNAL(toggled(bool)),this,SLOT(createNewChangeLatticeCommand()));
    connect(periodBox,SIGNAL(valueChanged(double)),this,SLOT(createNewChangeLatticeCommand()));
    connect(betaSliderMinBox, SIGNAL(valueChanged(double)),this,SLOT(createNewChangeLatticeCommand()));
    connect(betaSliderMaxBox,SIGNAL(valueChanged(double)),this,SLOT(createNewChangeLatticeCommand()));

#ifdef Q_OS_MAC
    static const int spinBoxWidth = 100; // on Mac, QFormLayout doesn't expand widgets, and the default width is too small
    numMeasuresBox->setFixedWidth(spinBoxWidth);
    tempoBox->setFixedWidth(spinBoxWidth);
    largeStepBox->setFixedWidth(spinBoxWidth);
    smallStepBox->setFixedWidth(spinBoxWidth);
    periodBox->setFixedWidth(spinBoxWidth);
    numPeriodsBox->setFixedWidth(spinBoxWidth);
    betaSliderMinBox->setFixedWidth(spinBoxWidth);
    betaSliderMaxBox->setFixedWidth(spinBoxWidth);
#endif
}

void ProjectSettingsDialog::createNewChangeLatticeCommand()
{
    if (suppressChangeLatticeCommands)
        return;

    undoStack->push(new ChangeLatticeSettingsCommand(this, latticeSettings, getCurrentLatticeSettings()));
}

void ProjectSettingsDialog::createNewChangeProjectTimingCommand()
{
    if (suppressProjectTimingCommands)
        return;

    undoStack->push(new ChangeProjectTimingCommand(this, timingSettings, getCurrentTimingSettings()));
}

LatticeSettings ProjectSettingsDialog::getCurrentLatticeSettings()
{
    LatticeSettings settings = {static_cast<unsigned char>(largeStepBox->value()),
                                static_cast<unsigned char>(smallStepBox->value()),
                                static_cast<unsigned char>(numPeriodsBox->value()),
                                static_cast<unsigned char>(APSComboBox->currentIndex()),
                                flipLatticeCheckBox->isChecked(),
                                periodBox->value(),
                                betaSliderMinBox->value(),
                                betaSliderMaxBox->value()};
    return settings;
}

TimingSettings ProjectSettingsDialog::getCurrentTimingSettings()
{
    TimingSettings settings = {timeSigNumerBox->currentText().toInt(),
                               timeSigDenomBox->currentText().toInt(),
                               numMeasuresBox->value(),
                               tempoBox->value()};
    return settings;
}

int ProjectSettingsDialog::numMeasures() const
{ return numMeasuresBox->value(); }

void ProjectSettingsDialog::setLatticePreset(int preset)
{
    if (preset < 0) // preset category titles are -1, so should be ignored
        return;

    setLatticeSettings(presets[preset].settings);

    createNewChangeLatticeCommand();

    emit betaChanged(presets[preset].beta);
    emit betaSliderRangeChanged(betaSliderMinBox->value(), betaSliderMaxBox->value());
}

void ProjectSettingsDialog::setLatticeSettings(const LatticeSettings &settings)
{
    suppressChangeLatticeCommands = true;

    APSComboBox->setCurrentIndex(settings.apsLayout);
    smallStepBox->setValue(settings.numSmallSteps);
    largeStepBox->setValue(settings.numLargeSteps);
    numPeriodsBox->setValue(settings.numPeriods);
    flipLatticeCheckBox->setChecked(settings.flipped);
    periodBox->setValue(settings.alpha);
    betaSliderMinBox->setValue(settings.betaSliderMin);
    betaSliderMaxBox->setValue(settings.betaSliderMax);

    updateStepBoxMaxima();

    suppressChangeLatticeCommands = false;

    latticeSettings = getCurrentLatticeSettings();

    latticeManager->setAlpha(settings.alpha);
    latticeManager->drawLattice(latticeSettings);

    emit betaSliderRangeChanged(betaSliderMinBox->value(), betaSliderMaxBox->value());
}

void ProjectSettingsDialog::setTimingSettings(const TimingSettings &settings)
{
    suppressProjectTimingCommands = true;
    timeSigNumerBox->setCurrentIndex(timeSigNumerBox->findText(QString::number(settings.timeSigNumerator)));
    timeSigDenomBox->setCurrentIndex(timeSigDenomBox->findText(QString::number(settings.timeSigDenominator)));
    numMeasuresBox->setValue(settings.numMeasures);
    tempoBox->setValue(settings.tempo);
    suppressProjectTimingCommands = false;

    timingSettings = getCurrentTimingSettings();
    emit projectTimingUpdated();
}

double ProjectSettingsDialog::tempoBPM() const
{ return tempoBox->value(); }

double ProjectSettingsDialog::tempoTicksPerMS() const
{
    double beatsPerSecond = tempoBox->value() * .016666666666666667; // divided by 60
    bool timeSigIsCompound = (timeSigNumerBox->currentText().toInt() != 3
                           && timeSigNumerBox->currentText().toInt() % 3 == 0);
    double quarterNotesPerBeat = ((!timeSigIsCompound) ? 4. : 6.) / static_cast<double>(timeSigDenomBox->currentText().toInt());
    double quarterNotesPerSecond = beatsPerSecond * quarterNotesPerBeat;
    return quarterNotesPerSecond * BarLineCalculator::ticksPerQuarterNote * .001;
}

int ProjectSettingsDialog::timeSigDen() const
{ return timeSigDenomBox->currentText().toInt(); }

int ProjectSettingsDialog::timeSigNum() const
{ return timeSigNumerBox->currentText().toInt(); }

void ProjectSettingsDialog::updateStepBoxMaxima()
{
    // 2L + s must be less than or equal to number of K values
    largeStepBox->setMaximum((HexSettings::numK - smallStepBox->value()) / 2);
    smallStepBox->setMaximum(HexSettings::numK - largeStepBox->value() - largeStepBox->value());
}
