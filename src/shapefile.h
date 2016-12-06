//
// Created by Benjamin Baron on 26/10/2016.
//

#ifndef LOCALL_SHAPEFILE_H
#define LOCALL_SHAPEFILE_H


#include <ogr_geometry.h>
#include "loader.h"
#include "geometry_index.h"

// forward class declarations
class Stop;

struct ProjectedPoint {
    ProjectedPoint(QPointF o, QPointF p):
            originalPoint(o), projectedPoint(p) { }
    QPointF originalPoint;
    QPointF projectedPoint;
    QSet<int> ls; // set of shapefile ids
    int projectedId; // id of the resulting linestring (that includes the projected point)
    OGRLineString* projectedLs;
    QSet<QPoint> cells;
    Stop* stop;
};

struct ConnectedComponent {
    ConnectedComponent(int id = -1): id(id) {}
    int id;
    QSet<QPointF> points;
    QSet<int> geomIds;
    void addPoint(const QPointF& pt) { points.insert(pt); }
    void addGeomId(int id) { geomIds.insert(id); }
    bool containsPoint(const QPointF& pt) const {
        return points.contains(pt);
    }
    void unite(const ConnectedComponent& cc) {
        points.unite(cc.points);
        geomIds.unite(cc.geomIds);
    }
    int nbPoints() { return points.size(); }
};


struct ShapefileIndexes {
    ShapefileIndexes(int join = -1, int desc = -1,
                     int x1 = -1, int y1 = -1,
                     int x2 = -1, int y2 = -1) {
        this->join = join;
        this->description = desc;
        this->x1 = x1;
        this->y1 = y1;
        this->x2 = x2;
        this->y2 = y2;
    }

    void clear() {
        this->join = -1;
        this->description = -1;
        this->x1 = -1;
        this->y1 = -1;
        this->x2 = -1;
        this->y2 = -1;
    }

    int join;
    int description;
    int x1;
    int y1;
    int x2;
    int y2;
};

struct ShapefileFeature {
    ShapefileFeature(int id = -1,
                     QList<QString>* attributes = nullptr,
                     OGRGeometry* ogrGeometry = nullptr) {
        this->id = id;
        this->attributes = attributes;
        this->ogrGeometry = ogrGeometry;
        this->geometry = OGRGeometryToGeometry(ogrGeometry);
    }
    QString toString() {
        return QString::number(id);
    }
    int id;
    QList<QString>* attributes;
    OGRGeometry* ogrGeometry = nullptr;
    Geometry* geometry = nullptr;
};

class Shapefile {
public:
    Shapefile(const QString& filename = "", ShapefileIndexes* indexes = nullptr):
            _filename(filename), _indexes(indexes) { }

    virtual bool open(Loader* loader);

    void addGeometry(OGRGeometry* geom, QList<QString>* list = nullptr) {
        int idx = _features.size();
        ShapefileFeature* feature = new ShapefileFeature(idx, list, geom);
        _features.insert(idx, feature);
        if(_indexes != nullptr && !list->isEmpty()) {
            _joinIdxToFeatureMap.insert(list->at(_indexes->join), idx);
        }
    }
    int nbFeatures() { return _features.size(); }
    QHash<int,ShapefileFeature*>* getFeatures() {
        return &_features;
    }
    OGRGeometry* getGeometry(int idx) {
        return _features.value(idx)->ogrGeometry;
    }
    ShapefileFeature* getFeature(int idx) {
        return _features.value(idx);
    }
    ShapefileFeature* getFeature(QString joinIdx) {
        return _features.value(_joinIdxToFeatureMap.value(joinIdx));
    }
    ShapefileIndexes* indexes() {
        return _indexes;
    }

    QSet<QPointF> getIntersections(double maxAngle = 10);
    bool projectPoints(Loader* loader, QHash<QPointF,ProjectedPoint*>* points);
    bool exportWKT(Loader* loader, QString output);
    bool removeNotConnectedComponents(Loader* loader);
    QList<QString>* getAttributeHeader() {
        return &_attributes;
    }

    GeometryIndex* makeGeometryIndex(int cellSize = 100);

private:
    const QString _filename;
    QHash<int, ShapefileFeature*> _features;
    QHash<QString, int> _joinIdxToFeatureMap;
    QList<QString> _attributes;
    ShapefileIndexes* _indexes;

    /* Loader functions */
    bool loadShapefile(Loader* loader);
    bool loadWKT(Loader* loader);
    bool loadCSV(Loader* loader);

    double getAngleAtIntersection(OGRLineString *ls1, OGRLineString *ls2, OGRPoint *pt);
    bool getSubLineContainingPoint(OGRLineString *ls, OGRPoint *pt, OGRPoint *ptBefore, OGRPoint *ptAfter);
    bool isOnLine(OGRPoint* a, OGRPoint* b, OGRPoint* c);
};


class LineStringIndex {
public:
    LineStringIndex(Shapefile* shapefile, double cellSize = 100);
    bool projectOnClosestLineString(double x, double y, double distance,
                                    int& id, OGRLineString** modified,
                                    QPointF* projectedPoint, ProjectedPoint* projPt);

private:
    double _cellSize;
    Shapefile* _shapefile;
    QHash<QPoint,QSet<int>*> _lineStringGrid; // ids of the LineStrings
    QPoint getGridCellAt(double x, double y) {
        return QPoint(qFloor(x / _cellSize), qFloor(y / _cellSize));
    }
    QPoint getGridCellAt(QPointF p) {
        return getGridCellAt(p.x(), p.y());
    }

    Bounds getBoundingBox(OGRGeometry* geom);
    void getGridCellsWithinDistance(Bounds b, QSet<QPoint>* points);
};

#endif //LOCALL_SHAPEFILE_H
