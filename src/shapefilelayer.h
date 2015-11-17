#ifndef SHAPEFILELAYER_H
#define SHAPEFILELAYER_H

#include "layer.h"
#include <ogrsf_frmts.h>

class ShapefileLayer: public Layer
{
public:
    ShapefileLayer(MainWindow* parent = 0, QString name = 0):
        Layer(parent, name) {}

    void addGeometry(OGRGeometry* geom) {
        _geometryItems.append(geom);
    }
    int countGeomerties() { return _geometryItems.size(); }


    QGraphicsItemGroup* draw();
    QList<std::tuple<QPointF,double,double>> getPoints(int deadline = 0, long long startTime = 0, long long endTime = 0);

private:
    QList<OGRGeometry*> _geometryItems;

};

#endif // SHAPEFILELAYER_H
