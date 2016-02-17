#ifndef SPATIALSTATS_H
#define SPATIALSTATS_H

#include "dockwidget_plots.h"
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
    int _sampling;                  // linear interoplation at different times
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


class SpatialStats: public Layer
{
    Q_OBJECT
public:
    SpatialStats(MainWindow* parent = 0, QString name = 0, TraceLayer* traceLayer = 0,
                 long long sampling = -1, long long startTime = -1, long long endTime = -1,
                 GeometryIndex* geometryIndex = 0);

    /* Populate nodes from the trace layer */
    void populateMobileNodes(Loader* loader);
    /* Compute spatial statistics */
    void computeStats(Loader* loader);

    GeometryValue* getValue(Geometry* geom) {
        if(_geometries.contains(geom))
            return _geometries.value(geom);
        return NULL;
    }
    GeometryMatrixValue* getValue(Geometry* geom1, Geometry* geom2) {
        if(_geometryMatrix.contains(geom1) && _geometryMatrix.value(geom1)->contains(geom2))
            return _geometryMatrix.value(geom1)->value(geom2);
        return NULL;
    }
    QHash<Geometry*, GeometryValue*>* getGeometries() { return &_geometries; }
    QColor selectColorForLocalStat(qreal zScore);
    double getAverageSpeed() { return _traceLayer->getAverageSpeed(); }

    /* Getis Ord G spatial stats
     * TODO: compute the p-value
    */
    qreal computeLocalStat(Geometry* geom_i);
    void addMenuBar();


    /* Draw the group of cells */
    QGraphicsItemGroup* draw();

    virtual bool load(Loader* loader);

    /* Returns the Geometry that contains the point (x,y) */
    QSet<Geometry*> containsPoint(double x, double y) {
        return _geometryIndex->getGeometriesAt(x,y);
    }
    QSet<Geometry*> containsPoint(QPointF p) {
        return containsPoint(p.x(), p.y());
    }
    ComputeAllocation* getComputeAllocation() { return _computeAllocation; }

private slots:
    void exportContourFile();

private:
    TraceLayer* _traceLayer;
    QHash<QString, MobileNode*> _mobileNodes;
    QHash<Geometry*, QHash<Geometry*, GeometryMatrixValue*>* > _geometryMatrix;
    QHash<Geometry*, GeometryValue*> _geometries;
    QHash<Geometry*, GeometryGraphics*> _geometryGraphics;
    long long _sampling = 1; // each 1 second
    long long _startTime;
    long long _endTime;
    Geometry* _selectedGeometry = NULL;
    DockWidgetPlots* _plots = 0;
    GeometryIndex* _geometryIndex;
    ComputeAllocation* _computeAllocation;
    RESTServer* _restServer = 0;
};

#endif // SPATIALSTATS_H
