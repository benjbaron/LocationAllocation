#include "grid_layer.h"

#include <QDebug>
#include "loader.h"

GridLayer::GridLayer(MainWindow* parent, QString name, int cellSize):
    Layer(parent, name), _geometriesize(cellSize) { }

QGraphicsItemGroup *GridLayer::draw() {
    _groupItem = new QGraphicsItemGroup();

    for(int i = 0; i < _geometries.size(); ++i) {
        QPointF p(_geometries.at(i));
        qreal x = _topLeft.x() + p.x()*_geometriesize;
        qreal y = _topLeft.y() + p.y()*_geometriesize;
        qreal w = _geometriesize;
        qreal h = _geometriesize;
        QGraphicsRectItem* item = new QGraphicsRectItem(x,-1*y,w,-1*h);
        item->setBrush(QBrush(QColor(255,0,0)));
        item->setOpacity(0.6);
        item->setPen(QPen(Qt::black));

        addGraphicsItem(item);
    }

    return _groupItem;
}

bool GridLayer::load(Loader *loader) {
    // get the grid from the scene
    qDebug() << _parent->getSceneRect()
             << _parent->getSceneRect().left()
             << _parent->getSceneRect().right()
             << _parent->getSceneRect().bottomLeft()
             << _parent->getSceneRect().bottom()
             << _parent->getSceneRect().top();

    _topLeft = _parent->getSceneRect().topLeft();
    double xCursor = _parent->getSceneRect().left();
    int count = 0;
    int size = (int) (_parent->getSceneRect().right() * _parent->getSceneRect().top());
    for(int i = 0; xCursor < _parent->getSceneRect().right(); ++i) {
        double yCursor = _parent->getSceneRect().top();
        for(int j = 0; yCursor < _parent->getSceneRect().bottom(); ++j) {
            _geometries.append(QPointF(i,j));
            yCursor += _geometriesize;
            loader->loadProgressChanged((qreal) count++ / (qreal) size, "");
        }
        xCursor += _geometriesize;
    }
    qDebug() << "cells size" << _geometries.size();

    loader->loadProgressChanged((qreal) 1.0, "Done");
    return true;
}
