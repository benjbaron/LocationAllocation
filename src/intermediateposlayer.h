#ifndef INTERMEDIATEPOSLAYER_H
#define INTERMEDIATEPOSLAYER_H

#include "layer.h"
#include "tracelayer.h"
#include <qmath.h>

class IntermediatePosLayer: public Layer
{
public:
    IntermediatePosLayer(MainWindow* parent = 0, QString name = 0, TraceLayer* traceLayer = 0):
        Layer(parent, name), _traceLayer(traceLayer) {
        populateNodes();
    }

    QGraphicsItemGroup* draw() {
        int radius;
        QColor col;
        _groupItem = new QGraphicsItemGroup();

        for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
            for(auto jt = it.value()->begin(); jt != it.value()->end(); ++jt) {
                QPointF pos = jt.value().first;
                bool isWaypoint = jt.value().second;
                int x = pos.x();
                int y = pos.y();

    //            qDebug() << "node" << it.key() << "(" << jt.key() << "," << x << "," << y << ")";
                if(isWaypoint) {
                    radius = 5;
                    col = Qt::red;
                } else {
                    radius = 2;
                    col = Qt::black;
                }

                QGraphicsEllipseItem* item = new QGraphicsEllipseItem(x-radius, y-radius, radius*2, radius*2);
    //            item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

                item->setBrush(QBrush(col));
                item->setPen(Qt::NoPen);

                addGraphicsItem(item);
            }
        }

        return _groupItem;
    }

    QList<std::tuple<QPointF,double,double>> getPoints(int deadline = 0, long long startTime = 0, long long endTime = 0) {
        return QList<std::tuple<QPointF,double,double>>();
    }

    void populateNodes() {
        auto nodes = _traceLayer->getNodes();
        for(auto it = nodes.begin(); it != nodes.end(); ++it) {
            QString nodeId = it.key();
            auto trace = it.value();
            for(auto jt = trace->begin(); jt != trace->end(); ++jt) {
                long long time = jt.key();
                QPointF pos = jt.value();

                if(!_prevPos.isNull() && time - _prevTime <= 300) {
                    int nbPos = (int) qCeil((time - _prevTime) / _sampling);
                    for(int i = 1; i < nbPos; ++i) {
                        long long t = _prevTime + i*_sampling;
                        QPointF p = (time - t)*_prevPos + (t - _prevTime)*pos;
                        p /= (time - _prevTime);
                        if(!_nodes.contains(nodeId))
                            _nodes.insert(nodeId, new QMap<long long, QPair<QPointF, bool>>());
                        _nodes.value(nodeId)->insert(t, qMakePair(p,false));
                    }
                }
                if(!_nodes.contains(nodeId))
                    _nodes.insert(nodeId, new QMap<long long, QPair<QPointF, bool>>());
                _nodes.value(nodeId)->insert(time, qMakePair(pos,true));

                _prevPos = pos;
                _prevTime = time;
            }
        }
    }

private:
    TraceLayer* _traceLayer;
    QHash<QString, QMap<long long, QPair<QPointF, bool>>*> _nodes;
    int _sampling = 1;
    long long _prevTime = 0;
    QPointF _prevPos = QPointF();
};

#endif // INTERMEDIATEPOSLAYER_H
