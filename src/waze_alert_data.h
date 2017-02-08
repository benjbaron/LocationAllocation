//
// Created by Benjamin Baron on 19/10/2016.
//

#ifndef LOCALL_WAZE_ALERT_FILE_H
#define LOCALL_WAZE_ALERT_FILE_H


#include "loader.h"
#include "constants.h"

struct WazeAlert {
    WazeAlert(QString uuid = QString(),
              QPointF pos = QPointF(),
              WazeAlertType type = WazeAlertType::WAZE_TYPE_NONE,
              WazeAlertSubType subtype = WazeAlertSubType::WAZE_SUBTYPE_NONE,
              long long timestamp = -1,
              int roadType = -1,
              int nbRatings = -1,
              int magvar = 0,
              QString street = QString()) {
        this->uuid = uuid;
        this->pos = pos;
        this->type = type;
        this->subType = subtype;
        this->timestamp = timestamp;
        this->roadType = roadType;
        this->nbRatings = nbRatings;
        this->magvar = magvar;
        this->street = street;
    }

    QString uuid;
    QPointF pos;
    WazeAlertType type;
    WazeAlertSubType subType;
    long long timestamp;
    int roadType;
    int nbRatings;
    int magvar; // direction of the alert (between -359 and +359)
    QString street;
};


class WazeAlertData {
public:
    WazeAlertData(const QString& wazeAlertFile):
            _wazeAlertFile(wazeAlertFile) { }

    bool open(Loader* loader);
    QHash<QString, QMap<long long, WazeAlert*>*>* getAlerts() {
        return &_alerts;
    };
    QSet<WazeAlertSubType>* getAlertSubTypes(WazeAlertType wat) {
        if(_alertTypes.contains(wat)) {
            return _alertTypes.value(wat);
        }
        return nullptr;
    }

private:
    QString _wazeAlertFile;
    QHash<QString, QMap<long long, WazeAlert*>*> _alerts;
    QHash<WazeAlertType,QSet<WazeAlertSubType>*> _alertTypes;
};


#endif //LOCALL_WAZE_ALERT_FILE_H
