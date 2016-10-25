//
// Created by Benjamin Baron on 08/02/16.

#include "linestring_layer.h"
#include "geometries.h"
#include "loader.h"

QGraphicsItemGroup* LineStringLayer::draw() {
    _groupItem = new QGraphicsItemGroup();
    QColor c = Qt::red;
    QPen pen = QPen(c);

    for(LineStringDisplay* lsd : _linestrings) {
        QList<QPointF> ls = lsd->_linestring;
        QPainterPath path;
        path.moveTo(QPointF(ls.first().x(), -1*ls.first().y()));
        for(int i = 1; i < ls.size(); ++i) {
            QPointF pt = ls.at(i);
            path.lineTo(QPointF(pt.x(), -1*pt.y()));
        }
        QGraphicsPathItem* item = new QGraphicsPathItem(path);
        pen.setWidth(lsd->_width);
        item->setPen(pen);
        addGraphicsItem(item);
//        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }
    return _groupItem;
}

bool LineStringLayer::load(Loader *loader) {
    loader->loadProgressChanged((qreal) 1.0, "Done");
    return true;
}
