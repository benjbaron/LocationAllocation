//
// Created by Benjamin Baron on 25/11/2016.
//

#ifndef LOCALL_GEOMETRY_LAYER_H
#define LOCALL_GEOMETRY_LAYER_H


#include "layer.h"

class GeometryLayer : public Layer {
    Q_OBJECT
public:
    GeometryLayer(MainWindow* parent = 0, QString name = 0, const QList<Geometry*>& geometries = QList<Geometry*>()):
            Layer(parent, name), _geometries(geometries) {}

    QGraphicsItemGroup* draw();
    bool load(Loader* loader);

private:
    QList<Geometry*> _geometries;

};


#endif //LOCALL_GEOMETRY_LAYER_H
