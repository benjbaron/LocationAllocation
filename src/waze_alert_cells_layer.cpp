//
// Created by Benjamin Baron on 20/10/2016.
//

#include "waze_alert_cells_layer.h"


QGraphicsItemGroup *WazeAlertCellsLayer::draw() {
    qDebug() << "Draw the waze cells";
    _groupItem = new QGraphicsItemGroup();
    _groupItem->setHandlesChildEvents(false);

    QHash<QPoint, WazeCellValue*>* wazeCells = _wazeAlertCells->getCells();

    for(auto it = wazeCells->begin(); it != wazeCells->end(); ++it) {
        WazeCellValue* wcv = it.value();
        wcv->color = _wazeAlertCells->selectColorGetisOrdG(_wazeAlertCells->computeGetisOrdG(it.key()));
        Geometry* geom = wcv->cell;
        GeometryGraphics* item;
        if(geom->getGeometryType() == CircleType) {
            item = new CircleGraphics(static_cast<Circle*>(geom));
            item->setZValue(10.0);
        } else if(geom->getGeometryType() == CellType) {
            item = new CellGraphics(static_cast<Cell*>(geom));
            item->setZValue(1.0);
        }

        item->setBrush(QBrush(wcv->color));
        item->setPen(Qt::NoPen);
        item->setOpacity(CELL_OPACITY);
        addGraphicsItem(item);
        _geometryGraphics.insert(geom, item);

        // add behavior on mouse press
        connect(item, &GeometryGraphics::mousePressedEvent, this, &WazeAlertCellsLayer::onWazeAlertCellSelected);
    }

    return _groupItem;

}

void WazeAlertCellsLayer::addMenuBar() {
    _menu = new QMenu();
    _menu->setTitle("Waze cells");
    _parent->addMenu(_menu);

    // add action to the menu to export a cellsfv file
    QAction* actionExportContour = _menu->addAction("Export cells file");
    connect(actionExportContour, &QAction::triggered, this, &WazeAlertCellsLayer::exportCellFile);
}

void WazeAlertCellsLayer::onWazeAlertCellSelected(Geometry* geom, bool mod) {
    WazeCellValue* wcv = _wazeAlertCells->getWazeCellValue(geom);
    qDebug() << "clicked on cell " << wcv->cell->toString() << " with " << wcv->alerts.size() << " alerts";

    _selectedGeometry = geom;

    if(_wazeAlertCellsPanel)
        _wazeAlertCellsPanel->onClosePanel();

    if(!_wazeAlertCellsPanel)
        _wazeAlertCellsPanel = new WazeAlertCellsPanel(_parent);

    if(_parent)
        _parent->addDockWidget(Qt::RightDockWidgetArea, _wazeAlertCellsPanel);

    _wazeAlertCellsPanel->show();
    _wazeAlertCellsPanel->showWazeAlertCellData(wcv);
}

void WazeAlertCellsLayer::exportCellFile() {
    qDebug() << "export cell file";
}