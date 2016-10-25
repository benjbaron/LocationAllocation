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
    virtual QGraphicsItemGroup* draw();
    const QList<QPointF> &get_points() const {
        return _points;
    }
    virtual bool load(Loader* loader);

private:
    QList<QPointF> _points;
};


#endif //LOCALL_POINTLAYER_H
