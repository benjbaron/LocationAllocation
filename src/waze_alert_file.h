//
// Created by Benjamin Baron on 19/10/2016.
//

#ifndef LOCALL_WAZE_ALERT_FILE_H
#define LOCALL_WAZE_ALERT_FILE_H


#include "loader.h"

enum WazeAlertType {
    WAZE_TYPE_NONE,
    WAZE_TYPE_JAM,
    WAZE_TYPE_ROAD_CLOSED,
    WAZE_TYPE_WEATHERHAZARD,
    WAZE_TYPE_ACCIDENT
};

enum WazeAlertSubType {
    WAZE_SUBTYPE_NONE,
    WAZE_SUBTYPE_JAM_HEAVY_TRAFFIC,
    WAZE_SUBTYPE_JAM_STAND_STILL_TRAFFIC,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_POT_HOLE,
    WAZE_SUBTYPE_JAM_MODERATE_TRAFFIC,
    WAZE_SUBTYPE_ROAD_CLOSED_EVENT,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_CAR_STOPPED,
    WAZE_SUBTYPE_HAZARD_ON_SHOULDER_CAR_STOPPED,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_CONSTRUCTION,
    WAZE_SUBTYPE_ACCIDENT_MINOR,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_OBJECT,
    WAZE_SUBTYPE_ACCIDENT_MAJOR,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_ICE,
    WAZE_SUBTYPE_HAZARD_ON_ROAD,
    WAZE_SUBTYPE_HAZARD_ON_SHOULDER,
    WAZE_SUBTYPE_ROAD_CLOSED_HAZARD,
    WAZE_SUBTYPE_HAZARD_WEATHER,
    WAZE_SUBTYPE_HAZARD_ON_SHOULDER_MISSING_SIGN,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_ROAD_KILL,
    WAZE_SUBTYPE_ROAD_CLOSED_CONSTRUCTION,
    WAZE_SUBTYPE_HAZARD_ON_SHOULDER_ANIMALS,
    WAZE_SUBTYPE_HAZARD_WEATHER_FOG,
    WAZE_SUBTYPE_HAZARD_WEATHER_FREEZING_RAIN
};

struct WazeAlert {
    WazeAlert(QString uuid = QString(),
              QPointF pos = QPointF(),
              WazeAlertType type = WazeAlertType::WAZE_TYPE_NONE,
              QString subtype = QString(),
              long long timestamp = -1,
              int roadType = -1,
              int nbRatings = -1) {
        this->uuid = uuid;
        this->pos = pos;
        this->type = type;
        this->subType = subtype;
        this->timestamp = timestamp;
        this->roadType = roadType;
        this->nbRatings = nbRatings;
    }

    QString uuid;
    QPointF pos;
    WazeAlertType type;
    QString subType;
    long long timestamp;
    int roadType;
    int nbRatings;
};

class WazeAlertFile {
public:
    WazeAlertFile(const QString& wazeAlertFile):
            _wazeAlertFile(wazeAlertFile) { }

    virtual bool open(Loader* loader);
    QHash<QString, QMap<long long, WazeAlert*>*>* getAlerts() {
        return &_alerts;
    };

private:
    QString _wazeAlertFile;
    QHash<QString, QMap<long long, WazeAlert*>*> _alerts;

    WazeAlertType stringToAlertType(const QString& s) {
        if(s == "JAM") return WazeAlertType::WAZE_TYPE_JAM;
        else if(s == "WEATHERHAZARD") return WazeAlertType::WAZE_TYPE_WEATHERHAZARD;
        else if(s == "ROAD_CLOSED") return WazeAlertType::WAZE_TYPE_ROAD_CLOSED;
        else if(s == "ACCIDENT") return WazeAlertType::WAZE_TYPE_ACCIDENT;
        else return WazeAlertType::WAZE_TYPE_NONE;
    }
};


#endif //LOCALL_WAZE_ALERT_FILE_H
