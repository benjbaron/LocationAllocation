//
// Created by Benjamin Baron on 10/03/16.
//

#include "trace_inspector_layer.h"
#include "constants.h"

QGraphicsItemGroup *TraceInspectorLayer::draw() {
    _groupItem = new QGraphicsItemGroup();
    _groupItem->setHandlesChildEvents(false);

    QColor c = Qt::gray;
    QPen pen = QPen(c);
    pen.setWidth(3);

    QMap<long long, QPointF> nodeTrace;
    _trace->getNodeTrace(&nodeTrace, _nodeId);

    QPainterPath path;
    QPointF pt = QPointF(nodeTrace.first().x(), -1*nodeTrace.first().y());
    path.moveTo(pt);
    for(auto it = nodeTrace.begin()+1; it != nodeTrace.end(); ++it) {
        pt = it.value();
        path.lineTo(QPointF(pt.x(), -1*pt.y()));
//        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }
    QGraphicsPathItem* item = new QGraphicsPathItem(path);
    item->setPen(pen);
//    item->setZValue(1.0);
    addGraphicsItem(item);

    for(auto it = nodeTrace.begin(); it != nodeTrace.end(); ++it) {
        QPointF pt = it.value();
        GraphicsWaypoint* wp = new GraphicsWaypoint(pt, QString::number(it.key()), 10);
        connect(wp, &GraphicsWaypoint::mousehoverEnterEvent, [=](QPointF id) {
            if(_selectedPoint != id) {
                _pointsGraphics.value(id)->setBrush(QBrush(BLUE));
                _pointsGraphics.value(id)->toggleLabel(true);
                _hoveredPoint = pt;
            }
        });
        connect(wp, &GraphicsWaypoint::mousehoverLeaveEvent, [=](QPointF id) {
            if(_selectedPoint != id) {
                _pointsGraphics.value(id)->setBrush(QBrush(RED));
                _pointsGraphics.value(id)->toggleLabel(false);
                _hoveredPoint = QPointF();
            }
        });
        connect(wp, &GraphicsWaypoint::mousePressedEvent, [=](QPointF id) {
            if(!_selectedPoint.isNull()) {
                _pointsGraphics.value(_selectedPoint)->setOpacity(0.5);
                _pointsGraphics.value(_selectedPoint)->setBrush(QBrush(RED));
                _pointsGraphics.value(_selectedPoint)->toggleLabel(false);
            }

            if(id != _selectedPoint) {
                _pointsGraphics.value(id)->setBrush(QBrush(BLUE));
                _pointsGraphics.value(id)->setOpacity(1.0);
                _pointsGraphics.value(id)->toggleLabel(true);
                _selectedPoint = id;
            } else {
                _selectedPoint = QPointF();
            }
        });

        wp->setBrush(QBrush(RED));
        wp->setPen(Qt::NoPen);
//        wp->setZValue(10.0);

        _pointsGraphics.insert(pt, wp);
        addGraphicsItem(wp);
    }

    return _groupItem;
}

QGraphicsItemGroup *TraceSamplingInspectorLayer::draw() {
    _groupItem = new QGraphicsItemGroup();
    _groupItem->setHandlesChildEvents(false);

    QColor c = Qt::gray;
    QPen pen = QPen(c);
    pen.setWidth(3);

    QMap<long long, QPointF> nodeTrace;
    _trace->getNodeTrace(&nodeTrace, _nodeId);

    /* draw the path between the points */
    QPainterPath pathPoints;
    QPointF pt = _pointsList.first()->point;
    pathPoints.moveTo(pt);
    for (int i = 1; i < _pointsList.size(); ++i) {
        pt = _pointsList.at(i)->point;
        pathPoints.lineTo(QPointF(pt.x(), -1 * pt.y()));
//        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }
    QGraphicsPathItem* item = new QGraphicsPathItem(pathPoints);
    pen.setColor(ORANGE);
    item->setPen(pen);
//    item->setZValue(1.0);
    addGraphicsItem(item);


    /* draw the trace with the nodes */
    QPainterPath pathTrace;
    pt = QPointF(nodeTrace.first().x(), -1 * nodeTrace.first().y());
    pathTrace.moveTo(pt);
    _tracePointsGraphics.insert(pt, new GraphicsWaypoint(pt, QString::number(nodeTrace.firstKey()), 10));
    for (auto it = nodeTrace.begin() + 1; it != nodeTrace.end(); ++it) {
        pt = it.value();
        pathTrace.lineTo(QPointF(pt.x(), -1 * pt.y()));
        GraphicsWaypoint* wp = new GraphicsWaypoint(pt, QString::number(it.key()), 10);
        wp->setBrush(QBrush(Qt::black));
        wp->setOpacity(0.5);
        _tracePointsGraphics.insert(pt, wp);
        addGraphicsItem(wp);
//        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }
    item = new QGraphicsPathItem(pathTrace);
    item->setPen(pen);
    item->setZValue(1.0);
    addGraphicsItem(item);


    /* Add the points */
    for (PointSampling *point : _pointsList) {
        QPointF pt = point->point;
        _points.insert(pt, point);
        GraphicsWaypoint* wp = new GraphicsWaypoint(pt, QString::number(point->time), 10);
        connect(wp, &GraphicsWaypoint::mousehoverEnterEvent, [=](QPointF id) {
            if (_selectedPoint != id) {
                _pointsGraphics.value(id)->setBrush(QBrush(BLUE));
                _pointsGraphics.value(id)->toggleLabel(true);
            }
        });
        connect(wp, &GraphicsWaypoint::mousehoverLeaveEvent, [=](QPointF id) {
            if (_selectedPoint != id) {
                _pointsGraphics.value(id)->setBrush(QBrush(RED));
                _pointsGraphics.value(id)->toggleLabel(false);
            }
        });
        connect(wp, &GraphicsWaypoint::mousePressedEvent, [=](QPointF id) {
            if (!_selectedPoint.isNull()) {
                _pointsGraphics.value(_selectedPoint)->setOpacity(0.5);
                _pointsGraphics.value(_selectedPoint)->setBrush(QBrush(RED));
                _pointsGraphics.value(_selectedPoint)->toggleLabel(false);

                QPointF low = _points.value(_selectedPoint)->low;
                QPointF up  = _points.value(_selectedPoint)->up;

                _tracePointsGraphics.value(low)->setOpacity(0.5);
                _tracePointsGraphics.value(low)->toggleLabel(false);

                _tracePointsGraphics.value(up)->setOpacity(0.5);
                _tracePointsGraphics.value(up)->toggleLabel(false);
            }

            if (id != _selectedPoint) {
                _pointsGraphics.value(id)->setBrush(QBrush(BLUE));
                _pointsGraphics.value(id)->setOpacity(1.0);
                _pointsGraphics.value(id)->toggleLabel(true);

                QPointF low = _points.value(id)->low;
                QPointF up  = _points.value(id)->up;

                _tracePointsGraphics.value(low)->setOpacity(0.75);
                _tracePointsGraphics.value(low)->toggleLabel(true);

                _tracePointsGraphics.value(up)->setOpacity(0.75);
                _tracePointsGraphics.value(up)->toggleLabel(true);

                _selectedPoint = id;
            } else {
                _selectedPoint = QPointF();
            }
        });

        wp->setBrush(QBrush(RED));
        wp->setPen(Qt::NoPen);
//        wp->setZValue(10.0);

        _pointsGraphics.insert(pt, wp);
        addGraphicsItem(wp);
    }

    return _groupItem;
}