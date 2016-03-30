//
// Created by Benjamin Baron on 01/03/16.
//

#ifndef LOCALL_COMPUTEALLOCATIONLAYER_H
#define LOCALL_COMPUTEALLOCATIONLAYER_H

#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>

// forward class declarations
class Loader;

class GraphicsPoint: public QObject, public QGraphicsEllipseItem {
    Q_OBJECT
public:
    GraphicsPoint():
            QGraphicsEllipseItem(), _id(QPointF()) {}
    GraphicsPoint(qreal x, qreal y, qreal w, qreal h, QPointF id):
            QGraphicsEllipseItem(x, -1*y, w, -1*h), _id(id) {}
    GraphicsPoint(QPointF p, int radius, QPointF id):
            QGraphicsEllipseItem(p.x()-radius, -1*(p.y()+radius), 2*radius, (2*radius)), _id(id) {}

    signals:
    void mousePressedEvent(QPointF, bool, bool);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) {
        if(event->button() == Qt::LeftButton) {
            event->accept();
            emit mousePressedEvent(_id, (event->modifiers() == Qt::ShiftModifier), false);
        } else {
            event->ignore();
        }
    }
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
        if(event->button() == Qt::LeftButton) {
            event->accept();
            emit mousePressedEvent(_id, (event->modifiers() == Qt::ShiftModifier), true);
        } else {
            event->ignore();
        }
    }

private:
    QPointF _id;
};

#include "layer.h"
#include "spatial_stats.h"

class ComputeAllocationLayer : public Layer {
    Q_OBJECT
public:
    ComputeAllocationLayer(MainWindow* parent = 0,
                           QString name = 0,
                           const QHash<Geometry*,Allocation*>& alloc = QHash<Geometry*,Allocation*>(),
                           SpatialStats* spatialStats = nullptr):
            Layer(parent, name), _allocation(alloc), _spatialStats(spatialStats) {}

    virtual bool load(Loader *loader);
    virtual QGraphicsItemGroup* draw();

private:
    const QHash<Geometry*,Allocation*>& _allocation;
    QHash<QPointF, Allocation*> _points;
    QHash<QPointF, GraphicsPoint*> _pointsGraphics;
    QHash<QPointF, QGraphicsItemGroup*> _pointsGroups;
    QSet<QPointF> _selectedPoints;
    SpatialStats* _spatialStats;
};


#endif //LOCALL_COMPUTEALLOCATIONLAYER_H
