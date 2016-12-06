//
// Created by Benjamin Baron on 25/11/2016.
//

#include "geometry_layer.h"

QGraphicsItemGroup* GeometryLayer::draw() {
    QPen pen = QPen(SHAPEFILE_COL);
    pen.setWidth(SHAPEFILE_WID);
    pen.setCosmetic(true);

    _groupItem = new QGraphicsItemGroup();

    for(Geometry* geometry : _geometries) {
        GeometryGraphics* item = nullptr;

        if(geometry->getGeometryType() == GeometryType::PathGeometryType) {
            item = new ArrowPathGraphics(static_cast<Path*>(geometry));
            pen.setColor(SHAPEFILE_COL);
            item->setPen(pen);
            addGraphicsItem(item);

        } else if(geometry->getGeometryType() == GeometryType::CircleGeometryType) {
            item = new CircleGraphics(static_cast<Circle*>(geometry));
            item->setBrush(QBrush(SHAPEFILE_COL));
            item->setPen(Qt::NoPen);
            addGraphicsItem(item);

        } else if(geometry->getGeometryType() == GeometryType::PolygonGeometryType) {
            item = new PolygonGraphics(static_cast<Polygon*>(geometry));
            pen.setColor(SHAPEFILE_COL);
            item->setPen(pen);
            addGraphicsItem(item);
        } else if(geometry->getGeometryType() == GeometryType::CellGeometryType) {
            item = new CellGraphics(static_cast<Cell*>(geometry));
            pen.setColor(SHAPEFILE_COL);
            item->setPen(pen);
            addGraphicsItem(item);

        } else {
            qDebug() << "geometry" << geometry->toString() << "not represented";
        }
    }

    return _groupItem;
}

bool GeometryLayer::load(Loader *loader) {
    loader->loadProgressChanged((qreal) 1.0, "Done");
    return false;
}