#include "compute_allocation_layer.h"
#include "constants.h"

bool ComputeAllocationLayer::load(Loader* loader) {
    loader->loadProgressChanged((qreal) 0.0, "");
    int count = 0;
    int size = _allocation.size();
    for(auto it = _allocation.begin(); it != _allocation.end(); ++it) {
        Geometry* g = it.key();
        _points.insert(g->getCenter(), it.value());
        qDebug() << "geom" << it.key()->toString() << it.value()->geom->toString();
        loader->loadProgressChanged((qreal) ++count / (qreal) size, "");
    }

    loader->loadProgressChanged((qreal) 1.0, "Done");
    return true;
}

QGraphicsItemGroup *ComputeAllocationLayer::draw() {
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

    double maxBackend = 0.0;
    for(auto it = _points.begin(); it != _points.end(); ++it) {
        for(auto jt = it+1; jt != _points.end(); ++jt) {
            Geometry* geom1 = it.value()->geom;
            Geometry* geom2 = jt.value()->geom;
            GeometryMatrixValue* val1;
            _spatialStats->getValue(&val1, geom1, geom2);
            GeometryMatrixValue* val2;
            _spatialStats->getValue(&val2, geom2, geom1);

            double w = 0.0;
            if(val1 != nullptr) w += val1->avgScore;
            if(val2 != nullptr) w += val2->avgScore;

            if(w > maxBackend)
                maxBackend = w;
        }
    }

    for(auto it = _points.begin(); it != _points.end(); ++it) {
        QPointF p = it.key();
        qreal size = radius + radius * it.value()->weight / maxWeight;

        GraphicsPoint* item = new GraphicsPoint(p, (int) size, p);
        connect(item, &GraphicsPoint::mousePressedEvent, [=](QPointF id, bool mod, bool doubleClicked) {
            qDebug() << "Clicked on point" << id << mod << doubleClicked;

            if(doubleClicked) {
                // select all points
                for(auto jt = _points.begin(); jt != _points.end(); ++jt) {
                    QPointF pt = jt.key();
                    if(!_pointsGroups.contains(pt)) {
                        // add the points to the group
                        QGraphicsItemGroup* group = new QGraphicsItemGroup();
                        for(Geometry* rp : _points.value(pt)->demands.keys()) {
                            QGraphicsEllipseItem* i = new CircleGraphics(new Circle(rp->getCenter(), demandRadius));
                            i->setBrush(QBrush(Qt::black));
                            i->setPen(Qt::NoPen);
                            QGraphicsLineItem* line = new QGraphicsLineItem(pt.x(), -1*pt.y(), rp->getCenter().x(), -1*rp->getCenter().y());
                            line->setPen(QPen(QBrush(Qt::black),2.0));
                            group->addToGroup(i);
                            group->addToGroup(line);
                        }
                        for(Geometry* rp : _points.value(pt)->deletedCandidates) {
                            QGraphicsEllipseItem* i = new CircleGraphics(new Circle(rp->getCenter(), deletedCandiateRadius));
                            i->setBrush(QBrush(ORANGE));
                            i->setPen(Qt::NoPen);
                            group->addToGroup(i);
                        }
                        _pointsGroups.insert(pt, group);
                        addGraphicsItem(group);
                    }
                    _pointsGroups.value(pt)->setVisible(true);
                    _selectedPoints.insert(pt);
                    _pointsGraphics.value(pt)->setBrush(QBrush(selectedColor));
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
                for (auto jt = _selectedPoints.begin(); jt != _selectedPoints.end();) {
                    QPointF selectedPoint = *jt;
                    _pointsGroups.value(selectedPoint)->setVisible(false);
                    _pointsGraphics.value(selectedPoint)->setBrush(QBrush(normalColor));
                    jt = _selectedPoints.erase(jt); // delete the instance in the QSet
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
                        QGraphicsLineItem* line = new QGraphicsLineItem(id.x(), -1*id.y(), rp->getCenter().x(), -1*rp->getCenter().y());
                        line->setPen(QPen(QBrush(Qt::black),2.0));
                        group->addToGroup(i);
                        group->addToGroup(line);
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


        // add the backend lines
        for(auto jt = it+1; jt != _points.end(); ++jt) {
            Geometry* geom1 = it.value()->geom;
            Geometry* geom2 = jt.value()->geom;
            GeometryMatrixValue* val1;
            _spatialStats->getValue(&val1, geom1, geom2);
            GeometryMatrixValue* val2;
            _spatialStats->getValue(&val2, geom2, geom1);
            double w = 0.0;
            if(val1 != nullptr) w += val1->avgScore;
            if(val2 != nullptr) w += val2->avgScore;

            qDebug() << geom1->toString() << geom2->toString() << w << val1 << val2;

            if(w == 0.0)
                continue;

            double width = 2.0 + 10.0*(w / maxBackend);

            QPointF pt1 = jt.key();
            QGraphicsLineItem* line = new QGraphicsLineItem(p.x(), -1*p.y(), pt1.x(), -1*pt1.y());
            line->setPen(QPen(QBrush(Qt::red),width));

            // add label
            QString labelText = QString::number(w);
            QGraphicsTextItem* label = new QGraphicsTextItem(labelText, line);
            QPointF center = line->boundingRect().center();
            label->setPos(center.x() - (label->boundingRect().width()  / 2.0),
                          center.y() - (label->boundingRect().height() / 2.0));
            label->setRotation(-line->line().angle());
            addGraphicsItem(line);
        }
    }

    return _groupItem;
}