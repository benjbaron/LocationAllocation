//
// Created by Benjamin Baron on 21/09/2016.
//

#ifndef LOCALL_ROAD_TRAFFIC_LAYER_H
#define LOCALL_ROAD_TRAFFIC_LAYER_H

#include "layer.h"
#include "road_traffic.h"
#include "road_traffic_examiner_panel.h"

class ArrowLineItem;

const QColor COL_LOW(56,168,0);
const QColor COL_MED_LOW(176,224,0);
const QColor COL_MED_HIGH(255,170,0);
const QColor COL_HIGH(255,0,0);

class RoadTrafficLayer: public Layer {
    Q_OBJECT
public:
    RoadTrafficLayer(MainWindow* parent = 0, QString name = 0, RoadTraffic* roadTraffic = nullptr):
        Layer(parent, name), _roadTraffic(roadTraffic) {
        if(parent) {
            addMenuBar();
        }
    }

    virtual QGraphicsItemGroup* draw();
    virtual bool load(Loader* loader);
    virtual void addMenuBar() {}

private slots:
    void onRoadTrafficLinkSelected(int nodeId, int linkId, bool mod, void* ptr);


protected:
    RoadTraffic* _roadTraffic = 0;
    QList<ArrowLineItem*> _linkGraphics;
    RoadTrafficExaminerPanel* _roadTrafficExaminerPanel = nullptr;
    int _selectedRoadLinkId = -1;

};


#endif //LOCALL_ROAD_TRAFFIC_LAYER_H
