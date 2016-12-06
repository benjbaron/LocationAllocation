//
// Created by Benjamin Baron on 17/05/16.
//

#include "gtfs_trace.h"
#include "csv_parser.h"
#include "loader.h"
#include "proj_factory.h"

// get the extension of the files (either .csv or .txt)
QString checkFile(QString foldername, QStringList exts, QString filename) {
    foreach(auto ext, exts) {
        QFileInfo check(foldername+"/"+filename+"."+ext);
        if(check.exists() && check.isFile())
            return check.absoluteFilePath();
    }
    return QString();
}


GTFSTrace::GTFSTrace(QString filename, bool snapToShape):
        Trace(filename), _snapToShape(snapToShape) {

    QStringList exts;
    exts << "txt" << "csv";
    _stopTimesFilePath = checkFile(filename,exts,"stop_times");
    _stopsFilePath     = checkFile(filename,exts,"stops");
    _shapesFilePath    = checkFile(filename,exts,"shapes");
    _tripsFilePath     = checkFile(filename,exts,"trips");
    _routesFilePath    = checkFile(filename,exts,"routes");

    if(_stopTimesFilePath.isEmpty() || _stopsFilePath.isEmpty() || _tripsFilePath.isEmpty()) {
        qWarning() << "One file is missing from the directory";
    }

    _trajectories = QMap<QString, Trajectory*>();
}

void GTFSTrace::parseTrips(Loader* loader) {
    QVector<QMap<QString, QString>> stopList   = QVector<QMap<QString, QString>>();
    QVector<QMap<QString, QString>> timesList  = QVector<QMap<QString, QString>>();
    QVector<QMap<QString, QString>> tripsList  = QVector<QMap<QString, QString>>();
    QVector<QMap<QString, QString>> shapesList = QVector<QMap<QString, QString>>();
    QVector<QMap<QString, QString>> routesList = QVector<QMap<QString, QString>>();

    CSVParser::parseCSV(_stopsFilePath,     stopList, ",");
    CSVParser::parseCSV(_stopTimesFilePath, timesList, ",");
    CSVParser::parseCSV(_tripsFilePath,     tripsList, ",");
    CSVParser::parseCSV(_shapesFilePath,    shapesList, ",");
    CSVParser::parseCSV(_routesFilePath,    routesList, ",");

    // TODO: case if the folder does not contain a shapes.txt file

    qDebug() << stopList.count() << " / " << timesList.count() << " / " << tripsList.count() << " / " << shapesList.count();

    // get all the trips (trajectories)
    QString progressMsg    = "Parse trip";
    double  progressFactor = _snapToShape ? 0.1 : 0.3;

    QSet<QString> okRoutes;
    QSet<QString> okTrips;
    QSet<QString> okShapes;

    // filter the bus routes (route_type = 3)
    // see https://developers.google.com/transit/gtfs/reference#routestxt
    foreach (auto route, routesList) {
        QString routeId  = route.value("route_id");
        int routeType    = route.value("route_type").toInt();
        if(routeType == 3)
            okRoutes.insert(routeId);
    }

    int size = tripsList.size();
    int count = 0;
    foreach (auto trip, tripsList) {
        QString routeId      = trip.value("route_id");
        QString serviceId    = trip.value("service_id");
        QString tripId       = trip.value("trip_id");
        QString tripHeadsign = trip.value("trip_headsign");
        QString directionId  = trip.value("direction_id");
        QString shapeId      = trip.value("shape_id");

        if(!okRoutes.contains(routeId) || serviceId != "1") {
            continue;
        }

        okTrips.insert(tripId);
        okShapes.insert(shapeId);

        _trajectories.insert(tripId,
                             new Trajectory(routeId, serviceId, tripId, tripHeadsign, directionId, shapeId));

        // add the corresponding trip_id to the shape_id
        if(!_shapesToTrips.contains(shapeId))
            _shapesToTrips.insert(shapeId, new QSet<QString>());
        _shapesToTrips.value(shapeId)->insert(tripId);

        loader->loadProgressChanged(progressFactor*((double)count / (double)size), progressMsg+" "+tripId);
        count ++;
    }

    qDebug() << "trajectories count: " << _trajectories.count();

    // get all the stops
    _stops = QMap<QString, Stop*>();
    foreach (auto stop, stopList) {
        QString stopId   = stop.value("stop_id");
        QString stopName = stop.value("stop_name");
        double lat       = stop.value("stop_lat").toDouble();
        double lon       = stop.value("stop_lon").toDouble();
        double x, y;
        ProjFactory::getInstance().transformCoordinates(lat, lon, &x, &y);
        QPointF coord(x, y);

        if(x < 500 || y < 500) {
            continue;
        }

        _stops.insert(stopId,
                      new Stop(stopId, stopName, coord));
    }

    qDebug() << "_stops " << _stops.count();

    // get all the shapes
    _shapes = QMap<QString, QMap<int,QPointF>* >();
    QMap<QString, geos::geom::LineString*> geomShapes;
    if(_snapToShape) {
        geos::geom::GeometryFactory* global_factory = new geos::geom::GeometryFactory();
        foreach(auto waypoint, shapesList) {
            QString shapeId = waypoint.value("shape_id");
            double lat      = waypoint.value("shape_pt_lat").toDouble();
            double lon      = waypoint.value("shape_pt_lon").toDouble();
            int seq         = waypoint.value("shape_pt_sequence").toInt();

            if(!okShapes.contains(shapeId))
                continue;

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
        QString tripId          = time.value("trip_id");
        QString stopId          = time.value("stop_id");
        long long arrivalTime   = toSeconds(time.value("arrival_time"));
        long long departureTime = toSeconds(time.value("departure_time"));
        int seq                 = time.value("stop_sequence").toInt();

        if(!okTrips.contains(tripId))
            continue;

        Stop* stop = _stops.value(stopId);
        WayPoint* wp = new WayPoint(stop, arrivalTime, departureTime);

        _stopsUsed.insert(stop->getId(), stop);

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
            Trajectory* trajectory = _trajectories.value(tripId);
            QString shapeId = trajectory->getShapeId();

            // convert the corresponding shape into a LineString
            if(!geomShapes.contains(shapeId))
                continue;

            geos::geom::LineString* ls = geomShapes.value(shapeId);
            geos::linearref::LocationIndexedLine* lineRef = new geos::linearref::LocationIndexedLine(ls);

            // get the first waypoint
            auto tIt = it.value()->begin();
            WayPoint* wp1 = tIt.value();
            geos::geom::Coordinate pt1(wp1->getCoords().x(), wp1->getCoords().y());
            geos::linearref::LinearLocation loc1 = lineRef->project(pt1);
            long long startTime = wp1->getDepartureTime();
            geos::geom::Coordinate projWp1 = loc1.getCoordinate(ls);
            wp1->setCoords(QPointF(projWp1.x, projWp1.y));
            trajectory->addWayPoint(wp1);

            tIt++; // go onto the second waypoint
            // snap each waypoint of the trajectory to the shape linestring
            for(; tIt != it.value()->end(); ++tIt) {
                WayPoint* wp2 = tIt.value();
                geos::geom::Coordinate pt2(wp2->getCoords().x(), wp2->getCoords().y());
                geos::linearref::LinearLocation loc2 = lineRef->project(pt2);
                long long endTime = wp2->getArrivalTime();
                geos::geom::Coordinate projWp2 = loc2.getCoordinate(ls);

                // Get the partial time
                geos::geom::LineString* partialLine = dynamic_cast<geos::geom::LineString*>(
                        lineRef->extractLine(loc1, loc2));
                double totalLength = partialLine->getLength();
                double sumLength = 0;

                // Insert the points in the linestring between loc1 and loc2
                // Interpolate the time of each waypoint
                // ignore the two extremity waypoints
                geos::geom::Coordinate prevPt = partialLine->getCoordinateN(0);
                for(int ptIdx = 1; ptIdx < partialLine->getNumPoints()-1; ++ptIdx) {
                    geos::geom::Coordinate curPt = partialLine->getCoordinateN(ptIdx);

                    double distance = prevPt.distance(curPt);
                    sumLength += distance;

                    long long time = (long long) (startTime + (endTime - startTime) * (sumLength/totalLength));
                    // add the new waypoint to the trajectory
                    WayPoint* wp = new WayPoint(QPointF(curPt.x, curPt.y), time, time);
                    trajectory->addWayPoint(wp);

                    // increment the distance and previous point
                    prevPt = curPt;
                }

                // add the waypoint to the trajectory
                wp2->setCoords(QPointF(projWp2.x, projWp2.y));
                trajectory->addWayPoint(wp2);


                // update the waypoints
                wp1 = wp2;
                pt1 = pt2;
                loc1 = loc2;
                startTime = endTime;
                projWp1 = projWp2;
            }
        }

        qDebug() << "trajectories" << _trajectories.count();
    }
}

long long GTFSTrace::toSeconds(QString time) {
    long hours = time.mid(0, 2).toLong();
    long minutes = time.mid(3, 2).toLong();
    long seconds = time.mid(6, 2).toLong();
    return hours * 3600 + minutes * 60 + seconds;
}

bool GTFSTrace::openTrace(Loader* loader) {
    qDebug() << "Begin parsing trips";
    loader->loadProgressChanged(0.0, "Parsing the trips");
    parseTrips(loader);

    loader->loadProgressChanged(0.1, "Adding trajectories");

    int id = 0;
    int count = _trajectories.size();
    for(auto it = _trajectories.begin(); it != _trajectories.end(); ++it) {
        Trajectory* traj = it.value();
        QString trajId = it.key();

        QMap<long long, WayPoint*> trajectory;
        traj->getTrajectory(&trajectory);
        for (auto mvt: trajectory.keys()) {
            WayPoint* stop = trajectory.value(mvt);
            addPoint(trajId, mvt,
                     stop->getCoords().x(), stop->getCoords().y());
        }
        id++;

        loader->loadProgressChanged(0.1 + 0.9 * ((qreal) id / (qreal) count), "Adding trajectory "+trajId);
//        qDebug() << "Trajectories added " << id;
//        if(id > 1000) break;
    }

    qDebug() << "[DONE] Added" << _nodes.size() << "nodes";
    loader->loadProgressChanged((qreal) 1.0, "Done");

    return true;
}

void GTFSTrace::getTrajectoryLineString(const QString& shapeId,
                                        geos::geom::LineString*& stops,
                                        const QSet<Stop*>& stopsToDiscard) {
    /* Project the stops on the corresponding shapes */

    // get one trajectory for the given routeId
    int maxCount = 0;
    QString maxTrajId;
    for(const QString trajId : *(_shapesToTrips.value(shapeId))) {
        int count = _trajectories.value(trajId)->getNbWaypoints();
        if(count > maxCount) {
            maxCount = count;
            maxTrajId = trajId;
        }
//        qDebug() << shapeId << trajId << _trajectories.value(trajId)->getNbWaypoints() << _trajectories.value(trajId)->getNbStops() << _shapes.value(shapeId)->size();
    }

    QString trajId = maxTrajId;
    Trajectory* trajectory = _trajectories.value(trajId);

    qDebug() << "shapeId" << shapeId << "trajId" << trajId << _trajectories.contains(trajId);
    qDebug() << "trajectory" << trajectory->getStartTime() << trajectory->getEndTime();

    geos::geom::GeometryFactory* global_factory = new geos::geom::GeometryFactory();

    // compute the Linestring from the list of waypoints of the trajectory
    QMap<long long, WayPoint*> trajList;
    trajectory->getTrajectory(&trajList);
    geos::geom::CoordinateSequence* clStops = new geos::geom::CoordinateArraySequence();

    for(auto it = trajList.begin(); it != trajList.end(); ++it) {
        WayPoint* wp = it.value();
        if(wp->isStop() && !stopsToDiscard.contains(wp->getStop())) {
            QPointF projStopCoords = wp->getStop()->getProjCoords();
            if(projStopCoords.isNull())
                continue;
            geos::geom::Coordinate coord(projStopCoords.x(), projStopCoords.y());
            clStops->add(coord);
        }

        if(wp->isStop() && stopsToDiscard.contains(wp->getStop()))
            qDebug() << shapeId << trajId << "Discarding stop" << wp->getStop()->toString();
    }

    // create the new stop linestring
    stops = global_factory->createLineString(clStops);
}

double GTFSTrace::getAverageNumberOfVehicles(const QString& shapeId) {
    QList<QString> trajectories(_shapesToTrips.value(shapeId)->toList());

    double nrofVehicles = 0.0;
    for(int i = 0 ; i < trajectories.size(); ++i) {
        Trajectory* traj1 = _trajectories.value(trajectories.at(i));

        long long startTime1 = traj1->getStartTime();
        long long endTime1   = traj1->getEndTime();

        if(startTime1 <= 3600*16 && endTime1 >= 3600*16) {
            nrofVehicles += 1.0;
        }
    }

    return nrofVehicles;
}

void GTFSTrace::getAverageSpeed(const QString& shapeId, double* mean, double* stddev) {
    QList<QString> trajectories(_shapesToTrips.value(shapeId)->toList());
    QList<double> averages;
    double speed = 0.0;
    for(int i = 0 ; i < trajectories.size(); ++i) {
        double avg = averageSpeed(trajectories.at(i));
        averages.append(avg);
        speed += avg;
    }
    double average = speed / trajectories.size();

    double variance = 0.0;
    for(int i = 0; i < averages.size(); ++i) {
        double x = averages.at(i);
        variance += qPow(average-x, 2);
    }

    *mean = average;
    *stddev = qSqrt(variance);
}

void GTFSTrace::pushLinestring(geos::geom::Geometry *geom, std::vector<geos::geom::Geometry *>* vector) {
    if(geom->getGeometryTypeId() == geos::geom::GEOS_LINESTRING)
        vector->push_back(geom);
    else if(geom->getGeometryTypeId() == geos::geom::GEOS_MULTILINESTRING) {
        for(int i = 0; i < geom->getNumGeometries(); ++i) {
            geos::geom::Geometry* g = const_cast<geos::geom::Geometry*>(geom->getGeometryN(i));
            if(g->getGeometryTypeId() == geos::geom::GEOS_LINESTRING) {
                vector->push_back(g);
            }
        }
    } else if(geom->getGeometryTypeId() == geos::geom::GEOS_GEOMETRYCOLLECTION) {
        for(int i = 0; i < geom->getNumGeometries(); ++i) {
            geos::geom::Geometry* g = const_cast<geos::geom::Geometry*>(geom->getGeometryN(i));
            pushLinestring(g, vector); // recursive call
        }
    }
}

void GTFSTrace::outputLines(QList<geos::geom::LineString *> *lines, const QString &output) {
    qDebug() << "Dump lines in" << output;

    QFile file(output);
    if(!file.open(QFile::WriteOnly)) {
        qWarning() << "Unable to write in file" << output;
        return;
    }

    geos::io::WKTWriter writer;
    QTextStream out(&file);
    writer.setRoundingPrecision(3); // remove unnecessary zeros
    for(auto it = lines->begin(); it != lines->end(); ++it) {
        const geos::geom::Geometry* geom = *it;
        qDebug() << "type" << QString::fromStdString(geom->getGeometryType());
        if(geom->getGeometryTypeId() == geos::geom::GEOS_LINESTRING) {
            out << QString::fromStdString(writer.write(geom)) << "\n";
        }
    }
}


void GTFSTrace::outputWorldMap(QList<geos::geom::LineString*>* lines, const QString& output) {
/** record all the trajectories (shapes) into the multilinestring "lines" */

    qDebug() << "create world and dump it in" << output;

    geos::geom::GeometryFactory *global_factory = new geos::geom::GeometryFactory();
    std::vector<geos::geom::Geometry*> mlsVector;
    for(auto it = lines->begin(); it != lines->end(); ++it) {

        // get the linestring
        geos::geom::LineString* ls = *it;

        // remove the parts of the current linestring that are shared by the previous recorded linestrings
        geos::geom::Geometry* result = ls;
        geos::geom::Geometry* resultIntersections = nullptr;

        std::vector<geos::geom::Geometry*> tmpVector;

        for(auto it = mlsVector.begin(); it != mlsVector.end();) {
            geos::geom::Geometry* lsOther = *it;
            geos::geom::Geometry* intersection = ls->intersection(lsOther);

            if(intersection->getNumGeometries() > 0) {
                it = mlsVector.erase(it);

                // update  the resulting ogrGeometry
                result = result->difference(intersection);
                // get the resulting difference
                geos::geom::Geometry* diff = lsOther->difference(intersection);
                // add the difference to the temporary vector
                pushLinestring(diff, &tmpVector);

                // update the resulting intersections
                if(resultIntersections != nullptr)
                    resultIntersections->Union(intersection);
                else resultIntersections = intersection;

            } else  {
                // go onto the next element of the vector
                ++it;
            }
        }

        // insert the temporary vector to the mls vector
        mlsVector.insert(mlsVector.end(), tmpVector.begin(), tmpVector.end());

        // insert the resulting LS (with the intersection parts)
        if(resultIntersections != nullptr)
            result = result->Union(resultIntersections);

        pushLinestring(result, &mlsVector);
    }

    geos::geom::MultiLineString* mls = global_factory->createMultiLineString(mlsVector);
    geos::io::WKTWriter writer;

// dump the multilinestring in the file
    QFile file(output);
    if(!file.open(QFile::WriteOnly)) {
        qWarning() << "Unable to write in file" << output;
        return;
    }

    QTextStream out(&file);
    writer.setRoundingPrecision(3); // remove unnecessary zeros
    for(int i = 0; i < mls->getNumGeometries(); ++i) {
        const geos::geom::Geometry* geom = mls->getGeometryN(i);
        qDebug() << "type" << QString::fromStdString(geom->getGeometryType());
        if(geom->getGeometryTypeId() == geos::geom::GEOS_LINESTRING) {
            out << QString::fromStdString(writer.write(geom)) << "\n";
        }
    }

    file.close();
}

