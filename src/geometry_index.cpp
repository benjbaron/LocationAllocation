#include "geometry_index.h"

#include "geometries.h"

GeometryIndex::GeometryIndex(QSet<Geometry*>& geometries, double cellSize)
{
    // save the cell size
    if(cellSize == -1.0) _cellSize = 100; // default cell size
    else _cellSize = cellSize;

    // populate the geometry set with the given geometries
    for(Geometry* geom : geometries) {
        QPointF topLeft = geom->getBounds().getTopLeft();
        QPointF bottomRight = geom->getBounds().getBottomRight();
        QPoint topLeftIdx     = getGridCellAt(topLeft);
        QPoint bottomRightIdx = getGridCellAt(bottomRight);
        for(int i = topLeftIdx.x(); i < bottomRightIdx.x(); ++i) {
            for(int j = topLeftIdx.y(); j < bottomRightIdx.y(); ++j) {
                QPoint cellIdx(i,j);
                if(!_geometryGrid.contains(cellIdx))
                    _geometryGrid.insert(cellIdx, new QSet<Geometry*>());
                _geometryGrid.value(cellIdx)->insert(geom);
            }
        }
    }
}

QSet<Geometry*> GeometryIndex::getGeometriesAt(double x, double y)
{
    // get the corresponding grid cell index of the point p
    QPoint cellIdx = getGridCellAt(x,y);
    // get the set of Geometries at the index
    QSet<Geometry*>* geoms = _geometryGrid.value(cellIdx);
//    qDebug() << "point" << x << y << cellIdx << _cellSize << geoms->size() << _geometryGrid.size();
    if(!geoms) return QSet<Geometry*>();

    // loop through the geometries to get the resulting set of geometries that contain the point p
    QSet<Geometry*> resultingGeometries;
    for(Geometry* geom : *geoms) {
        if(geom->contains(x,y))
            resultingGeometries.insert(geom);
    }
    return resultingGeometries;
}

