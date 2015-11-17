#include "gridlayer.h"

#include <QDebug>

GridLayer::GridLayer(MainWindow* parent, QString name, int cellSize):
    Layer(parent, name), _cellSize(cellSize)
{
    // get the grid from the scene
    qDebug() << parent->getSceneRect() << parent->getSceneRect().left() << parent->getSceneRect().right() << parent->getSceneRect().bottomLeft() << parent->getSceneRect().bottom() << parent->getSceneRect().top();
    _topLeft = parent->getSceneRect().topLeft();
    double xCursor = parent->getSceneRect().left();
    for(int i = 0; xCursor < parent->getSceneRect().right(); ++i) {
        double yCursor = parent->getSceneRect().top();
        for(int j = 0; yCursor < parent->getSceneRect().bottom(); ++j) {
            _cells.append(QPointF(i,j));
            yCursor += _cellSize;
        }
        xCursor += _cellSize;
    }
    qDebug() << "cells size" << _cells.size();
}

QGraphicsItemGroup *GridLayer::draw()
{
    _groupItem = new QGraphicsItemGroup();

    for(int i = 0; i < _cells.size(); ++i) {
        QPointF p(_cells.at(i));
        qreal x = _topLeft.x() + p.x()*_cellSize;
        qreal y = _topLeft.y() + p.y()*_cellSize;
        qreal w = _cellSize;
        qreal h = _cellSize;
        QGraphicsRectItem* item = new QGraphicsRectItem(x,y,w,h);
        item->setBrush(QBrush(QColor(255,0,0)));
        item->setOpacity(0.6);
        item->setPen(QPen(Qt::black));

        addGraphicsItem(item);
    }

    return _groupItem;
}

QList<std::tuple<QPointF, double, double> > GridLayer::getPoints(int deadline, long long startTime, long long endTime)
{
    Q_UNUSED(deadline)
    Q_UNUSED(startTime)
    Q_UNUSED(endTime)

    QList<std::tuple<QPointF,double,double>> res;
    for(int i = 0; i < _cells.size(); ++i) {
        double x = _topLeft.x() + _cells.at(i).x()*_cellSize;
        double y = _topLeft.y() + _cells.at(i).y()*_cellSize;
        QRectF cell(QPoint(x,y), QPoint(x+_cellSize, y+_cellSize));
        auto t = std::make_tuple(cell.center(), 1.0, -1.0);
        res.append(t);
    }

    return res;
}

OGRGeometryCollection *GridLayer::getGeometry(long long startTime, long long endTime) {
    OGRGeometryCollection* collection = new OGRGeometryCollection();
    for(int i = 0; i < _cells.size(); ++i) {
        OGRLinearRing  oRing;
        OGRPolygon oPoly;

        double x = _topLeft.x() + _cells.at(i).x()*_cellSize;
        double y = _topLeft.y() + _cells.at(i).y()*_cellSize;
        QRectF cell(QPoint(x,y), QPoint(x+_cellSize, y+_cellSize));

        oRing.addPoint(cell.bottomLeft().x(),  cell.bottomLeft().y());
        oRing.addPoint(cell.bottomRight().x(), cell.bottomRight().y());
        oRing.addPoint(cell.topRight().x(),    cell.topRight().y());
        oRing.addPoint(cell.topLeft().x(),     cell.topLeft().y());
        oRing.addPoint(cell.bottomLeft().x(),  cell.bottomLeft().y());

        oPoly.addRing( &oRing );
        collection->addGeometry( &oPoly );
    }

    return collection;
}
