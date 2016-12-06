//
// Created by Benjamin Baron on 01/03/16.
//

#include "spatial_stats_layer.h"
#include "constants.h"
#include "allocation_dialog.h"
#include "polygon_layer.h"

bool SpatialStatsLayer::load(Loader* loader) {
    return _spatialStats->computeStats(loader);
}

QGraphicsItemGroup *SpatialStatsLayer::draw() {
    qDebug() << "Draw the spatial stats";
    _groupItem = new QGraphicsItemGroup();
    _groupItem->setHandlesChildEvents(false);

    QHash<Geometry*, QHash<Geometry*, GeometryMatrixValue*>* > matrix;
    _spatialStats->getGeometryMatrix(&matrix);

    for(auto it = matrix.begin(); it != matrix.end(); ++it) {
        Geometry* geom = it.key();
        GeometryGraphics* item = nullptr;
        if(geom->getGeometryType() == CircleGeometryType) {
            item = new CircleGraphics(static_cast<Circle*>(geom));
            item->setZValue(10.0);
        } else if(geom->getGeometryType() == CellGeometryType) {
            item = new CellGraphics(static_cast<Cell*>(geom));
            item->setZValue(1.0);
        }

        GeometryValue* val;
        _spatialStats->getValue(&val, it.key());
        item->setBrush(QBrush(val->color));
        item->setPen(Qt::NoPen);
        item->setOpacity(CELL_OPACITY);
        addGraphicsItem(item);
        _geometryGraphics.insert(it.key(), item);

        // add behavior on mouse press
        if(item != nullptr) {
            connect(item, &GeometryGraphics::mousePressedEvent, this, &SpatialStatsLayer::onMousePress);
        }
    }

    return _groupItem;
}


void SpatialStatsLayer::addMenuBar() {
    _menu = new QMenu();
    _menu->setTitle("Stat analysis");
    _parent->addMenu(_menu);

    // add action to compute the location allocation
    QAction* actionComputeAllocation = _menu->addAction("Compute allocation");
    connect(actionComputeAllocation, &QAction::triggered, this, &SpatialStatsLayer::computeAllocation);

    // add action to the menu to export a contour file
    QAction* actionExportContour = _menu->addAction("Export contour file");
    connect(actionExportContour, &QAction::triggered, this, &SpatialStatsLayer::exportContourFile);

    // add action to the menu to launch the REST server
    QAction* actionRestServer = _menu->addAction("Start REST server");
    connect(actionRestServer, &QAction::triggered, this, &SpatialStatsLayer::startRESTServer);

    QAction* actionShowDemandGrid = _menu->addAction("Show demand grid");
    connect(actionShowDemandGrid, &QAction::triggered, this, &SpatialStatsLayer::showDemandGrid);

    // TODO use method addBarMenuItems (see trace_layer.h)
    hideMenu();
}


void SpatialStatsLayer::exportContourFile() {
    if(!_selectedGeometry) {
        QMessageBox q(QMessageBox::Warning, "Error", "No ogrGeometry is selected", QMessageBox::Ok);
        q.exec(); // synchronous
        return;
    }

    // get output filename
    QString filename = QFileDialog::getSaveFileName(0,
                                                    tr("Export contour file"),
                                                    QString(),
                                                    tr("CSV file (*.csv)"));
    if(filename.isEmpty())
        return;

    // compute the contour plot
    qDebug() << "Export the contour plot file";
    QFile file(filename);
    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "Unable to write in file "<< filename;
        return;
    }

    QHash<Geometry*, GeometryValue*> geometries;
    _spatialStats->getGeometries(&geometries);
    Geometry* randomGeom = geometries.keys().first();
    double cellSize = randomGeom->getBounds().getQRectF().width();
    QPointF topLeft = randomGeom->getBounds().getTopLeft();
    QPointF bottomRight = randomGeom->getBounds().getBottomRight();
    for(Geometry* geom : geometries.keys()) {
        QPointF tl = geom->getBounds().getTopLeft();
        QPointF br = geom->getBounds().getBottomRight();
        if(tl.x() < topLeft.x())
            topLeft.setX(tl.x());
        if(tl.y() < topLeft.y())
            topLeft.setY(tl.y());
        if(br.x() > bottomRight.x())
            bottomRight.setX(br.x());
        if(br.y() > bottomRight.y())
            bottomRight.setY(br.y());
    }

    QTextStream out(&file);
    // first line (header)
    out << "x;y;z\n";
//    out << QString::number(_selectedGeometry->getCenter().x(), 'f', 4) << ";"
//        << QString::number(_selectedGeometry->getCenter().y(), 'f', 4) << ";"
//        << QString::number(1e10, 'f', 4) << "\n";

    qDebug() << topLeft << bottomRight << cellSize << (bottomRight.x() - topLeft.x()) / cellSize << (bottomRight.y() - topLeft.y()) / cellSize;
    QHash<Geometry*, GeometryMatrixValue*>* cells;
    _spatialStats->getValues(&cells, _selectedGeometry);
    for(double i = topLeft.x(); i < bottomRight.x(); i += 10) {
        for(double j = topLeft.y(); j < bottomRight.y(); j += 10) {
            QSet<Geometry*>* geoms = _spatialStats->getGeometriesAt(i,j);
            bool foundRightGeom = false;
            if(geoms->isEmpty()) {
                return;
            }
            for(Geometry* geom : *geoms) {
                if(geom->getGeometryType() == CellGeometryType) {
                    // add the ogrGeometry to the output
                    if(cells->contains(geom)) {
                        GeometryMatrixValue* val = cells->value(geom);
//                            double x = geom->getCenter().x();
//                            double y = geom->getCenter().y();
                        double z = 10 * val->avgScore;
                        out << QString::number(i, 'f', 0) << ";"
                        << QString::number(j, 'f', 0) << ";"
                        << QString::number(z, 'f', 2) << "\n";
                        foundRightGeom = true;
                        break;
                    }
                }
            }
            if(!foundRightGeom) {
                // add the generic output
                out << QString::number(i, 'f', 0) << ";"
                << QString::number(j, 'f', 0) << ";"
                << QString::number(0, 'f', 2) << "\n";
            }
        }
    }
//    for(Geometry* geom : cells->keys()) {
//        GeometryMatrixValue* val = cells->value(geom);
//        double x = geom->getCenter().x();
//        double y = geom->getCenter().y();
//        double z = 100.0 * val->avgScore;
//
//        out << QString::number(x, 'f', 0) << ";"
//            << QString::number(y, 'f', 0) << ";"
//            << QString::number(z, 'f', 2) << "\n";
//    }

    file.close();

    qDebug() << "[DONE] export contour file";
}

void SpatialStatsLayer::startRESTServer() {
    qDebug() << "Launch REST server";
    if(!_restServer) {

        // create an instance of the compute allocation
        ComputeAllocation* computeAllocation = new ComputeAllocation(_spatialStats);

        // start the server
        _restServer = new RESTServer(10, 0, computeAllocation);
        _restServer->setTimeOut(500);
        _restServer->listen(8080);
        qDebug() << "REST Server listening on port 8080";
    }
}

void SpatialStatsLayer::showDemandGrid() {
    qDebug() << "Show demand grid";

    QHash<Geometry *, QHash<Geometry *, GeometryMatrixValue *> *> matrix;
    _spatialStats->getGeometryMatrix(&matrix);

    double cellSize = _spatialStats->getCellSize();
    QString name = "Demand cells grid";

    /* get the top left (xmin,ymin) and bottom right (xmax,ymax) cells */
    double xmin = 1e10, xmax = 0.0, ymin = 1e10, ymax = 0.0;
    for (auto it = matrix.begin(); it != matrix.end(); ++it) {
        Geometry *geom = it.key();
        if (geom->getGeometryType() == CellGeometryType) {
            if(geom->getBounds().getTopLeft().x() < xmin)
                xmin = geom->getBounds().getTopLeft().x();
            if(geom->getBounds().getBottomRight().x() > xmax)
                xmax = geom->getBounds().getBottomRight().x();
            if(geom->getBounds().getTopLeft().y() < ymin)
                ymin = geom->getBounds().getTopLeft().y();
            if(geom->getBounds().getBottomRight().y() > ymax)
                ymax = geom->getBounds().getBottomRight().y();
        }
    }

    QList<QList<QPointF>*> cells;
    /* fill the space with cells */
    for(double x = xmin; x < xmax; x += cellSize) {
        for(double y = ymin; y < ymax; y += cellSize) {
            QList<QPointF>* points = new QList<QPointF>();
            *points << QPointF(x,-1*y)
                    << QPointF(x+cellSize,-1*y)
                    << QPointF(x+cellSize,-1*(y+cellSize))
                    << QPointF(x,-1*(y+cellSize))
                    << QPointF(x,-1*y);
            qDebug() << QPointF(x,y) << QPointF(x+cellSize,y+cellSize);
            cells.append(points);
        }
    }

    qDebug() << cells.size() << "cells in the grid" << xmin << ymin << xmax << ymax;
    PolygonLayer* layer = new PolygonLayer(_parent, name, cells);
    Loader loader;
    _parent->createLayer(name, layer, &loader);
}

void SpatialStatsLayer::onMousePress(Geometry* geom, bool mod) {

    if(!_spatialStats->hasValue(geom) || !_spatialStats->hasMatrixValue(geom))
        return;

    qDebug() << "Clicked on ogrGeometry" << geom->toString() << mod;
    // select all the reachable geometries
    // restore the parameters for the previously selected ogrGeometry
    if(_selectedGeometry && mod) {
        // select the link
        if(!_spatialStats->hasMatrixValue(_selectedGeometry, geom))
            return;

        if(!_plots) {
            // docks the widget on the mainwindow
            _plots = new DockWidgetPlots(_parent, _spatialStats);
        }
        if(_parent)
            _parent->addDockWidget(Qt::RightDockWidgetArea, _plots);
        _plots->show();
        _plots->showLinkData(_selectedGeometry, geom);

    } else {
        // select the ogrGeometry
        if(_selectedGeometry) {
            // restore the "normal" opacity
            _geometryGraphics[_selectedGeometry]->setOpacity(CELL_OPACITY);
            _pointsGroups[_selectedGeometry]->setVisible(false);

            // restore the "normal" colors for the neighbor geometries
            QHash<Geometry*, GeometryMatrixValue*>* geoms;
            _spatialStats->getValues(&geoms, _selectedGeometry);
            for(auto jt = geoms->begin(); jt != geoms->end(); ++jt) {
                Geometry* g = jt.key();
                if(_geometryGraphics.contains(g)) {
                    GeometryValue* val;
                    _spatialStats->getValue(&val,g);
                    _geometryGraphics[g]->setBrush(QBrush(val->color));
                    _geometryGraphics[g]->update();
                }
            }

            if(_selectedGeometry == geom) {
                GeometryValue* val;
                _spatialStats->getValue(&val, _selectedGeometry);
                _geometryGraphics[_selectedGeometry]->setBrush(val->color);
                _geometryGraphics[_selectedGeometry]->update();

                _selectedGeometry = NULL;
                qDebug() << "clicked on same ogrGeometry";
                return;
            }
        }

        QHash<Geometry*, GeometryMatrixValue*>* geoms;
        _spatialStats->getValues(&geoms, geom);
        double maxWeight = 0.0;
        for(auto jt = geoms->begin(); jt != geoms->end(); ++jt) {
            if(jt.key() == geom) continue;
            double score = jt.value()->avgScore;
            if(score > maxWeight)
                maxWeight = score;
        }
        _geometryGraphics[geom]->setOpacity(1.0);
        _geometryGraphics[geom]->setBrush(BLUE);
        _geometryGraphics[geom]->update();
        QGraphicsItemGroup* group = new QGraphicsItemGroup();
        for(auto jt = geoms->begin(); jt != geoms->end(); ++jt) {
            Geometry* g = jt.key();
            if(g == geom) continue;
            auto val = jt.value();
            double score = val->avgScore;
            int scoreClass = (score == maxWeight) ? 4 : (int) (5.0 * (score / maxWeight));
            int factor = 50 + 100 * scoreClass;
//                    qDebug() << score << maxWeight << factor;

            if(_geometryGraphics.contains(g)) {
                _geometryGraphics[g]->setBrush(BLUE.darker(factor));
                _geometryGraphics[g]->update();
            }

            // add the points to the group
            QGraphicsEllipseItem* i = new CircleGraphics(new Circle(g->getCenter(), 10));
            i->setBrush(QBrush(Qt::black));
            i->setPen(Qt::NoPen);
            QGraphicsLineItem* line = new QGraphicsLineItem(geom->getCenter().x(), -1*geom->getCenter().y(),
                                                            g->getCenter().x(), -1*g->getCenter().y());
            line->setPen(QPen(QBrush(Qt::black),2.0));
            group->addToGroup(i);
            group->addToGroup(line);
        }
        _pointsGroups.insert(geom, group);
        addGraphicsItem(group);

        if(!_plots) {
            _plots = new DockWidgetPlots(_parent, _spatialStats); // see to dock the widget on the mainwindow
        }
        if(_parent)
            _parent->addDockWidget(Qt::RightDockWidgetArea, _plots);
        _plots->show();
        _plots->showNodeData(geom);

        _selectedGeometry = geom;
    }
}

void SpatialStatsLayer::computeAllocation() {
    if(!_computeAllocation) {
        // Initialize a new compute allocation layer
        _computeAllocation = new ComputeAllocation(_spatialStats);
    }

    AllocationDialog diag(_parent);
    if (diag.exec() == QDialog::Rejected) {
        return;
    }

    QHash<Geometry*, Allocation*> allocation; // resulting allocation
    AllocationParams params;
    diag.allocationParams(&params);

    loadWithProgressDialog(_computeAllocation, &ComputeAllocation::processAllocationMethod, &params, &allocation);

    // print the resulting allocation
    for(auto it = allocation.begin(); it != allocation.end(); ++it) {
        qDebug() << "candidate" << it.value()->rank
                 << "allocated" << it.value()->weight << "(demand weight)"
                 << it.value()->backendWeight << "(backend weight)" << it.value()->backends.size()
                 << it.value()->incomingWeight << "(incoming weight)"
                 << "with" << it.value()->demands.size() << "demands"
                 << "and" << it.value()->deletedCandidates.size() << "candidates deleted" ;
    }

    if(!params.computeAllStorageNodes.isEmpty()) {
        return; // do not create a layer
    }

    // create a new layer
    QString layerName = "Location allocation";
    ComputeAllocationLayer* allocationLayer = new ComputeAllocationLayer(_parent, layerName, allocation, _spatialStats);

    // load the layer
    Loader loader;
    _parent->createLayer(layerName, allocationLayer, &loader);
}
