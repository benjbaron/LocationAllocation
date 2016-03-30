#ifndef SPATIALSTATS_H
#define SPATIALSTATS_H

#include "utils.h"
#include "layer.h"
#include "geometries.h"
#include "compute_allocation.h"
#include "trace_layer.h"
#include "weighted_allocation_layer.h"
#include "geometry_index.h"
#include "rest_server.h"


class TraceLayer;

class MobileNode {
public:
    MobileNode(QString id = "", int sampling = -1, SpatialStats* spatialStats = 0):
        _id(id), _sampling(sampling), _spatialStats(spatialStats) { }

    void addPosition(long long time, double x, double y);
    QString getId() { return _id; }

    QMap<long long, QHash<Geometry*,long long>*> getGeometries() { return _visitedGeometries; }

    QMap<long long, QHash<Geometry*,long long>*> getCells(long long start, long long end) {
        auto up = _visitedGeometries.lowerBound(start);
        auto it = up;
        if(up.key() != start)
            it = up-1;
        if(up == _visitedGeometries.end())
            return QMap<long long, QHash<Geometry*,long long>*>(); // reached the end

        QMap<long long, QHash<Geometry*,long long>*> res;
        for(; it != _visitedGeometries.end() && it.key() <= end; ++it) {
            res.insert(it.key(), it.value());
        }

        return res;
    }

private:
    QString _id;
    int _sampling;                  // linear interpolation at different times
    QSet<Geometry*> _prevGeometries;
    QHash<Geometry*, long long> _startTimeGeometries;   // record the start time of each geometry (reverse hash)
    long long _prevTime = 0;        // node previous time of the point recording
    QPointF _prevPos = QPointF();   // node previous position
    SpatialStats* _spatialStats;

    // start time, < Geometry id, end time >
    QMap<long long, QHash<Geometry*,long long>*> _visitedGeometries;
};


struct GeometryValue {
    Geometry* cell;
    GeometryValue(Geometry* c) { cell = c; }
    Distribution interVisitDurationDist;
    QList<long long> visitFrequency; // timestamp of the begining of the visit
    QMultiMap<long long, long long> visits; // <start, end>
    QSet<QString> nodes; // nodes that visited the cell
    Distribution travelTimes;
    int connections = 0;
    qreal localStat;
    QColor color;
    qreal medIncomingScore = 0.0; // sum of the score of the incoming edges (with median)
    qreal avgIncomingScore = 0.0; // sum of the score of the incoming edges (with average)
    qreal medScore = 0.0; // score with median of the inter-visit distribution
    qreal avgScore = 0.0; // score with average of the inter-visit distribution
};


struct GeometryMatrixValue {
    GeometryMatrixValue(Geometry* c1, Geometry* c2) { cell1 = c1; cell2 = c2; }
    Geometry* cell1;
    Geometry* cell2;
    Distribution travelTimeDist;
    Distribution interVisitDurationDist;
    QList<long long> visitFrequency; // timestamp of the begining of the visit
    QMultiMap<long long, long long> visits; // <start, end>
    QSet<QString> nodes; // nodes that visited the link
    qreal medScore = 0.0; // score with median of the inter-visit distribution
    qreal avgScore = 0.0; // score with average of the inter-visit distribution
};


class SpatialStats : public QObject {
    Q_OBJECT
public:
    SpatialStats(Trace* trace = nullptr,
                 long long sampling = -1,
                 long long startTime = -1,
                 long long endTime = -1,
                 GeometryIndex* geometryIndex = 0);

    /* Populate nodes from the trace layer */
    void populateMobileNodes(Loader* loader);

    /* Compute spatial statistics */
    bool computeStats(Loader* loader);

    void getValue(GeometryValue** val, Geometry* geom) {
        if(_geometries.contains(geom))
            *val = _geometries.value(geom);
        else
            *val = nullptr;
    }

    void getValue(GeometryMatrixValue** val, Geometry* geom1, Geometry* geom2) {
        if(_geometryMatrix.contains(geom1)
           && _geometryMatrix.value(geom1)->contains(geom2))
            *val = _geometryMatrix.value(geom1)->value(geom2);
        else
            *val = nullptr;
    }

    bool hasValue(Geometry* geom) {
        return _geometries.contains(geom);
    }

    bool hasMatrixValue(Geometry* geom) {
        return _geometryMatrix.contains(geom);
    }

    bool hasMatrixValue(Geometry* geom1, Geometry* geom2) {
        return _geometryMatrix.contains(geom1)
               && _geometryMatrix.value(geom1)->contains(geom2);
    }

    void getValues(QHash<Geometry*, GeometryMatrixValue*>** geometries, Geometry* geom) {
        if(_geometryMatrix.contains(geom))
            *geometries = _geometryMatrix.value(geom);
        else
            *geometries = nullptr;
    }

    void getGeometryMatrix(QHash<Geometry*, QHash<Geometry*, GeometryMatrixValue*>* >* geometryMatrix) {
        *geometryMatrix = _geometryMatrix;
    }

    void getGeometries(QHash<Geometry*, GeometryValue*>* geometries) {
        *geometries = _geometries;
    }

    double getAverageSpeed() {
        return _trace->averageSpeed();
    }

    /* Getis Ord G spatial stats
     * TODO: compute the p-value
    */
    qreal computeLocalStat(Geometry* geom_i);

    /* Returns the Geometry that contains the point (x,y) */
    void containsPoint(QSet<Geometry*>* geometries, double x, double y) {
        _geometryIndex->getGeometriesAt(geometries, x,y);
    }
    void containsPoint(QSet<Geometry*>* geometries, QPointF p) {
        containsPoint(geometries, p.x(), p.y());
    }

    void getGeometriesAt(QSet<Geometry*>* geometries, double x, double y) const {
        _geometryIndex->getGeometriesAt(geometries,x,y);
    }

private:
    Trace* _trace;
    QHash<QString, MobileNode*> _mobileNodes; // <mobileNodeId, mobileNode>
    QHash<Geometry*, QHash<Geometry*, GeometryMatrixValue*>* > _geometryMatrix;
    QMutex _geometryMatrixMutex;
    QHash<Geometry*, GeometryValue*> _geometries;
    QMutex _geometriesMutex;
    GeometryIndex* _geometryIndex;

    long long _sampling = 1; // each 1 second
    long long _startTime;
    long long _endTime;

    QColor selectColorForLocalStat(qreal zScore);
    void computeVisitMatrix(QString& node);
    void computeInterVisits(Geometry* geom);
    void computeInterVisitsMatrix(Geometry* geom1);

};

#endif // SPATIALSTATS_H
