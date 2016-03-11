#ifndef WEIGHTEDALLOCATIONLAYER_H
#define WEIGHTEDALLOCATIONLAYER_H

#include "geometries.h"

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

class WeightedAllocationLayer: public Layer {
    Q_OBJECT

public:
    WeightedAllocationLayer(MainWindow* parent = 0, QString name = 0,
                            const QHash<Geometry*,Allocation*>& alloc = QHash<Geometry*,Allocation*>()):
        Layer(parent, name), _alloc(&alloc) { }

    virtual QGraphicsItemGroup* draw();
    virtual bool load(Loader* loader);

private:
    QHash<QPointF, Allocation*> _points;
    QHash<QPointF, GraphicsPoint*> _pointsGraphics;
    QHash<QPointF, QGraphicsItemGroup*> _pointsGroups;
    QSet<QPointF> _selectedPoints;
    const QHash<Geometry*,Allocation*>* _alloc;
};

#endif // WEIGHTEDALLOCATIONLAYER_H
