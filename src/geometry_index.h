#ifndef GEOMETRYINDEX_H
#define GEOMETRYINDEX_H

#include <QPointF>
#include <QSet>
#include <QHash>
#include <qmath.h>

// forward class declaration
class Geometry;

class GeometryIndex {
public:
    GeometryIndex(const QSet<Geometry*>& geometries, double cellSize = 100);
    QSet<Geometry*>* getGeometriesAt(double x, double y);
    QSet<Geometry*>* getGeometriesAt(QPointF p) {
        return getGeometriesAt(p.x(), p.y());
    }
    double getCellSize() {
        return _cellSize;
    }

    QSet<Geometry*>* getGeometriesWithin(double x, double y, double distance);
    QSet<Geometry*>* getGeometriesWithin(QPointF p, double distance) {
        return getGeometriesWithin(p.x(), p.y(), distance);
    }
    const QList<Geometry*>& getGrid();

private:
    double _cellSize;
    QHash<QPoint,QSet<Geometry*>*> _geometryGrid;
    QList<Geometry*> _grid;

    /* private functions */
    QPoint getGridCellAt(double x, double y) {
        return QPoint(qFloor(x / _cellSize), qFloor(y / _cellSize));
    }
    QPoint getGridCellAt(QPointF p) {
        return getGridCellAt(p.x(), p.y());
    }
};

#endif // GEOMETRYINDEX_H
