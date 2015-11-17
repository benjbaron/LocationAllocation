#include "weightedallocationlayer.h"

QGraphicsItemGroup *WeightedAllocationLayer::draw()
{
    int radius = 20;
    _groupItem = new QGraphicsItemGroup();

    for(auto it = _points.begin(); it != _points.end(); ++it) {
        QPoint p = it.key();
        QRectF cell(p.x()*_cellSize, p.y()*_cellSize, _cellSize, _cellSize);
        QPointF center = cell.center();

        //!\\ TODO Size of the point as a function of the weight assigned to it

//            qDebug() << "node" << it.key() << "(" << jt.key() << "," << x << "," << y << ")";
        QGraphicsEllipseItem* item = new QGraphicsEllipseItem(center.x()-radius, center.y()-radius, radius*2, radius*2);
//            item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

        item->setBrush(QBrush(Qt::red));
        item->setPen(Qt::NoPen);

        addGraphicsItem(item);
    }

    return _groupItem;
}
