#include "geometry_index.h"

#include "geometries.h"
#include "trace_layer.h"

GeometryIndex::GeometryIndex(QSet<Geometry*>& geometries, double cellSize) {
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

void GeometryIndex::getGeometriesAt(QSet<Geometry*>* geometries, double x, double y) {
    // get the corresponding grid cell index of the point p
    QPoint cellIdx = getGridCellAt(x,y);

    // get the set of Geometries at the index
    QSet<Geometry*>* geoms = _geometryGrid.value(cellIdx);
//    qDebug() << "point" << x << y << cellIdx << _cellSize << geoms->size() << _geometryGrid.size();
    if(!geoms)
        return;

    // loop through the geometries to get the resulting set of geometries that contain the point p
    for(Geometry* geom : *geoms) {
        if(geom->contains(x,y))
        (*geometries).insert(geom);
    }
}

GeometryIndex* GeometryIndex::make_geometryIndex(Trace* trace,
                                                 double sampling, double startTime, double endTime,
                                                 double geometryCellsSize,
                                                 GeometryType geometryType, QString geometryCirclesFile) {
    // build the geometry index
    QSet<Geometry*> geometries;
    // build the cells from the trace layer

    QHash<QString, QMap<long long, QPointF>*> nodes;
    trace->getNodes(&nodes);

    QSet<QPoint> cellGeometries;
    for(auto it = nodes.begin(); it != nodes.end(); ++it) {

        if(it.value()->lastKey() < startTime) {
            continue;
        }

        auto jt = (startTime == -1) ? it.value()->begin() : it.value()->lowerBound(startTime);
        if(jt == it.value()->end()) {
            continue;
        }

        long long prevTimestamp = jt.key(); // previous timestamp
        QPointF prevPos = jt.value();       // previous position
        for(++jt; jt != it.value()->end(); ++jt) {
            // start from the second position
            long long timestamp = jt.key(); // current timestamp
            QPointF pos = jt.value();       // current position

            // number of intermediate positions (with the sampling)
            int nbPos = qMax(1, qCeil((timestamp - prevTimestamp) / sampling));
            for(int i = 1; i <= nbPos; ++i) {
                long long t = prevTimestamp + i*sampling; // get the sampling time
                QPointF p = (timestamp - t)*prevPos + (t - prevTimestamp)*pos;
                p /= (timestamp - prevTimestamp);

                if(endTime != -1 && t > endTime) break;

                QPoint cellIdx(qFloor(p.x() / geometryCellsSize), (int)qFloor(p.y() / geometryCellsSize));
                if(!cellGeometries.contains(cellIdx)) {
                    Geometry* geom = new Cell(cellIdx.x()*geometryCellsSize, cellIdx.y()*geometryCellsSize, geometryCellsSize);
                    cellGeometries.insert(cellIdx);
                    geometries.insert(geom);
                }

                prevPos = pos;
                prevTimestamp = timestamp;
            }
        }
    }

    // add the circles from the given file
    if(geometryType == CircleType) {
        // build the circles from the given file
        QFile* file = new QFile(geometryCirclesFile);
        if(!file->open(QFile::ReadOnly | QFile::Text))
            return 0;
        while(!file->atEnd()) {
            // line format: "x;y;radius"
            QString line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
            QStringList fields = line.split(";");
            double x = fields.at(0).toDouble();
            double y = fields.at(1).toDouble();
            double radius = fields.at(2).toDouble();
            Geometry* geom = new Circle(x,y,radius);
            geometries.insert(geom);
        }
    }

    for(Geometry* geom : geometries) {
        if(geom->getGeometryType() == CircleType) {
            qDebug() << geom->getCenter();
        }
    }

    GeometryIndex* geometryIndex = new GeometryIndex(geometries, geometryCellsSize);
    return geometryIndex;
}
