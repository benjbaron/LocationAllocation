#ifndef SHAPEFILELAYER_H
#define SHAPEFILELAYER_H

#include "layer.h"
#include "point_layer.h"
#include "mainwindow.h"
#include "geometries.h"
#include "constants.h"
#include <ogrsf_frmts.h>
#include <geos/geom/LineString.h>
#include <QMenu>

class Stop;

struct ProjectedPoint {
    ProjectedPoint(QPointF o, QPointF p):
        originalPoint(o), projectedPoint(p) { }
    QPointF originalPoint;
    QPointF projectedPoint;
    QSet<int> ls; // set of shapefile ids
    QGraphicsItem* item = nullptr;
    int projectedId; // id of the resulting linestring (that includes the projected point)
    OGRLineString* projectedLs;
    QSet<QPoint> cells;
    Stop* stop;
};


struct ConnectedComponent {
    ConnectedComponent(int id = -1): id(id) {}
    int id;
    QSet<QPointF> points;
    QSet<int> geomIds;
    void addPoint(const QPointF& pt) { points.insert(pt); }
    void addGeomId(int id) { geomIds.insert(id); }
    bool containsPoint(const QPointF& pt) const {
        return points.contains(pt);
    }
    void unite(const ConnectedComponent& cc) {
        points.unite(cc.points);
        geomIds.unite(cc.geomIds);
    }
    int nbPoints() { return points.size(); }
};

class Shapefile {
public:
    Shapefile(const QString& filename):
            _filename(filename) { }

    bool openShapefile(Loader* loader);
    void addGeometry(OGRGeometry* geom) {
        _geometryItems.insert(_geometryItems.size(),geom);
    }
    int countGeometries() { return _geometryItems.size(); }
    void getGeometries(QHash<int,OGRGeometry*>* geometries) {
        *geometries = _geometryItems;
    }
    OGRGeometry* getGeometry(int lsId) {
        return _geometryItems.value(lsId);
    }
    int getNbGeometries() { return _geometryItems.size(); }
    QSet<QPointF> getIntersections(double maxAngle = 10);
    bool projectPoints(Loader *loader, QHash<QPointF,ProjectedPoint*>* points);
    bool exportWKT(Loader *loader, const QString& output);
    bool removeNotConnectedComponents(Loader *loader);

private:
    const QString _filename;
    QHash<int,OGRGeometry*> _geometryItems;

    /* Loader functions */
    bool loadWKT(Loader *loader);
    bool loadShapefile(Loader *loader);

    double getAngleAtIntersection(OGRLineString *ls1, OGRLineString *ls2, OGRPoint *pt);
    bool getSubLineContainingPoint(OGRLineString *ls, OGRPoint *pt, OGRPoint *ptBefore, OGRPoint *ptAfter);
    bool isOnLine(OGRPoint* a, OGRPoint* b, OGRPoint* c);
};


class ProjectedPointsLayer : public Layer {
public:
    ProjectedPointsLayer(MainWindow* parent = 0, const QString& name = 0,
                         const QHash<QPointF,ProjectedPoint*>& points = QHash<QPointF,ProjectedPoint*>(),
                         Shapefile* shapefile = nullptr):
            Layer(parent, name), _points(points), _shapefile(shapefile) { }

    virtual QGraphicsItemGroup* draw() {
        int radius = 10;
        _groupItem = new QGraphicsItemGroup();
        _groupItem->setHandlesChildEvents(false);
        QColor c = Qt::red;

        for(ProjectedPoint* p : _points) {
            QColor c(BLUE);
            if(p->projectedPoint.isNull())
                c = ORANGE;

            GeometryGraphics* item = new CircleGraphics(new Circle(p->originalPoint, radius));
            item->setPen(Qt::NoPen);
            item->setBrush(QBrush(c));
            addGraphicsItem(item);

            connect(item, &CircleGraphics::mousePressedEvent, [=](Geometry* geom, bool mod){
                ProjectedPoint* projPt = _points.value(geom->getCenter());

                if(projPt->item == nullptr) {
                    // compute the QGraphicsItemGroup
                    QGraphicsItemGroup* group = new QGraphicsItemGroup();
                    for(int lsId : projPt->ls) {
                        QGraphicsPathItem* item;
                        getLineStringGraphicsItem((OGRLineString*) _shapefile->getGeometry(lsId), item);
                        QPen pen = QPen(ORANGE);
                        pen.setWidth(5);
                        pen.setCosmetic(true);
                        if(lsId == projPt->projectedId)
                            pen.setColor(Qt::darkRed);
                        item->setPen(pen);
                        group->addToGroup(item);
                    }

                    for(QPoint p : projPt->cells) {
                        QGraphicsRectItem* item = new QGraphicsRectItem(100*p.x(), -100*p.y(), 100, -100);
                        item->setBrush(Qt::red);
                        item->setPen(Qt::NoPen);
                        item->setOpacity(0.5);
                        group->addToGroup(item);
                    }
                    projPt->item = group;
                    addGraphicsItem(group);
                }
                projPt->item->setVisible(true);
            });

            if(p->projectedPoint.isNull())
                continue;

            item = new CircleGraphics(new Circle(p->projectedPoint, radius));
            item->setPen(Qt::NoPen);
            item->setBrush(QBrush(RED));
            addGraphicsItem(item);

            ArrowLineItem* line = new ArrowLineItem(p->projectedPoint.x(), p->projectedPoint.y(),
                                                    p->originalPoint.x(), p->originalPoint.y(), -1, -1, radius, radius);
            addGraphicsItem(line);
        }
        return _groupItem;
    }

    virtual bool load(Loader* loader) {
        loader->loadProgressChanged(1.0, "Done");
        return true;
    }

private:
    QHash<QPointF, ProjectedPoint*> _points;
    Shapefile* _shapefile;
};


class LineStringIndex {
public:
    LineStringIndex(Shapefile* shapefile, double cellSize = 100);
    bool projectOnClosestLineString(double x, double y, double distance,
                                    int& id, OGRLineString** modified,
                                    QPointF* projectedPoint, ProjectedPoint* projPt);

private:
    double _cellSize;
    Shapefile* _shapefile;
    QHash<QPoint,QSet<int>*> _lineStringGrid; // ids of the linestrings
    QPoint getGridCellAt(double x, double y) {
        return QPoint(qFloor(x / _cellSize), qFloor(y / _cellSize));
    }
    QPoint getGridCellAt(QPointF p) {
        return getGridCellAt(p.x(), p.y());
    }

    Bounds getBoundingBox(OGRGeometry* geom);
    void getGridCellsWithinDistance(Bounds b, QSet<QPoint>* points);
};


class ShapefileLayer: public Layer {
    Q_OBJECT
public:
    ShapefileLayer(MainWindow* parent = 0, QString name = 0, Shapefile* shapefile = nullptr):
        Layer(parent, name), _shapefile(shapefile) {
        // add the menu to compute the intersections of the shapefile
        _menu = new QMenu();
        _menu->setTitle("Shapefile");
        QAction* action_int = _menu->addAction("Compute intersections");
        connect(action_int, &QAction::triggered, this, &ShapefileLayer::computeIntersections);
        QAction* action_wkt = _menu->addAction("Export WKT");
        connect(action_wkt, &QAction::triggered, this, &ShapefileLayer::exportWKT);
        QAction* action_project = _menu->addAction("Project points");
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


private slots:
    void computeIntersections();
    void exportIntersectionPoints();
    void exportWKT();
    void projectPoints();

private:
    PointLayer* _pointLayer;
    Shapefile* _shapefile;

};

#endif // SHAPEFILELAYER_H
