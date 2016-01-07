#ifndef SHAPEFILELAYER_H
#define SHAPEFILELAYER_H

#include "layer.h"
#include "PointLayer.h"
#include "mainwindow.h"
#include <ogrsf_frmts.h>
#include <QMenu>

class ShapefileLayer: public Layer
{
    Q_OBJECT
public:
    ShapefileLayer(MainWindow* parent = 0, QString name = 0, QString filename = 0):
        Layer(parent, name), _filename(filename)
    {
        // add the menu to compute the intersections of the shapefile
        _menu = new QMenu();
        _menu->setTitle("Shapefile");
        QAction* action = _menu->addAction("Compute intersections");
        connect(action, &QAction::triggered, this, &ShapefileLayer::computeIntersections);
        _parent->addMenu(_menu);
        hideMenu();
    }

    void addGeometry(OGRGeometry* geom) {
        _geometryItems.append(geom);
    }
    int countGeometries() { return _geometryItems.size(); }
    QString getInformation() {
        QStringList fileSplits = _name.split(".", QString::SkipEmptyParts);
        QString fileFormat = fileSplits.at(fileSplits.size()-1);

        if(fileFormat == "shp")
            return "Shapefile Layer: " + _name;
        else if(fileFormat == "wkt")
            return "WKT Layer: " + _name;
        else return "Layer: " + _name;
    }

    QGraphicsItemGroup* draw();
    virtual bool load(Loader* loader);

    /* Loader functions */
    bool loadWKT(Loader *loader);
    bool loadShapefile(Loader *loader);

private slots:
    void computeIntersections();
    void exportIntersectionPoints();

private:
    QList<OGRGeometry*> _geometryItems;
    PointLayer* _pointLayer;
    QString _filename;

    QSet<QPointF> getIntersections(double maxAngle = 10);
    double getAngleAtIntersection(OGRLineString *ls1, OGRLineString *ls2, OGRPoint *pt);
    bool getSubLineContainingPoint(OGRLineString *ls, OGRPoint *pt, OGRPoint *ptBefore, OGRPoint *ptAfter);
    bool isOnLine(OGRPoint* a, OGRPoint* b, OGRPoint* c);
};

#endif // SHAPEFILELAYER_H
