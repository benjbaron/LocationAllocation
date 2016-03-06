//
// Created by Benjamin Baron on 01/01/16.
//

#include "point_layer.h"
#include "geometries.h"
#include "loader.h"

QGraphicsItemGroup *PointLayer::draw() {
    int radius = 20;
    _groupItem = new QGraphicsItemGroup();
    QColor c = Qt::red;

    for(QPointF p : _points) {
        GeometryGraphics* item = new CircleGraphics(new Circle(p, radius));
        item->setPen(Qt::NoPen);
        item->setBrush(QBrush(c));

        addGraphicsItem(item);
//        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }
    return _groupItem;
}

bool PointLayer::load(Loader *loader) {
    loader->loadProgressChanged((qreal) 1.0, "Done");
    return false;
}
