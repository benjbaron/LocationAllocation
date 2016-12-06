#ifndef SHAPEFILELAYER_H
#define SHAPEFILELAYER_H

#include "layer.h"
#include "point_layer.h"
#include "mainwindow.h"
#include "geometries.h"
#include "constants.h"
#include "shapefile.h"
#include <ogrsf_frmts.h>
#include <geos/geom/LineString.h>
#include <QMenu>


class ProjectedPointsLayer : public Layer {
    Q_OBJECT
public:
    ProjectedPointsLayer(MainWindow* parent = 0, const QString& name = 0,
                         const QHash<QPointF,ProjectedPoint*>& points = QHash<QPointF,ProjectedPoint*>(),
                         Shapefile* shapefile = nullptr):
            Layer(parent, name), _points(points), _shapefile(shapefile) { }

    virtual QGraphicsItemGroup* draw() {
        int radius = 1;
        _groupItem = new QGraphicsItemGroup();
        _groupItem->setHandlesChildEvents(false);

        for(ProjectedPoint* p : _points) {
            QGraphicsItemGroup* group = new QGraphicsItemGroup();
            group->setHandlesChildEvents(false);

            QColor c(BLUE);
            if (p->projectedPoint.isNull())
                c = ORANGE;

            GeometryGraphics* item = new CircleGraphics(new Circle(p->originalPoint, radius));
            item->setPen(Qt::NoPen);
            item->setBrush(QBrush(c));
            item->setZValue(10);
            group->addToGroup(item);
            _projectedPointSignalGraphics.insert(p, item);

            connect(static_cast<CircleGraphics*>(item), &CircleGraphics::mousePressedEvent, this,
                    &ProjectedPointsLayer::onSelectedPoint);

            if(p->projectedPoint.isNull())
                continue;

            item = new CircleGraphics(new Circle(p->projectedPoint, radius));
            item->setPen(Qt::NoPen);
            item->setBrush(QBrush(RED));
            item->setZValue(1);
            group->addToGroup(item);

            ArrowLineItem* line = new ArrowLineItem(p->projectedPoint.x(), p->projectedPoint.y(),
                                                    p->originalPoint.x(), p->originalPoint.y(),
                                                    -1, -1, nullptr, radius, radius);
            line->setZValue(1);
            line->disconnect();

            group->addToGroup(line);
            _projectedPointGraphics.insert(p, group);

            addGraphicsItem(group);
        }

        return _groupItem;
    }

    virtual bool load(Loader* loader) {
        loader->loadProgressChanged(1.0, "Done");
        return true;
    }

protected slots:
    virtual void onSelectedPoint(Geometry* geom, bool mod) {
        ProjectedPoint* projPt = _points.value(geom->getCenter());

        if(!_projectedPointClickGraphics.contains(projPt)) {
            // compute the QGraphicsItemGroup
            QGraphicsItemGroup* group = new QGraphicsItemGroup();
            for(int lsId : projPt->ls) {
                OGRLineString* ls = (OGRLineString*) _shapefile->getGeometry(lsId);

                Path* path = new Path();
                OGRPoint pt;
                ls->getPoint(0,&pt);
                path->moveTo(QPointF(pt.getX(), pt.getY()));
                for(int i = 1; i < ls->getNumPoints(); ++i) {
                    ls->getPoint(i,&pt);
                    path->lineTo(QPointF(pt.getX(), pt.getY()));
                }
                GeometryGraphics* geometry = new ArrowPathGraphics(path);

                QPen pen = QPen(ORANGE);
                pen.setWidth(5);
                pen.setCosmetic(true);
                if(lsId == projPt->projectedId)
                    pen.setColor(Qt::darkRed);
                geometry->setPen(pen);
                group->addToGroup(geometry);
            }

            for(QPoint pt : projPt->cells) {
                QGraphicsRectItem* cellItem = new QGraphicsRectItem(100*pt.x(), -100*pt.y(), 100, -100);
                cellItem->setBrush(Qt::red);
                cellItem->setPen(Qt::NoPen);
                cellItem->setOpacity(0.5);
                group->addToGroup(cellItem);
            }
            _projectedPointClickGraphics.insert(projPt, group);
            addGraphicsItem(group);
        }
        _projectedPointClickGraphics.value(projPt)->setVisible(true);
    }


protected:
    QHash<QPointF, ProjectedPoint*> _points;
    Shapefile* _shapefile;
    QHash<ProjectedPoint*, QGraphicsItem*> _projectedPointClickGraphics;
    QHash<ProjectedPoint*, QGraphicsItem*> _projectedPointGraphics;
    QHash<ProjectedPoint*, QGraphicsItem*> _projectedPointSignalGraphics;
};

class ShapefileLayer: public Layer {
    Q_OBJECT
public:
    ShapefileLayer(MainWindow* parent = 0, QString name = 0, Shapefile* shapefile = nullptr):
        Layer(parent, name), _shapefile(shapefile) {
        // add the menu to compute the intersections of the shapefile
        _menu = new QMenu();
        _menu->setTitle("Shapefile");
        QAction* action_attributes = _menu->addAction("Show attributes...");
        connect(action_attributes, &QAction::triggered, this, &ShapefileLayer::showAttributes);
        QAction* action_int = _menu->addAction("Compute intersections...");
        connect(action_int, &QAction::triggered, this, &ShapefileLayer::computeIntersections);
        QAction* action_wkt = _menu->addAction("Export WKT...");
        connect(action_wkt, &QAction::triggered, this, &ShapefileLayer::exportWKT);
        QAction* action_project = _menu->addAction("Project points...");
        connect(action_project, &QAction::triggered, this, &ShapefileLayer::projectPoints);
        _parent->addMenu(_menu);
        hideMenu();
    }

    QString getInformation() {
        QStringList fileSplits = _name.split(".", QString::SkipEmptyParts);
        QString fileFormat = fileSplits.at(fileSplits.size()-1);

        if(fileFormat == "shp")
            return "Shapefile Layer: " + _name;
        else if(fileFormat == "wkt")
            return "WKT Layer: " + _name;
        else return "Layer: " + _name;
    }

    virtual QGraphicsItemGroup* draw();
    virtual bool load(Loader* loader);

public slots:
    virtual void onFeatureSelectedEvent(Geometry* geom, bool mod);

private slots:
    void computeIntersections();
    void exportIntersectionPoints();
    void exportWKT();
    void projectPoints();
    void showAttributes();

protected:
    PointLayer* _pointLayer;
    Shapefile* _shapefile;
    QHash<Geometry*, GeometryGraphics*> _geometryGraphics;
    QHash<Geometry*, ShapefileFeature*> _geometryShapefileFeatures;
    Geometry* _selectedGeometry = nullptr;
};

#endif // SHAPEFILELAYER_H
