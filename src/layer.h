#ifndef LAYER_H
#define LAYER_H

#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QDebug>
#include <tuple>
#include <ogrsf_frmts.h>

#include "mainwindow.h"

class Layer: public QObject
{
    Q_OBJECT
public:
    Layer(MainWindow* parent = 0, QString name = 0):
        _parent(parent), _name(name) {}

    void addGraphicsItem(QGraphicsItem* item) {
        _groupItem->addToGroup(item);
        _graphicsItems.append(item);
    }
    void setVisible(bool visible) {
        _groupItem->setVisible(visible);
    }
    QString getName() { return _name; }
    void setZValue(qreal value) { _groupItem->setZValue(value); }
    qreal getZValue() { return _groupItem->zValue(); }
    MainWindow* getParent() { return _parent; }
   virtual QString getInformation() { return "Layer: " + _name; }

    // draw the layer objects
    virtual QGraphicsItemGroup* draw() = 0;

    // get the demand or candidate (point, weight, distance)
    virtual QList<std::tuple<QPointF,double,double>> getPoints(int deadline = 0, long long startTime = 0, long long endTime = 0) = 0;

    // get the geometries
    virtual OGRGeometryCollection* getGeometry(long long startTime = 0, long long endTime = 0) { return NULL; }

protected:
    MainWindow* _parent;
    QString _name;
    QGraphicsItemGroup* _groupItem;
    QList<QGraphicsItem*> _graphicsItems;
};

#endif // LAYER_H
