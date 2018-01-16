#include "barlinedrawer.h"
#include "abstractsequencerscene.h"
#include <QtGui/QPainter>
#include <math.h>

BarLineDrawer::BarLineDrawer()
{
    barLinePen.setCosmetic(true);
    beatLinePen.setCosmetic(true);
    gridLinePen.setCosmetic(true);
}

void BarLineDrawer::drawBarLines(QPainter *painter, const QRectF &rect)
{
    this->painter = painter;
    painterRectLeft = rect.left();
    painterRectTop = rect.top();
    painterRectRight = rect.right();
    painterRectBottom = rect.bottom();

    painter->setPen(gridLinePen);
    drawLineSet(gridLineSpacing);
    painter->setPen(beatLinePen);
    drawLineSet(beatLineSpacing);
    painter->setPen(barLinePen);
    drawLineSet(barLineSpacing);
}

void BarLineDrawer::drawLineSet(unsigned int spacing)
{    
    for (unsigned int pos = ceil(painterRectLeft / spacing) * spacing; pos < static_cast<unsigned int>(painterRectRight); pos += spacing)
    {
        painter->drawLine(QLineF(pos, painterRectTop, pos, painterRectBottom));
    }
}
