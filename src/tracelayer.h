#ifndef TRACELAYER_H
#define TRACELAYER_H

#include "layer.h"

class TraceLayer: public Layer
{
public:
    TraceLayer(MainWindow* parent = 0, QString name = 0):
        Layer(parent, name) { }

    void addPoint(QString node, long long ts, double lat, double lon) {
        if(!_nodes.contains(node)) {
            // create the corresponding Map of successive posiitons
            _nodes.insert(node, new QMap<long long, QPointF>());
        }
        // update the node position
        _nodes.value(node)->insert(ts, QPoint(lat, lon));
    }

    QGraphicsItemGroup* draw();

    QList<std::tuple<QPointF,double,double>> getPoints(int weight = 0, long long startTime = 0, long long endTime = 0);

    OGRGeometryCollection *getGeometry(long long startTime = 0, long long endTime = 0);

    QHash<QString, QMap<long long, QPointF>*> getNodes() const { return _nodes; }
    QMap<long long, QPointF>* const getNodeTrace(QString node) { return _nodes.value(node); }

    void exportLayer(QString output);
    void exportLayerONE(QString output);
    void exportLayerText(QString output, long long duration = 86400);
    void exportLayerGrid(QString output, int cellSize = 200, long long duration = 86400);

private:
    QHash<QString, QMap<long long, QPointF>*> _nodes;

};

#endif // TRACELAYER_H
