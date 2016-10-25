//
// Created by Benjamin Baron on 19/10/2016.
//

#include "waze_alert_file_layer.h"
#include "geometries.h"
#include "waze_alert_cells.h"

QGraphicsItemGroup *WazeAlertFileLayer::draw() {
    int radius = 20;
    _groupItem = new QGraphicsItemGroup();

    QHash<QString, QMap<long long, WazeAlert*>*>* alerts = _wazeAlertFile->getAlerts();
    for(auto it = alerts->begin(); it != alerts->end(); it++) {
        for(auto jt = it.value()->begin(); jt != it.value()->end(); jt++) {
            WazeAlert* wazeAlert = jt.value();
            QPointF p = wazeAlert->pos;
            GeometryGraphics* item = new CircleGraphics(new Circle(p, radius));
            item->setPen(Qt::NoPen);
            switch(wazeAlert->type) {
                case WazeAlertType::WAZE_TYPE_JAM:
                    item->setBrush(QBrush(COL_JAM));
                    break;
                case WazeAlertType::WAZE_TYPE_ROAD_CLOSED:
                    item->setBrush(QBrush(COL_ROAD_CLOSED));
                    break;
                case WazeAlertType::WAZE_TYPE_WEATHERHAZARD:
                    item->setBrush(QBrush(COL_WEATHERHAZARD));
                    break;
                case WazeAlertType::WAZE_TYPE_ACCIDENT:
                    item->setBrush(QBrush(COL_ACCIDENT));
                    break;
                default:
                    break;
            }

            addGraphicsItem(item);
//        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        }
    }
    return _groupItem;
}

bool WazeAlertFileLayer::load(Loader *loader) {
    bool ret = _wazeAlertFile->open(loader);
    return ret;
}

void WazeAlertFileLayer::addMenuBar() {
    _menu = new QMenu();
    _menu->setTitle("Waze alerts");
    _parent->addMenu(_menu);

    // add action to the menu to export a cellsfv file
    QAction* actionExportContour = _menu->addAction("Create cells layer");
    connect(actionExportContour, &QAction::triggered, this, [=](bool checked){
        qDebug() << "Compute waze cells";
        if(!_wazeAlertCellsLayer) {
            QString layerName = "Waze Alert Cell layer";

            WazeAlertCells* wac = new WazeAlertCells(_wazeAlertFile,100.0);
            _wazeAlertCellsLayer = new WazeAlertCellsLayer(_parent, layerName, wac);
            Loader loader;
            _parent->createLayer(layerName, _wazeAlertCellsLayer, &loader);
        }
    });
}