#include "weightedallocationlayer.h"
#include "constants.h"
#include "loader.h"
#include "qdebug.h"

QGraphicsItemGroup *WeightedAllocationLayer::draw()
{
    int radius = 20;
    int demandRadius = 10;
    int deletedCandiateRadius = 7;
    _groupItem = new QGraphicsItemGroup();
    _groupItem->setHandlesChildEvents(false);
    QColor normalColor = Qt::blue;
    QColor selectedColor = Qt::green;

    double maxWeight = 0.0;
    for(auto it = _points.begin(); it != _points.end(); ++it) {
        double weight = it.value()->weight;
        if(weight > maxWeight) maxWeight = weight;
    }

    for(auto it = _points.begin(); it != _points.end(); ++it) {
        QPointF p = it.key();
        qreal size = radius + radius * it.value()->weight / maxWeight;

        GraphicsPoint* item = new GraphicsPoint(p, size, p);
        connect(item, &GraphicsPoint::mousePressedEvent, [=](QPointF id, bool mod, bool doubleClicked) {
            qDebug() << "Clicked on point" << id << mod << doubleClicked;

            if(doubleClicked) {
                // select all points
                for(auto it = _points.begin(); it != _points.end(); ++it) {
                    QPointF id = it.key();
                    if(!_pointsGroups.contains(id)) {
                        // add the points to the group
                        QGraphicsItemGroup* group = new QGraphicsItemGroup();
                        for(Geometry* rp : _points.value(id)->demands.keys()) {
                            QGraphicsEllipseItem* i = new CircleGraphics(new Circle(rp->getCenter(), demandRadius));
                            i->setBrush(QBrush(Qt::black));
                            i->setPen(Qt::NoPen);
                            group->addToGroup(i);
                        }
                        for(Geometry* rp : _points.value(id)->deletedCandidates) {
                            QGraphicsEllipseItem* i = new CircleGraphics(new Circle(rp->getCenter(), deletedCandiateRadius));
                            i->setBrush(QBrush(ORANGE));
                            i->setPen(Qt::NoPen);
                            group->addToGroup(i);
                        }
                        _pointsGroups.insert(id, group);
                        addGraphicsItem(group);
                    }
                    _pointsGroups.value(id)->setVisible(true);
                    _selectedPoints.insert(id);
                    _pointsGraphics.value(id)->setBrush(QBrush(selectedColor));
                }
                return;
            }

            bool wasSelected = false;
            if(_selectedPoints.contains(id)) {
                // remove teh point from the selected set
                _pointsGroups.value(id)->setVisible(false);
                _pointsGraphics.value(id)->setBrush(QBrush(normalColor));
                _selectedPoints.remove(id);
                wasSelected = true; // prevent to display it again
            }

            if(!mod) {
                // remove all of the previously selected points
                for (auto it = _selectedPoints.begin(); it != _selectedPoints.end();) {
                    QPointF selectedPoint = *it;
                    _pointsGroups.value(selectedPoint)->setVisible(false);
                    _pointsGraphics.value(selectedPoint)->setBrush(QBrush(normalColor));
                    it = _selectedPoints.erase(it); // delete the instance in the QSet
                }
            }

            if(!wasSelected) {
                // display all points related to the selected point
                if(!_pointsGroups.contains(id)) {
                    // add the points to the group
                    QGraphicsItemGroup* group = new QGraphicsItemGroup();
                    for(Geometry* rp : _points.value(id)->demands.keys()) {
                        QGraphicsEllipseItem* i = new CircleGraphics(new Circle(rp->getCenter(), demandRadius));
                        i->setBrush(QBrush(Qt::black));
                        i->setPen(Qt::NoPen);
                        group->addToGroup(i);
                    }
                    for(Geometry* rp : _points.value(id)->deletedCandidates) {
                        QGraphicsEllipseItem* i = new CircleGraphics(new Circle(rp->getCenter(), deletedCandiateRadius));
                        i->setBrush(QBrush(ORANGE));
                        i->setPen(Qt::NoPen);
                        group->addToGroup(i);
                    }
                    _pointsGroups.insert(id, group);
                    addGraphicsItem(group);
                }
                _pointsGroups.value(id)->setVisible(true);
                _selectedPoints.insert(id);
                _pointsGraphics.value(id)->setBrush(QBrush(selectedColor));
            }
        });

        item->setBrush(QBrush(normalColor));
        item->setPen(Qt::NoPen);
        _pointsGraphics.insert(it.key(), item);

        addGraphicsItem(item);
    }

    return _groupItem;
}

bool WeightedAllocationLayer::load(Loader *loader) {
    emit loader->loadProgressChanged((qreal) 0.0);
    int count = 0;
    int size = _alloc->size();
    for(auto it = _alloc->begin(); it != _alloc->end(); ++it) {
        Geometry* g = it.key();
        _points.insert(g->getCenter(), it.value());
        emit loader->loadProgressChanged((qreal) ++count / (qreal) size);
    }

    emit loader->loadProgressChanged((qreal) 1.0);
    return true;
}
