//
// Created by Benjamin Baron on 17/11/2016.
//

#include "waze_alert_road_traffic.h"
#include "geometry_index.h"



bool WazeAlertRoadTraffic::open(Loader* loader) {

    qDebug() << "open Waze Alert Road Traffic";
    RoadTraffic::open(nullptr);
    if(loader != nullptr)
        loader->loadProgressChanged(0.3, "Shapefile loaded");

    qDebug() << "populate the alerts";
    /* populate the _roadLinksAlerts hash set */
    populateAlerts(loader);

    if(loader != nullptr)
        loader->loadProgressChanged(1.0, "Done");

    return true;
}

bool WazeAlertRoadTraffic::populateAlerts(Loader* loader) {
    for(auto it = _roadLinks.begin(); it != _roadLinks.end(); ++it) {
        QString id = it.key();
        RoadLink* rl = it.value();

        ShapefileFeature* shpFeature = rl->getShapefileFeature();
        OGRGeometry* geom = shpFeature->ogrGeometry;
        if(geom->getGeometryType() == wkbLineString) {
            // get the linestring
            OGRLineString* ls = (OGRLineString*) geom;

            // buffer the linestring
            OGRGeometry* buffer = ls->Buffer(50);

            // add the linestring to the buffered road links hash set
            _bufferedRoadLinks.insert(OGRGeometryToGeometry(buffer), id);
        }
    }

    // create a geometry index with the buffered road links
    _geometryIndex = new GeometryIndex(_bufferedRoadLinks.keys().toSet(), 100);
    qDebug() << "number of geometry index cells" << _geometryIndex->getGrid().size();

    /* Match the Waze alerts with the closest road link */
    QHash<QString, QMap<long long, WazeAlert*>*>* alerts = _wazeAlerts->getAlerts();

    for(auto it = alerts->begin(); it != alerts->end(); ++it) {
        QString user = it.key();
        for(auto jt = it.value()->begin(); jt != it.value()->end(); ++jt) {
            long long time = jt.key();
            WazeAlert* alert = jt.value();
            QPointF alertPos = alert->pos;
            double magvar = -1*alert->magvar;

            // try to match the alert with the closest road link
            QSet<Geometry*>* closestGeometries = _geometryIndex->getGeometriesAt(alert->pos);
            if(closestGeometries == nullptr) {
//                qDebug() << "no geometry within 100 meters";
                continue;
            }
            double minDist = 1000;
            Geometry* closestGeom = nullptr;
            ShapefileFeature* closestShpFeature = nullptr;
            QPointF closestProjectedPt = QPointF();
            ProjectedWazeAlert* projPoint = new ProjectedWazeAlert(alertPos, QPointF());
            projPoint->wazeAlert = alert;
            for(Geometry* geom : *closestGeometries) {
                QString roadLinkId = _bufferedRoadLinks.value(geom);
                if(!_roadLinks.contains(roadLinkId)) {
                    qDebug() << "Road link id" << roadLinkId << "not contained in the road link set";
                    continue;
                }

                ShapefileFeature* shpFeature = _roadLinks.value(roadLinkId)->getShapefileFeature();
                int shpId = shpFeature->id;
                OGRGeometry* ogrGeometry = shpFeature->ogrGeometry;
                OGRLineString* ogrls = (OGRLineString*) ogrGeometry;
                double angle;
                QLineF lineSegment;
                QPointF projectedPt = projectPointOnLineString(ogrls, alertPos, &angle, &lineSegment);
                projPoint->candidateProjectedPoints.insert(shpId, projectedPt);
                projPoint->candidateFeatures.insert(shpId, shpFeature);
                projPoint->candidateAngles.insert(shpId, angle);
                projPoint->candidateSegments.insert(shpId, lineSegment);

                double dist = euclideanDistance(alertPos, projectedPt);
                if(dist < minDist && qAbs(angle - magvar) < 15) {
                    minDist = dist;
                    closestGeom = geom;
                    closestProjectedPt = projectedPt;
                }
            }

            // add the alert to the corresponding road link
            QString roadLinkId = _bufferedRoadLinks.value(closestGeom);
            RoadLink* roadLink = _roadLinks.value(roadLinkId);
            if(!_roadLinksAlerts.contains(roadLinkId)) {
                _roadLinksAlerts.insert(roadLinkId, new RoadLinkWazeAlerts(roadLink));
            }
            _roadLinksAlerts.value(roadLinkId)->addAlert(alert);

            // add the projected point
            projPoint->projectedPoint = closestProjectedPt;
            _projectedPoints.insert(alertPos, projPoint);
        }
    }

    return true;
}