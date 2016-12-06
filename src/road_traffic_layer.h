//
// Created by Benjamin Baron on 21/09/2016.
//

#ifndef LOCALL_ROAD_TRAFFIC_LAYER_H
#define LOCALL_ROAD_TRAFFIC_LAYER_H

#include "layer.h"
#include "road_traffic.h"
#include "road_traffic_examiner_panel.h"
#include "shapefile_layer.h"


class RoadTrafficLayer: public ShapefileLayer {
    Q_OBJECT
public:
    RoadTrafficLayer(MainWindow* parent = 0, QString name = 0, RoadTraffic* roadTraffic = nullptr):
            ShapefileLayer(parent, name, roadTraffic) {
        if(parent) {
            addMenuBar();
        }
    }

    virtual QGraphicsItemGroup* draw();
    virtual bool load(Loader* loader);
    virtual void addMenuBar() {}

private slots:
    void onFeatureSelectedEvent(Geometry* geom, bool mod);


protected:
    RoadTrafficExaminerPanel* _roadTrafficExaminerPanel = nullptr;
    int _selectedRoadLinkId = -1;
};


#endif //LOCALL_ROAD_TRAFFIC_LAYER_H
