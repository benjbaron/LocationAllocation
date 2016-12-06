//
// Created by Benjamin Baron on 28/11/2016.
//

#ifndef LOCALL_ROAD_TRAFFIC_WAZE_ALERTS_OPEN_DIALOG_H
#define LOCALL_ROAD_TRAFFIC_WAZE_ALERTS_OPEN_DIALOG_H


#include "road_traffic_open_dialog.h"

class RoadTrafficWazeAlertsOpenDialog: public RoadTrafficOpenDialog {
    Q_OBJECT

public:
    RoadTrafficWazeAlertsOpenDialog(QWidget* parent = 0, const QString& name = "");

private:
    QString _wazeAlertFilePath = "";
    void saveFields();
    bool checkConsistency();
};


#endif //LOCALL_ROAD_TRAFFIC_WAZE_ALERTS_OPEN_DIALOG_H
