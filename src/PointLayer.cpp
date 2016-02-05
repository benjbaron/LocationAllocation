//
// Created by Benjamin Baron on 01/01/16.
//

#include "PointLayer.h"
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
    }
    return _groupItem;
}

bool PointLayer::load(Loader *loader) {
    emit loader->loadProgressChanged((qreal) 1.0);
    return false;
}
