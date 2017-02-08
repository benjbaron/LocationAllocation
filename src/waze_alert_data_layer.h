//
// Created by Benjamin Baron on 19/10/2016.
//

#ifndef LOCALL_WAZE_ALERT_FILE_LAYER_H
#define LOCALL_WAZE_ALERT_FILE_LAYER_H


#include "layer.h"
#include "waze_alert_data.h"
#include "waze_alert_cells_layer.h"
#include "waze_alert_road_traffic_layer.h"

class WazeAlertDataLayer : public Layer {
    Q_OBJECT
public:
    WazeAlertDataLayer(MainWindow* parent = 0, QString name = 0, WazeAlertData* wazeAlertData = nullptr) :
        Layer(parent, name), _wazeAlertData(wazeAlertData) {
        if(parent) {
            addMenuBar();
        }
    }

    virtual QGraphicsItemGroup* draw();
    virtual bool load(Loader* loader);
    virtual void addMenuBar();

private:
    WazeAlertData* _wazeAlertData = nullptr;
    WazeAlertCellsLayer* _wazeAlertCellsLayer = nullptr;
    WazeAlertRoadTrafficLayer* _wazeAlertRoadTrafficLayer = nullptr;
};


#endif //LOCALL_WAZE_ALERT_FILE_LAYER_H
