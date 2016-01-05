#ifndef WEIGHTEDALLOCATIONLAYER_H
#define WEIGHTEDALLOCATIONLAYER_H

#include "layer.h"
#include "utils.h"

#include "geometries.h"

class GraphicsPoint: public QObject, public QGraphicsEllipseItem
{
Q_OBJECT
public:
    GraphicsPoint():
            QGraphicsEllipseItem(), _id(QPointF()) {}
    GraphicsPoint(qreal x, qreal y, qreal w, qreal h, QPointF id):
            QGraphicsEllipseItem(x, y, w, h), _id(id) {}
    GraphicsPoint(QPointF p, int radius, QPointF id):
            QGraphicsEllipseItem(p.x()-radius, p.y()-radius, 2*radius, 2*radius), _id(id) {}

signals:
    void mousePressedEvent(QPointF, bool, bool);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
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

class WeightedAllocationLayer: public Layer
{
    Q_OBJECT
public:
    WeightedAllocationLayer(MainWindow* parent = 0, QString name = 0, const QHash<Geometry*,Allocation*>& alloc = QHash<Geometry*,Allocation*>()):
        Layer(parent, name)
    {
        for(auto it = alloc.begin(); it != alloc.end(); ++it) {
            Geometry* g = it.key();
            _points.insert(g->getCenter(), it.value());
        }
    }

    QGraphicsItemGroup* draw();
    QList<std::tuple<QPointF,double,double>> getPoints(int weight = 0, long long startTime = 0, long long endTime = 0) { return QList<std::tuple<QPointF,double,double>>(); }

    //!\\ TODO functionality to export the allocated points

private:
    QHash<QPointF, Allocation*> _points;
    QHash<QPointF, GraphicsPoint*> _pointsGraphics;
    QHash<QPointF, QGraphicsItemGroup*> _pointsGroups;
    QSet<QPointF> _selectedPoints;
};

#endif // WEIGHTEDALLOCATIONLAYER_H
