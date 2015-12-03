#include "weightedallocationlayer.h"
#include "constants.h"

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
        connect(item, &GraphicsPoint::mousePressedEvent, [=](QPointF id, bool mod) {
            qDebug() << "Clicked on point" << id << mod;
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
                    for(QPointF rp : _points.value(id)->demands.keys()) {
                        QGraphicsEllipseItem* i = new QGraphicsEllipseItem(rp.x()-demandRadius, rp.y()-demandRadius, 2*demandRadius, 2*demandRadius);
                        i->setBrush(QBrush(Qt::black));
                        i->setPen(Qt::NoPen);
                        group->addToGroup(i);
                    }
                    for(QPointF rp : _points.value(id)->deletedCandidates) {
                        QGraphicsEllipseItem* i = new QGraphicsEllipseItem(rp.x()-deletedCandiateRadius, rp.y()-deletedCandiateRadius, 2*deletedCandiateRadius, 2*deletedCandiateRadius);
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
