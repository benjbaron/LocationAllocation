//
// Created by Benjamin Baron on 01/03/16.
//

#ifndef LOCALL_SPATIALSTATSLAYER_H
#define LOCALL_SPATIALSTATSLAYER_H


#include "layer.h"
#include "spatial_stats.h"
#include "dockwidget_plots.h"
#include "compute_allocation_layer.h"

class SpatialStatsLayer : public Layer {
    Q_OBJECT

public:
    SpatialStatsLayer(MainWindow* parent = 0, QString name = 0, SpatialStats* spatialStats = 0) :
            Layer(parent, name), _spatialStats(spatialStats) {
        if(parent) {
            addMenuBar();
        }
    }

    virtual bool load(Loader *loader);
    virtual QGraphicsItemGroup* draw();
    virtual void addMenuBar();

private slots:
    void computeAllocation();
    void exportContourFile();
    void startRESTServer();
    void onMousePress(Geometry* geom, bool mod);

private:
    SpatialStats* _spatialStats = nullptr;
    ComputeAllocationLayer* _computeAllocationLayer = nullptr;
    Geometry* _selectedGeometry = nullptr;
    QHash<Geometry*, GeometryGraphics*> _geometryGraphics;
    DockWidgetPlots* _plots = nullptr;
    RESTServer* _restServer = nullptr;
};


#endif //LOCALL_SPATIALSTATSLAYER_H
