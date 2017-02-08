//
// Created by Benjamin Baron on 17/11/2016.
//

#ifndef LOCALL_WAZE_ALERT_ROAD_TRAFFIC_LAYER_H
#define LOCALL_WAZE_ALERT_ROAD_TRAFFIC_LAYER_H


#include "layer.h"
#include "waze_alert_road_traffic.h"
#include "shapefile_layer.h"
#include "road_traffic_examiner_panel.h"
#include "road_traffic_waze_data_examiner_panel.h"

class ProjectedWazeAlertsLayer : public ProjectedPointsLayer {
    Q_OBJECT
public:
    ProjectedWazeAlertsLayer(MainWindow* parent = 0, const QString& name = 0,
                         const QHash<QPointF,ProjectedPoint*>& points = QHash<QPointF,ProjectedPoint*>(),
                         Shapefile* shapefile = nullptr) :
            ProjectedPointsLayer(parent, name, points, shapefile) { }

    virtual QGraphicsItemGroup* draw() {
        ProjectedPointsLayer::draw();

        for(auto it = _projectedPointSignalGraphics.begin(); it != _projectedPointSignalGraphics.end(); ++it) {
            CircleGraphics* c = static_cast<CircleGraphics*>(static_cast<GeometryGraphics*>(it.value()));
            c->disconnect();
            connect(c, &CircleGraphics::mousePressedEvent, this, &ProjectedWazeAlertsLayer::onSelectedPoint);

            if(!_projectedPointGraphics.contains(it.key()))
                continue;

//            // add vector magvar to group
            ProjectedWazeAlert* projAlert = static_cast<ProjectedWazeAlert*>(it.key());
            double angle = (projAlert->wazeAlert->magvar - 90) % 360;
            QPointF center = projAlert->wazeAlert->pos;
            QLineF l(center, center+QPointF(5,0));
            l.setAngle(angle);

            ArrowLineItem* line = new ArrowLineItem(l.p2().x(), l.p2().y(),
                                                    l.p1().x(), l.p1().y(),
                                                    -1, -1, nullptr, 1, 1);
            line->setZValue(1);
            line->disconnect();

            QString labelText = QString::number(-1*projAlert->wazeAlert->magvar) + "-" + projAlert->wazeAlert->street;
            QGraphicsTextItem* label = new QGraphicsTextItem(labelText, line);
            label->setScale(0.1);
            QPointF labelCenter = line->boundingRect().center();
            label->setPos(labelCenter.x(), labelCenter.y());

            QGraphicsItemGroup* group = static_cast<QGraphicsItemGroup*>(_projectedPointGraphics.value(it.key()));
            group->addToGroup(line);
        }

        return _groupItem;
    }

protected slots:
    virtual void onSelectedPoint(Geometry* geom, bool mod) {
        int radius = 1;
        QColor c(GREEN);

        ProjectedWazeAlert *projPt = static_cast<ProjectedWazeAlert*>(_points.value(geom->getCenter()));
        qDebug() << "waze alert" << projPt->wazeAlert->uuid << projPt->wazeAlert->magvar;
        if (!_projectedPointClickGraphics.contains(projPt)) {
            QGraphicsItemGroup* group = new QGraphicsItemGroup();
            for(auto it = projPt->candidateProjectedPoints.begin(); it != projPt->candidateProjectedPoints.end(); ++it) {
                GeometryGraphics* circle = new CircleGraphics(new Circle(it.value(), radius));
                circle->setPen(Qt::NoPen);
                circle->setBrush(QBrush(c));
                circle->setOpacity(0.5);

                QString labelText = QString::number(projPt->candidateAngles.value(it.key()));
                QGraphicsTextItem* label = new QGraphicsTextItem(labelText, circle);
                QPointF center = circle->boundingRect().center();
                label->setPos(center.x()-2, center.y()-2);
                label->setScale(0.1);
                group->addToGroup(circle);

                QLineF l = projPt->candidateSegments.value(it.key());
                ArrowLineItem* line = new ArrowLineItem(l.p2().x(), l.p2().y(),
                                                        l.p1().x(), l.p1().y(),
                                                        -1, -1, nullptr, radius, radius);
                line->setZValue(1);
                group->addToGroup(line);

                qDebug() << "\tangle" << projPt->candidateAngles.value(it.key()) << projPt->wazeAlert->magvar;
            }
            addGraphicsItem(group);
            _projectedPointClickGraphics.insert(projPt, group);
        }
        _projectedPointClickGraphics.value(projPt)->setVisible(true);
    }
};

class WazeAlertRoadTrafficLayer : public ShapefileLayer {
    Q_OBJECT
public:
    WazeAlertRoadTrafficLayer(MainWindow* parent = 0, QString name = 0, WazeAlertRoadTraffic* wazeAlertRoadTraffic = nullptr) :
            ShapefileLayer(parent, name, wazeAlertRoadTraffic) {
        if(parent) {
            addMenuBar();
        }
    }

    QGraphicsItemGroup* draw();
    bool load(Loader* loader);
    void addMenuBar();

private slots:
    void onFeatureSelectedEvent(Geometry* geom, bool mod);

protected:
    RoadTrafficWazeDataExaminerPanel* _roadTrafficWazeDataExaminerPanel = nullptr;
    QHash<WazeAlert*,GeometryGraphics*> _alertsToDisplay;
    QGraphicsItemGroup* _alertsDisplayed = nullptr;

    void displayAlerts(const QSet<WazeAlert*>& alerts);
};


#endif //LOCALL_WAZE_ALERT_ROAD_TRAFFIC_LAYER_H
