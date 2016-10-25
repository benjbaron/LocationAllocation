//
// Created by Benjamin Baron on 20/10/2016.
//

#ifndef LOCALL_WAZE_ALERT_CELLS_LAYER_H
#define LOCALL_WAZE_ALERT_CELLS_LAYER_H


#include "layer.h"
#include "waze_alert_cells.h"
#include "geometries.h"
#include "waze_alert_cells_panel.h"

class WazeAlertCellsLayer : public Layer {
    Q_OBJECT

public:
    WazeAlertCellsLayer(MainWindow* parent = 0, QString name = 0, WazeAlertCells* wazeAlertCells = nullptr) :
            Layer(parent, name), _wazeAlertCells(wazeAlertCells) {
        if(parent) {
            addMenuBar();
        }
    }

    virtual void addMenuBar();
    virtual bool load(Loader *loader) {
        bool ret = _wazeAlertCells->computeCells();
        loader->loadProgressChanged(1.0, "DONE");
        return ret;
    }
    virtual QGraphicsItemGroup* draw();

private slots:
    void onWazeAlertCellSelected(Geometry* geom, bool mod);
    void exportCellFile();

private:
    WazeAlertCells* _wazeAlertCells = nullptr;
    Geometry* _selectedGeometry = nullptr;
    QHash<Geometry*, GeometryGraphics*> _geometryGraphics;
    WazeAlertCellsPanel* _wazeAlertCellsPanel = nullptr;
};


#endif //LOCALL_WAZE_ALERT_CELLS_LAYER_H
