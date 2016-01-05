#ifndef GRIDLAYER_H
#define GRIDLAYER_H

#include "layer.h"

class GridLayer: public Layer
{
public:
    GridLayer(MainWindow* parent = 0, QString name = 0, int cellSize = 100);

    QGraphicsItemGroup* draw();
    QList<std::tuple<QPointF,double,double>> getPoints(int deadline = 0, long long startTime = 0, long long endTime = 0);
    OGRGeometryCollection* getGeometry(long long startTime = 0, long long endTime = 0);

private:
    int _geometriesize;
    QPointF _topLeft;
    QList<QPointF> _geometries;

};

#endif // GRIDLAYER_H
