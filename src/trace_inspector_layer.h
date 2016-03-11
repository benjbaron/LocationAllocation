//
// Created by Benjamin Baron on 10/03/16.
//

#ifndef LOCALL_TRACE_INSPECTOR_LAYER_H
#define LOCALL_TRACE_INSPECTOR_LAYER_H


#include "layer.h"
#include "trace.h"
#include "weighted_allocation_layer.h"

class GraphicsWaypoint : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT
public:
    GraphicsWaypoint(QPointF p, const QString& label, int radius):
            QGraphicsEllipseItem(p.x()-radius, -1*(p.y()+radius), radius*2, radius*2),
            _id(p), _radius(radius), _color(Qt::red) {

        setAcceptHoverEvents(true);

        setBrush(QBrush(_color));
        setPen(Qt::NoPen);
        setOpacity(0.5);

        // set the label
        _label = new QGraphicsTextItem(label, this);
        double scale = 1.5*_radius / _label->boundingRect().height();
        _label->setScale(scale);
        _label->setDefaultTextColor(Qt::black);

        QPointF center = this->boundingRect().center();
        _label->setPos(center.x()-(scale*_label->boundingRect().width()/2.0),
                       center.y()-(scale*_label->boundingRect().height()/2.0)-15);
        _label->setVisible(false);
    }

    void toggleLabel(bool toggle) {
        _label->setVisible(toggle);
    }

signals:
    void mousePressedEvent(QPointF);
    void mousehoverEnterEvent(QPointF);
    void mousehoverLeaveEvent(QPointF);


protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) {
        if(event->button() == Qt::LeftButton) {
            event->accept();
            emit mousePressedEvent(_id);
        } else {
            event->ignore();
        }
    }

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
        event->accept();
        emit mousehoverEnterEvent(_id);
    }
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
        event->accept();
        emit mousehoverLeaveEvent(_id);
    }

private:
    const int _radius;
    const QPointF _id;
    const QColor _color;
    QGraphicsTextItem* _label;
};


class TraceInspectorLayer : public Layer {
    Q_OBJECT
public:
    TraceInspectorLayer(MainWindow* parent = 0,
                        const QString& name = 0,
                        Trace* trace = nullptr,
                        const QString& nodeId = 0):
            Layer(parent, name), _trace(trace), _nodeId(nodeId) { }

    virtual QGraphicsItemGroup* draw();
    virtual bool load(Loader* loader) {
        loader->loadProgressChanged((qreal) 1.0, "Done");
        return true;
    }

private:
    Trace* _trace;
    const QString _nodeId;
    QPointF _selectedPoint = QPointF();
    QPointF _hoveredPoint = QPointF();
    QHash<QPointF, GraphicsWaypoint*> _pointsGraphics;
};

struct PointSampling {
    QPointF point;
    long long time;
    QPointF low;
    long long lowTime;
    QPointF up;
    long long upTime;
};

class TraceSamplingInspectorLayer : public Layer {
    Q_OBJECT
public:
    TraceSamplingInspectorLayer(MainWindow* parent = 0,
                                const QString& name = 0,
                                Trace* trace = nullptr,
                                const QList<PointSampling*>& points = QList<PointSampling*>(),
                                const QString& nodeId = 0):
            Layer(parent, name), _trace(trace), _pointsList(points), _nodeId(nodeId) { }

    virtual QGraphicsItemGroup* draw();
    virtual bool load(Loader* loader) {
        loader->loadProgressChanged((qreal) 1.0, "Done");
        return true;
    }

private:
    const QString _nodeId;
    const Trace* _trace;
    QPointF _selectedPoint;
    QHash<QPointF, GraphicsWaypoint*> _pointsGraphics;
    QHash<QPointF, GraphicsWaypoint*> _tracePointsGraphics;
    QHash<QPointF, PointSampling*> _points;
    QList<PointSampling*> _pointsList;
};


#endif //LOCALL_TRACE_INSPECTOR_LAYER_H
