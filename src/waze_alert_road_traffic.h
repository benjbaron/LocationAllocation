//
// Created by Benjamin Baron on 17/11/2016.
//

#ifndef LOCALL_WAZE_ALERT_ROAD_TRAFFIC_H
#define LOCALL_WAZE_ALERT_ROAD_TRAFFIC_H


#include "road_traffic.h"
#include "waze_alert_data.h"

struct ProjectedWazeAlert : public ProjectedPoint {
    ProjectedWazeAlert(QPointF o, QPointF p) : ProjectedPoint(o,p) {}
    WazeAlert* wazeAlert;
    QHash<int, QPointF> candidateProjectedPoints;
    QHash<int, ShapefileFeature*> candidateFeatures;
    QHash<int, double> candidateAngles;
    QHash<int, QLineF> candidateSegments;
};

struct RoadLinkWazeAlerts {
    RoadLinkWazeAlerts(RoadLink* roadLink) {
        this->roadLink = roadLink;
    }
    void addAlert(WazeAlert* alert) {
        alerts.insert(alert->timestamp, alert);
    }
    RoadLink* roadLink;
    QMap<long long, WazeAlert*> alerts;
};

class WazeAlertRoadTraffic : public RoadTraffic {
public:
    WazeAlertRoadTraffic(const QString& shapefilePath, const QString& dataPath, const QString& additionalPath,
            ShapefileIndexes* shapefileIdx, RoadTrafficDataIndexes* dataIdx,
            WazeAlertData* wazeAlerts) :
            RoadTraffic(shapefilePath, dataPath, additionalPath, shapefileIdx, dataIdx), _wazeAlerts(wazeAlerts) { }

    bool open(Loader* loader);
    QHash<Geometry*, QString>* getBufferedRoadLinks() {
        return &_bufferedRoadLinks;
    }
    QHash<QString, RoadLinkWazeAlerts*>* getRoadLinksAlerts() {
        return &_roadLinksAlerts;
    };
    RoadLinkWazeAlerts* getRoadLinkAlerts(QString idx) {
        return _roadLinksAlerts.value(idx);
    }
    RoadLinkWazeAlerts* getRoadLinkAlerts(int idx) {
        /* Get the road link alerts corresponding to the shapefile index */
        int joinIdx = indexes()->join;
        QString rlIdx = getFeature(idx)->attributes->at(joinIdx);
        if(_roadLinks.contains(rlIdx))
            return _roadLinksAlerts.value(rlIdx);

        return nullptr;
    }
    GeometryIndex* getGeometryIndex() {
        return _geometryIndex;
    }
    const QHash<QPointF, ProjectedPoint*>& getProjectedPoints() {
        return _projectedPoints;
    }
    WazeAlertData* getWazeAlertData() {
        return _wazeAlerts;
    }


protected:
    WazeAlertData* _wazeAlerts;
    QHash<QString, RoadLinkWazeAlerts*> _roadLinksAlerts; // indexed by the road link id
    QHash<Geometry*, QString> _bufferedRoadLinks; // indexed by the buffered road link pointer
    GeometryIndex* _geometryIndex;
    QHash<QPointF, ProjectedPoint*> _projectedPoints;

    bool populateAlerts(Loader* loader);

};


#endif //LOCALL_WAZE_ALERT_ROAD_TRAFFIC_H
