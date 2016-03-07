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

//using namespace geos;
//using namespace geos::geom;
//using namespace geos::linearref;

class Stop
{
public:
    Stop():
            _id(""), _name(""), _coords(QPointF()) {}
    Stop(QString id, QString name, QPointF coords):
            _id(id), _name(name), _coords(coords) {}

    QPointF getCoords() const {return _coords;}
    void setCoords(double x, double y) { _coords = QPointF(x,y); }
    void setCoords(QPointF p) { _coords = QPointF(p); }

    QString toString() const {
        return QString("%1 - %2 (%3, %4)").arg(_id).arg(_name).arg(_coords.x()).arg(_coords.y());
    }

private:
    QString _id;
    QString _name;
    QPointF _coords;
};

class WayPoint
{
public:
    WayPoint():
            _coords(QPointF()), _departureTime(0), _arrivalTime(0) {}
    WayPoint(QPointF coords):
            _coords(coords), _departureTime(0), _arrivalTime(0) {}
    WayPoint(Stop * stop):
            _coords(stop->getCoords()), _departureTime(0), _arrivalTime(0), _stop(stop) {}
    WayPoint(Stop * stop, long long departureTime, long long arrivalTime):
            _coords(stop->getCoords()), _departureTime(departureTime), _arrivalTime(arrivalTime), _stop(stop) {}
    WayPoint(QPointF coords, long long departureTime, long long arrivalTime, Stop * stop = NULL):
            _coords(coords), _departureTime(departureTime), _arrivalTime(arrivalTime), _stop(stop) {}

    QPointF getCoords() const {return _coords;}
    void setCoords(double x, double y) { _coords = QPointF(x,y); }
    void setCoords(QPointF p) { _coords = QPointF(p); }
    void setTimes(long long departureTime, long long arrivalTime) { _departureTime = departureTime; _arrivalTime = arrivalTime; }
    long long getDepartureTime() const { return _departureTime; }
    long long getArrivalTime() const { return _arrivalTime; }

protected:
    QPointF _coords;
    long long _departureTime;
    long long _arrivalTime;
    Stop * _stop;
};

class Trip
{
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
    Trip(Trip &t):
            _routeId(t._routeId),
            _serviceId(t._serviceId),
            _tripId(t._tripId),
            _tripHeadsign(t._tripHeadsign),
            _directionId(t._directionId),
            _shapeId(t._shapeId) {}

    QString getRouteId() const { return _routeId; }
    QString getServiceId() const { return _serviceId; }
    QString getShapeId() const { return _shapeId; }

protected:
    QString _routeId;
    QString _serviceId;
    QString _tripId;
    QString _tripHeadsign;
    QString _directionId;
    QString _shapeId;

};

class GTFSLayer;

class Trajectory: public Trip
{
public:
    Trajectory():
            Trip() {}

    Trajectory(QString routeId, QString serviceId, QString tripId, QString tripHeadsign, QString directionId, QString shapeId):
            Trip(routeId, serviceId, tripId, tripHeadsign, directionId, shapeId),
            _trajectory(QMap<long long, WayPoint*>()) {}
    Trajectory(Trip t):
            Trip(t),
            _trajectory(QMap<long long, WayPoint*>()) {}

    QMap<long long, WayPoint*> getTrajectory() const { return _trajectory; }

    // returns whether the vehicle is active at timestamp time
    bool isActive(long long time) const {
        return time <= _trajectory.lastKey() && time >= _trajectory.firstKey();
    }

    void addWayPoint(WayPoint * p) {
        _trajectory.insert(p->getArrivalTime(), p);
    }

    QString toString() {
        QString ret;
        return QString("%1 / %2 / %3 / %4").arg(_tripId).arg(_serviceId).arg(_routeId).arg(_trajectory.count());
    }

    friend class GTFSLayer;

private:
    QMap<long long, WayPoint *> _trajectory;
};


class GTFSTrace : public Trace {
public:
    GTFSTrace(QString filename, bool snapToShape);

    virtual bool openTrace(Loader* loader);
    QMap<QString, QMap<int,QPointF>*>* getShapes() { return &_shapes; }
    QMap<QString,Stop*>* getStops() { return &_stops; }

private:
    QString _folderPath;
    QString _stopTimesFilePath;
    QString _stopsFilePath;
    QString _shapesFilePath;
    QString _tripsFilePath;
    bool _snapToShape = true;

    // trajectories indexed by the trip id of each trajectory
    QMap<QString,Trajectory *> _trajectories;
    QMap<QString,Stop*> _stops;
    QMap<QString, QMap<int,QPointF>*> _shapes;

    // helper methods
    void parseTrips();
    long long toSeconds(QString time);

};

class GTFSLayer : public TraceLayer
{
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

    QString getInformation() { return "GTFS Layer: " + _name; }
    bool load(Loader* loader) {
        _trace->openTrace(loader);
        return true;
    }

private slots:
    void exportStops();
};

#endif //LOCALL_GTFS_LOADER_H
