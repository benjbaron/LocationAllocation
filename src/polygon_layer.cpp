//
// Created by Benjamin Baron on 17/05/16.
//

#include "polygon_layer.h"

QGraphicsItemGroup* PolygonLayer::draw() {
    _groupItem = new QGraphicsItemGroup();
    QColor c = Qt::red;
    QPen pen = QPen(c);
    pen.setWidth(3);

    for(QList<QPointF>* p : _polygons) {
        QPolygonF polygon(QVector<QPointF>::fromList(*p));
        QGraphicsPolygonItem* item = new QGraphicsPolygonItem(polygon);
        item->setPen(pen);
        item->setBrush(QBrush(Qt::red));
        item->setOpacity(0.5);
        addGraphicsItem(item);
    }

    return _groupItem;
}

bool PolygonLayer::load(Loader *loader) {
    loader->loadProgressChanged((qreal) 1.0, "Done");
    return true;
}
