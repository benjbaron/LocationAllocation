//
// Created by Benjamin Baron on 17/05/16.
//

#ifndef LOCALL_POLYGON_LAYER_H
#define LOCALL_POLYGON_LAYER_H

#include "layer.h"

class PolygonLayer: public Layer {
public:
    PolygonLayer(MainWindow* parent = 0, QString name = 0, const QList<QList<QPointF>*>& polygons = QList<QList<QPointF>*>()):
            Layer(parent, name), _polygons(polygons) {}
    virtual QGraphicsItemGroup* draw();
    const QList<QList<QPointF>*> &get_polygons() const {
        return _polygons;
    }
    virtual bool load(Loader* loader);

private:
    QList<QList<QPointF>*> _polygons;
};


#endif //LOCALL_POLYGON_LAYER_H
