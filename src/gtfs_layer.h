//
// Created by Benjamin Baron on 08/02/16.
//

#ifndef LOCALL_GTFS_LOADER_H
#define LOCALL_GTFS_LOADER_H

#include <QObject>
#include <QString>
#include <QPointF>
#include <QMap>

#include "proj_api.h"
#include "trace_layer.h"
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/operation/overlay/snap/GeometrySnapper.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/linearref/LinearLocation.h>
#include <geos/linearref/LocationIndexedLine.h>
#include <geos/linearref/ExtractLineByLocation.h>
#include <geos/io/WKTWriter.h>

class Stop {
public:
    Stop():
            _id(""), _name(""), _coords(QPointF()) {}
    Stop(QString id, QString name, QPointF coords):
            _id(id), _name(name), _coords(coords) {}

    QPointF getCoords() const {return _coords;}
    QPointF getProjCoords() const { return _projCoord; }
    QString getId() const { return _id; }
    void setProjCoords(const QPointF& pt) { _projCoord = pt; }

    QString toString() const {
        return QString("%1 - %2 (%3, %4)").arg(_id).arg(_name).arg(_coords.x()).arg(_coords.y());
    }

private:
    QString _id;
    QString _name;
    QPointF _coords;
    QPointF _projCoord;
};

class WayPoint {
public:
    WayPoint():
            _coords(QPointF()), _departureTime(0), _arrivalTime(0) {}
    WayPoint(QPointF coords):
            _coords(coords), _departureTime(0), _arrivalTime(0) {}
    WayPoint(Stop* stop):
            _coords(stop->getCoords()), _departureTime(0), _arrivalTime(0), _stop(stop) {}
    WayPoint(Stop* stop, long long departureTime, long long arrivalTime):
            _coords(stop->getCoords()), _departureTime(departureTime), _arrivalTime(arrivalTime), _stop(stop) {}
    WayPoint(QPointF coords, long long departureTime, long long arrivalTime, Stop* stop = nullptr):
            _coords(coords), _departureTime(departureTime), _arrivalTime(arrivalTime), _stop(stop) {}
    WayPoint(const WayPoint& other):
            _coords(other._coords),
            _departureTime(other._departureTime),
            _arrivalTime(other._arrivalTime),
            _stop(other._stop) {}

    double x() const {
        return _coords.x();
    }
    double y() const {
        return _coords.y();
    }
    QPointF getCoords() const {return _coords;}
    void setCoords(QPointF p) { _coords = QPointF(p); }
    void setTimes(long long departureTime, long long arrivalTime) {
        _departureTime = departureTime;
        _arrivalTime = arrivalTime;
    }
    long long getDepartureTime() const { return _departureTime; }
    long long getArrivalTime() const { return _arrivalTime; }
    bool isStop() {
        return _stop != nullptr;
    }
    Stop* getStop() { return _stop; }

    QString toString() {
        QString ret = QString("(%1, %2) / %3 -> %4").arg(_coords.x()).arg(_coords.y()).arg(_departureTime).arg(_arrivalTime);
        if(_stop != nullptr)
            ret += QString(" -- (%1)").arg(_stop->toString());
        return ret;
    }

protected:
    QPointF _coords;
    long long _departureTime;
    long long _arrivalTime;
    Stop* _stop = nullptr;
};

class Trip {
public:
    Trip():
            _routeId(""), _serviceId(""), _tripId(""), _tripHeadsign(""), _directionId(""), _shapeId("") {}
    Trip(QString routeId, QString serviceId, QString tripId, QString tripHeadsign, QString directionId, QString shapeId):
            _routeId(routeId),
            _serviceId(serviceId),
            _tripId(tripId),
            _tripHeadsign(tripHeadsign),
            _directionId(directionId),
            _shapeId(shapeId) {}
    Trip(const Trip& t):
            _routeId(t._routeId),
            _serviceId(t._serviceId),
            _tripId(t._tripId),
            _tripHeadsign(t._tripHeadsign),
            _directionId(t._directionId),
            _shapeId(t._shapeId) {}

    QString getRouteId() { return _routeId; }
    QString getServiceId() { return _serviceId; }
    QString getShapeId() { return _shapeId; }
    QString getTripId() { return _tripId; }

protected:
    QString _routeId;
    QString _serviceId;
    QString _tripId;
    QString _tripHeadsign;
    QString _directionId;
    QString _shapeId;

};

class GTFSLayer;

class Trajectory: public Trip {
public:
    friend class GTFSLayer;
    Trajectory():
            Trip() {}
    Trajectory(QString routeId, QString serviceId, QString tripId, QString tripHeadsign, QString directionId, QString shapeId):
            Trip(routeId, serviceId, tripId, tripHeadsign, directionId, shapeId) {}
    Trajectory(const Trip& t):
            Trip(t) {}

    void getTrajectory(QMap<long long, WayPoint*>* trajectory) const {
        *trajectory = _trajectory;
    }

    // returns whether the vehicle is active at timestamp time
    bool isActive(long long time) const {
        return time <= _trajectory.lastKey() && time >= _trajectory.firstKey();
    }
    long long getStartTime() { return _trajectory.firstKey(); }
    long long getEndTime() { return _trajectory.lastKey(); }
    int getNbWaypoints() { return _trajectory.size(); }
    int getNbStops() {
        int count = 0;
        for(auto it = _trajectory.begin(); it != _trajectory.end(); ++it){
            if(it.value()->isStop()) count++;
        }
        return count;
    }
    void addWayPoint(WayPoint* p) {
        _trajectory.insert(p->getArrivalTime(), p);
    }

    QString toString() {
        return QString("%1 / %2 / %3 / %4").arg(_tripId).arg(_serviceId).arg(_routeId).arg(_trajectory.count());
    }

private:
    QMap<long long, WayPoint*> _trajectory;
};


class GTFSTrace : public Trace {
public:
    GTFSTrace(QString filename, bool snapToShape);

    virtual bool openTrace(Loader* loader);
    void getShapes(QMap<QString, QMap<int,QPointF>*>* shapes) {
        *shapes = _shapes;
    }
    void getStops(QMap<QString, Stop*>* stops) {
        *stops = _stopsUsed;
    }
    void getAllStops(QMap<QString, Stop*>* stops) {
        *stops = _stops;
    }
    void getShapesToTrips(QMap<QString, QSet<QString>*>* shapes) {
        *shapes = _shapesToTrips;
    }
    void outputWorldMap(QList<geos::geom::LineString*>* lines, const QString& output);
    void getTrajectoryLineString(const QString& routeId,
                                 geos::geom::LineString*& stops,
                                 const QSet<Stop*>& stopsToDiscard);
    double getAverageNumberOfVehicles(const QString& routeId);
    void getAverageSpeed(const QString& routeId, double* mean, double* stddev);
    void outputLines(QList<geos::geom::LineString*>* lines, const QString& output);

private:
    QString _folderPath;
    QString _stopTimesFilePath;
    QString _stopsFilePath;
    QString _shapesFilePath;
    QString _tripsFilePath;
    QString _routesFilePath;
    bool _snapToShape = true;

    // trajectories indexed by the trip_id of each trajectory
    QMap<QString, Trajectory*> _trajectories;
    QMap<QString, Stop*> _stopsUsed;
    QMap<QString, Stop*> _stops;
    QMap<QString, QMap<int,QPointF>*> _shapes;
    QMap<QString, QSet<QString>*> _shapesToTrips; // shape_id: {traj_id}

    // helper methods
    void parseTrips(Loader* loader);
    long long toSeconds(QString time);
    void pushLinestring(geos::geom::Geometry* geom, std::vector<geos::geom::Geometry*>* vector);
};


class GTFSLayer : public TraceLayer {
    Q_OBJECT
public:
    GTFSLayer(MainWindow* parent = 0, QString name = 0, GTFSTrace* trace = nullptr):
            TraceLayer(parent, name, trace, false) {
        if(parent) {
            addMenuBar();
        }
    }

    virtual QGraphicsItemGroup* draw();
    virtual void addMenuBar();

    QString getInformation() {
        return "GTFS Layer: " + _name;
    }
    bool load(Loader* loader) {
        _trace->openTrace(loader);
        return true;
    }

private slots:
    void exportStops();
    void exportONESettings();

private:
    void toWKTFile(geos::geom::LineString* ls, const QString& output);
};

#endif //LOCALL_GTFS_LOADER_H
