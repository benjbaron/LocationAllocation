#ifndef GEOMETRYINDEX_H
#define GEOMETRYINDEX_H

#include <QPointF>
#include <QSet>
#include <qmath.h>
#include "trace_layer.h"
#include "geometries.h"

class Geometry;

class GeometryIndex
{
public:
    GeometryIndex(QSet<Geometry*>& geometries, double cellSize = 100);
    void getGeometriesAt(QSet<Geometry*>* geometries, double x, double y);
    void getGeometriesAt(QSet<Geometry*>* geometries, QPointF p) {
        return getGeometriesAt(geometries, p.x(), p.y());
    }

    // Factory method
    static GeometryIndex* make_geometryIndex(Trace* trace,
                                             double sampling = -1, double startTime = -1, double endTime = -1,
                                             double geometryCellsSize = -1,
                                             GeometryType geometryType = NoneType,
                                             QString geometryCirclesFile = QString());

private:
    double _cellSize;
    QHash<QPoint,QSet<Geometry*>*> _geometryGrid;

    /* private functions */
    QPoint getGridCellAt(double x, double y) {
        return QPoint((int)qFloor(x / _cellSize), (int)qFloor(y / _cellSize));
    }
    QPoint getGridCellAt(QPointF p) { return getGridCellAt(p.x(), p.y()); }
};

#endif // GEOMETRYINDEX_H
