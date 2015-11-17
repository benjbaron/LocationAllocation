#ifndef WEIGHTEDALLOCATIONLAYER_H
#define WEIGHTEDALLOCATIONLAYER_H

#include "layer.h"

class WeightedAllocationLayer: public Layer
{
public:
    WeightedAllocationLayer(MainWindow* parent = 0, QString name = 0, const QHash<QPoint, QPair<double, QSet<QPoint>>>& points = QHash<QPoint, QPair<double, QSet<QPoint>>>(), int cellSize = 50):
        Layer(parent, name), _points(points), _cellSize(cellSize) { }
    QGraphicsItemGroup* draw();
    QList<std::tuple<QPointF,double,double>> getPoints(int weight = 0, long long startTime = 0, long long endTime = 0) {}

    //!\\ TODO functionality to export the allocated points

private:
    QHash<QPoint, QPair<double, QSet<QPoint>>> _points;
    int _cellSize = 50; // 400 x 400 square meters
};

#endif // WEIGHTEDALLOCATIONLAYER_H
