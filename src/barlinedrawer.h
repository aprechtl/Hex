#ifndef BARLINEDRAWER_H
#define BARLINEDRAWER_H
#include <QtGui/QPen>

class BarLineDrawer
{
public:
    BarLineDrawer();
    void drawBarLines(QPainter *painter, const QRectF &painterRect);
    void setBarLineColor(const QColor &color);
    void setBeatLineColor(const QColor &color);
    void setGridLineColor(const QColor &color);
    void setLineSpacings(double barSpacing, double beatSpacing, double gridSpacing);

private:
    void drawLineSet(unsigned int spacing);

    QPen barLinePen;
    QPen beatLinePen;
    QPen gridLinePen;
    unsigned int barLineSpacing;
    unsigned int beatLineSpacing;
    unsigned int gridLineSpacing;
    QPainter *painter;
    double painterRectLeft;
    double painterRectTop;
    double painterRectRight;
    double painterRectBottom;
};

inline void BarLineDrawer::setBarLineColor(const QColor &color)
{ barLinePen.setColor(color); }

inline void BarLineDrawer::setBeatLineColor(const QColor &color)
{ beatLinePen.setColor(color); }

inline void BarLineDrawer::setGridLineColor(const QColor &color)
{ gridLinePen.setColor(color); }

inline void BarLineDrawer::setLineSpacings(double barSpacing, double beatSpacing, double gridSpacing)
{
    barLineSpacing = barSpacing;
    beatLineSpacing = beatSpacing;
    gridLineSpacing = gridSpacing;
}

#endif
