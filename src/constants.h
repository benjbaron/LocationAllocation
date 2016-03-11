#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QColor>

const QColor SHAPEFILE_COL       = QColor(127, 140, 141);
const int    SHAPEFILE_WID       = 2;

const long long MaxTime = std::numeric_limits<long long>::max();

const int GRID_SIZE = 2000;

const QColor CELL_COLOR = Qt::red;
const double CELL_OPACITY = 0.5;
const QColor ORANGE = QColor(255, 127, 0);
const QColor BLUE   = Qt::darkBlue;
const QColor RED    = Qt::darkRed;
const QColor BLACK  = Qt::black;

#endif // CONSTANTS_H
