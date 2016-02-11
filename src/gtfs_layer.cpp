//
// Created by Benjamin Baron on 08/02/16.
//

#include <QFileInfo>
#include <QDebug>
#include "loader.h"

#include "gtfs_layer.h"
#include "projfactory.h"
#include "csv_parser.h"
#include "PointLayer.h"
#include "linestring_layer.h"
#include "NumberDialog.h"


// get the extension of the files (either .csv or .txt)
QString checkFile(QString foldername, QStringList exts, QString filename) {
    foreach(auto ext, exts) {
        QFileInfo check(foldername+"/"+filename+"."+ext);
        if(check.exists() && check.isFile())
            return check.absoluteFilePath();
    }
    return QString();
}


GTFSLoader::GTFSLoader(MainWindow* parent, QString name, QString filename, bool snapToShape):
        TraceLayer(parent, name, filename), _snapToShape(snapToShape)
{
    addBarMenuItems();

    QStringList exts;
    exts << "txt" << "csv";
    _stopTimesFilePath = checkFile(filename,exts,"stop_times");
    _stopsFilePath = checkFile(filename,exts,"stops");
    _shapesFilePath = checkFile(filename,exts,"shapes");
    _tripsFilePath = checkFile(filename,exts,"trips");

    if(_stopTimesFilePath.isEmpty() || _stopsFilePath.isEmpty() || _tripsFilePath.isEmpty()) {
        qWarning() << "One file is missing from the directory";
    }

    _trajectories = QMap<QString, Trajectory *>();

    if(snapToShape && !_shapesFilePath.isEmpty())
        _snapToShape = true;
    else
        _snapToShape = false;
}

GTFSLoader::GTFSLoader(const GTFSLoader &other) :
        _folderPath(other._folderPath),
        _stopTimesFilePath(other._stopTimesFilePath),
        _stopsFilePath(other._stopsFilePath),
        _shapesFilePath(other._shapesFilePath),
        _tripsFilePath(other._tripsFilePath)
{
    _trajectories = QMap<QString, Trajectory *>();
}


GTFSLoader::~GTFSLoader()
{
}

void GTFSLoader::parseTrips()
{
    QVector<QMap<QString, QString>> stopList  = QVector<QMap<QString, QString>>();
    QVector<QMap<QString, QString>> timesList = QVector<QMap<QString, QString>>();
    QVector<QMap<QString, QString>> tripsList = QVector<QMap<QString, QString>>();
    QVector<QMap<QString, QString>> shapesList = QVector<QMap<QString, QString>>();

    CSVParser::parseCSV(_stopsFilePath, stopList, ",");
    CSVParser::parseCSV(_stopTimesFilePath, timesList, ",");
    CSVParser::parseCSV(_tripsFilePath, tripsList, ",");
    CSVParser::parseCSV(_shapesFilePath, shapesList, ",");

    // TODO: case if the folder does not contain a shapes.txt file

    qDebug() << stopList.count() << " / " << timesList.count() << " / " << tripsList.count() << " / " << shapesList.count();

    // get all the trips (trajectories)
    QMap<QString,Trip*> tripsMap = QMap<QString,Trip*>();
    foreach (auto trip, tripsList) {
        QString routeId = trip.value("route_id");
        QString serviceId = trip.value("service_id");
        QString tripId = trip.value("trip_id");
        QString tripHeadsign = trip.value("trip_headsign");
        QString directionId = trip.value("direction_id");
        QString shapeId = trip.value("shape_id");
        tripsMap.insert(tripId,
                        new Trip(routeId, serviceId, tripId, tripHeadsign, directionId, shapeId));
    }

    qDebug() << "trajectories count: " << tripsMap.count();

    // get all the stops
    _stops = QMap<QString, Stop*>();
    foreach (auto stop, stopList) {
        QString stopId = stop.value("stop_id");
        QString stopName = stop.value("stop_name");
        double lat = stop.value("stop_lat").toDouble();
        double lon = stop.value("stop_lon").toDouble();
        double x, y;
        ProjFactory::getInstance().transformCoordinates(lat, lon, &x, &y);
        QPointF coord(x, y);

        if(x < 0 || y < 0) continue;

        _stops.insert(stopId,
                   new Stop(stopId, stopName, coord));
    }

    qDebug() << "_stops " << _stops.count();

    // get all the shapes
    _shapes = QMap<QString, QMap<int,QPointF>* >();
    QMap<QString, geos::geom::LineString*> geomShapes;
    if(_snapToShape) {
        geos::geom::GeometryFactory *global_factory = new geos::geom::GeometryFactory();
        foreach(auto waypoint, shapesList) {
            QString shapeId = waypoint.value("shape_id");
            double lat = waypoint.value("shape_pt_lat").toDouble();
            double lon = waypoint.value("shape_pt_lon").toDouble();
            int seq = waypoint.value("shape_pt_sequence").toInt();
            double x, y;
            ProjFactory::getInstance().transformCoordinates(lat, lon, &x, &y);
            QPointF coord(x, y);

            if(!_shapes.contains(shapeId)) {
                // instantiate a new point sequence
                _shapes.insert(shapeId, new QMap<int,QPointF>());
            }
                _shapes.value(shapeId)->insert(seq, coord);
        }

        qDebug() << "shapesMap" << _shapes.count();


        // create the map of shape linestring
        for(auto it = _shapes.begin(); it != _shapes.end(); it++) {
            geos::geom::CoordinateSequence* cl = new geos::geom::CoordinateArraySequence();
            for(auto sIt = it.value()->begin(); sIt != it.value()->end(); ++sIt) {
                cl->add(geos::geom::Coordinate(sIt.value().x(), sIt.value().y()));
            }
            geos::geom::LineString *ls = global_factory->createLineString(cl);
            geomShapes.insert(it.key(), ls);
        }

        qDebug() << "shapes" << geomShapes.count();
    }

    // initialize all the trajectories
    QMap<QString, QMap<int, WayPoint*>* > trajectories = QMap<QString, QMap<int, WayPoint*>* >();
    foreach(auto time, timesList) {
        QString tripId = time.value("trip_id");
        QString stopId = time.value("stop_id");
        long long arrivalTime = toSeconds(time.value("arrival_time"));
        long long departureTime = toSeconds(time.value("departure_time"));
        int seq = time.value("stop_sequence").toInt();
        Stop* stop = _stops.value(stopId);
        WayPoint* wp = new WayPoint(stop, arrivalTime, departureTime);

        if(!trajectories.contains(tripId)){
            // Add the trajectory to the trajectories map
            trajectories.insert(tripId,
                                new QMap<int, WayPoint*>());
        }

        trajectories.value(tripId)->insert(seq, wp);
    }

    qDebug() << "trajectories" << trajectories.count();

    if(_snapToShape) {
        for(auto it = trajectories.begin(); it != trajectories.end(); ++it) {
            QString tripId = it.key();
            Trip * trip = tripsMap.value(tripId);
            QString shapeId = trip->getShapeId();

            // convert the corresponding shape into a LineString
            if(!geomShapes.contains(shapeId))
                continue;

            // Add the trajectory to the map
            _trajectories.insert(tripId,
                                 new Trajectory(*trip));

            geos::geom::LineString* ls = geomShapes.value(shapeId);
            geos::linearref::LocationIndexedLine * lineRef = new geos::linearref::LocationIndexedLine(ls);

            // get the first waypoint
            auto tIt = it.value()->begin();
            WayPoint* wp1 = tIt.value();
            geos::geom::Coordinate pt1(wp1->getCoords().x(), wp1->getCoords().y());
            geos::linearref::LinearLocation loc1 = lineRef->project(pt1);
            long long startTime = wp1->getDepartureTime();
            geos::geom::Coordinate projWp1 = loc1.getCoordinate(ls);

            tIt++; // go onto the second waypoint
            // snap each waypoint of the trajectory to the shape linestring
            for(; tIt != it.value()->end(); ++tIt) {
                WayPoint* wp2 = tIt.value();
                geos::geom::Coordinate pt2(wp2->getCoords().x(), wp2->getCoords().y());
                geos::linearref::LinearLocation loc2 = lineRef->project(pt2);
                long long endTime   = wp2->getArrivalTime();
                geos::geom::Coordinate projWp2 = loc2.getCoordinate(ls);

                // change the coordinates of the stop to match those of the projected point on the linestring
                wp1->setCoords(QPointF(projWp1.x, projWp1.y));
                wp2->setCoords(QPointF(projWp2.x, projWp2.y));

                // add the way point to the corresponding trajectory
                _trajectories.value(tripId)->addWayPoint(wp1);
                _trajectories.value(tripId)->addWayPoint(wp2);

                // Get the partial time
                geos::geom::LineString* partialLine = dynamic_cast<geos::geom::LineString*>(
                        lineRef->extractLine(loc1, loc2));
                double totalLength = partialLine->getLength();
                double sumLength = 0;

                // Insert the points in the linestring between loc1 and loc2
                // Interpolate the time of each waypoint
                // ignore the two extremity waypoints
                geos::geom::Coordinate prevPt = partialLine->getCoordinateN(0);
                geos::geom::Coordinate curPt;
                for(int ptIdx = 1; ptIdx < partialLine->getNumPoints()-1; ++ptIdx) {
                    curPt = partialLine->getCoordinateN(ptIdx);
                    double distance = prevPt.distance(curPt);
                    long long time = startTime + (endTime - startTime) * (sumLength/totalLength);
                    // add the new waypoint to the trajectory
                    WayPoint* wp = new WayPoint(QPointF(curPt.x, curPt.y), time, time);
                    _trajectories.value(tripId)->addWayPoint(wp);

                    // increment the distance and previous point
                    sumLength += distance;
                    prevPt = curPt;
                }

                // update the waypoints
                wp1 = wp2;
                pt1 = pt2;
                loc1 = loc2;
                startTime = endTime;
                projWp1 = projWp2;
            }
        }

        qDebug() << "trajectories " << _trajectories.count();
    }
}

long long GTFSLoader::toSeconds(QString time)
{
    long hours = time.mid(0, 2).toLong();
    long minutes = time.mid(3, 2).toLong();
    long seconds = time.mid(6, 2).toLong();
    return hours * 3600 + minutes * 60 + seconds;
}

bool GTFSLoader::load(Loader* loader)
{
    qDebug() << "Begin parsing trips";
    loader->changeText("Parsing the trips");
    parseTrips();

    emit loader->loadProgressChanged((qreal)0.1);
    qDebug() << "Trips parsed " << _trajectories.count();
    qDebug() << "Adding trajectories";

    loader->changeText("Add trajectories");

    int id = 0;
    int count = _trajectories.size();
    for(auto trajId: _trajectories.keys()) {
        Trajectory *traj = _trajectories.value(trajId);
        for (auto mvt: traj->getTrajectory().keys()) {
            WayPoint *stop = traj->getTrajectory().value(mvt);
            addPoint(QString::number(id), mvt,
                     stop->getCoords().x(), stop->getCoords().y());
        }
        id++;
        delete traj;
        emit loader->loadProgressChanged(0.1 + 0.9*((qreal) id / (qreal) count));
//        qDebug() << "Trajectories added " << id;
//        if(id > 1000) break;
    }

    qDebug() << "[DONE] Added" << _nodes.size() << "nodes";

    emit loader->loadProgressChanged((qreal)1.0);
    return 1;
}

QGraphicsItemGroup *GTFSLoader::draw() {
    // show the bus lines
    qDebug() << _nodes.size() << "nodes";
    return TraceLayer::draw();
}

void GTFSLoader::addBarMenuItems() {
    _menu->addSeparator();
    QAction* actionShowStops   = _menu->addAction("Show Stops");
    QAction* actionShowLines   = _menu->addAction("Show Lines");
    QAction* actionExportStops = _menu->addAction("Export Stops");

    connect(actionShowStops, &QAction::triggered, [=](bool checked){
        qDebug() << "Show stops of " << getName();
        QString name = "GTFS Stops";
        QList<QPointF> stops;
        for(auto it = _stops.begin(); it != _stops.end(); ++it) {
            stops.append(it.value()->getCoords());
        }
        qDebug() << stops.size() << "stops";
        PointLayer* layer   = new PointLayer(_parent, name, stops);
        Loader loader(layer);
        _parent->createLayer(name, layer, &loader);
    });

    connect(actionShowLines, &QAction::triggered, [=](bool checked){
        qDebug() << "Show lines of " << getName();
        QString name = "GTFS Lines";
        QList<QList<QPointF>*> lines;
        for(auto it = _shapes.begin(); it != _shapes.end(); ++it) {
            QList<QPointF>* line = new QList<QPointF>();
            for(auto jt = it.value()->begin(); jt != it.value()->end(); ++jt) {
                line->append(jt.value());
            }
            lines.append(line);
        }
        LineStringLayer* layer   = new LineStringLayer(_parent, name, lines);
        Loader loader(layer);
        _parent->createLayer(name, layer, &loader);
    });

    connect(actionExportStops, &QAction::triggered, this, &GTFSLoader::exportStops);
}

void GTFSLoader::exportStops() {
    QString filename = QFileDialog::getSaveFileName(0,
                                                    tr("Save the stops"),
                                                    QString(),
                                                    tr("CSV file (*.csv)"));

    if(filename.isEmpty())
        return;

    // choose radius
    NumberDialog numDiag(_parent, "Radius");
    int ret = numDiag.exec(); // synchronous
    if (ret == QDialog::Rejected) {
        return;
    }
    int radius = numDiag.getNumber();

    qDebug() << "Exporting" << _stops.size() << "stops in" << filename;
    QFile file(filename);
    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "Unable to write in file "<< filename;
        return;
    }

    QTextStream out(&file);
    for(const Stop* s : _stops.values()) {
        out << QString::number(s->getCoords().x(), 'f', 4) << ";"
        << QString::number(s->getCoords().y(), 'f', 4) << ";"
        << QString::number(radius) << "\n";
    }
    file.close();

    qDebug() << "[DONE] export stops points";
}
