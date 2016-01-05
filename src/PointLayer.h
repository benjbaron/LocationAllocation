//
// Created by Benjamin Baron on 01/01/16.
//

#ifndef LOCALL_POINTLAYER_H
#define LOCALL_POINTLAYER_H

#include "layer.h"

class PointLayer: public Layer {
public:
    PointLayer(MainWindow* parent = 0, QString name = 0, const QList<QPointF>& points = QList<QPointF>()):
            Layer(parent, name), _points(points) {}
    QGraphicsItemGroup* draw();
    QList<std::tuple<QPointF,double,double>> getPoints(int deadline = 0, long long startTime = 0, long long endTime = 0) {};
    const QList<QPointF> &get_points() const {
        return _points;
    }

private:
    QList<QPointF> _points;
};


#endif //LOCALL_POINTLAYER_H
