#include "geometry_index.h"
#include "geometries.h"

inline uint qHash(const QPointF &key) {
    return qHash(key.x()) ^ qHash(key.y());
}

GeometryIndex::GeometryIndex(const QSet<Geometry*>& geometries, double cellSize) {
    // save the cell size
    if(cellSize == -1.0) _cellSize = 100; // default cell size
    else _cellSize = cellSize;

    qDebug() << "geometry index / number of geometries" << geometries.size();

    // populate the Geometry set with the given geometries
    for(Geometry* geom : geometries) {
        QPointF topLeft       = geom->getBounds().getTopLeft();
        QPointF bottomRight   = geom->getBounds().getBottomRight();
        QPoint topLeftIdx     = getGridCellAt(topLeft);
        QPoint bottomRightIdx = getGridCellAt(bottomRight);

        qDebug() << "Geometry type" << geom->getGeometryType() << GeometryType::PathGeometryType << GeometryType::PolygonGeometryType << geom->toString()
                 << "\n\ttopLeft" << topLeftIdx << topLeft << "bottomRight" << bottomRightIdx << bottomRight;

        for(int i = topLeftIdx.x(); i <= bottomRightIdx.x(); ++i) {
            for(int j = topLeftIdx.y(); j <= bottomRightIdx.y(); ++j) {
                QPoint cellIdx(i,j);
                if(!_geometryGrid.contains(cellIdx))
                    _geometryGrid.insert(cellIdx, new QSet<Geometry*>());
                _geometryGrid.value(cellIdx)->insert(geom);
            }
        }
    }
}

const QList<Geometry*>& GeometryIndex::getGrid() {
    if(_grid.isEmpty()) {
        for(auto it = _geometryGrid.begin(); it != _geometryGrid.end(); ++it) {
            Geometry* geom = new Cell(it.key().x()*_cellSize, it.key().y()*_cellSize, _cellSize);
            _grid.append(geom);
        }
    }
    return _grid;
}

QSet<Geometry*>* GeometryIndex::getGeometriesAt(double x, double y) {
    QSet<Geometry*>* geometries = new QSet<Geometry*>();

    // get the corresponding grid cell index of the point p
    QPoint cellIdx = getGridCellAt(x,y);

    // get the set of Geometries at the index
    QSet<Geometry*>* geoms = _geometryGrid.value(cellIdx);
//    qDebug() << "point" << x << y << cellIdx << _cellSize << geoms->size() << _geometryGrid.size();
    if(!geoms) {
        return nullptr;
    }

    // loop through the geometries to get the resulting set of geometries that contain the point p
    for(Geometry* geom : *geoms) {
        if(geom->contains(x,y)) {
            geometries->insert(geom);
        }
    }

    return geometries;
}

QSet<Geometry*>* GeometryIndex::getGeometriesWithin(double x, double y, double distance) {
    QSet<Geometry*>* geometries = new QSet<Geometry*>();

    // get the number of cells covered by distance
    int n = qCeil(distance / _cellSize);

    // get the corresponding grid cell index of the point p
    QPoint cellIdx = getGridCellAt(x,y);

    QPoint topLeftIdx     = getGridCellAt(cellIdx - QPoint(n,n));
    QPoint bottomRightIdx = getGridCellAt(cellIdx + QPoint(n,n));
    for(int i = qMax(0,topLeftIdx.x()); i <= bottomRightIdx.x(); ++i) {
        for(int j = qMax(0,topLeftIdx.y()); j <= bottomRightIdx.y(); ++j) {
            QPoint idx(i,j);
            if(_geometryGrid.contains(idx)) {
                geometries->unite(*(_geometryGrid.value(idx)));
            }
        }
    }
    return geometries;
}