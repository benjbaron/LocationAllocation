#ifndef GRIDLAYER_H
#define GRIDLAYER_H

#include "layer.h"

class GridLayer: public Layer {
public:
    GridLayer(MainWindow* parent = 0, QString name = 0, int cellSize = 100);

    QGraphicsItemGroup* draw();

    virtual bool load(Loader* loader);

private:
    int _geometriesize;
    QPointF _topLeft;
    QList<QPointF> _geometries;

};

#endif // GRIDLAYER_H
