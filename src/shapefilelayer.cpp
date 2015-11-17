#include "shapefilelayer.h"

#include "constants.h"

QGraphicsItemGroup* ShapefileLayer::ShapefileLayer::draw()
{
    QPen pen = QPen(SHAPEFILE_COL);
    pen.setWidth(SHAPEFILE_WID);
    pen.setCosmetic(true);

    qDeleteAll(_graphicsItems);
    _graphicsItems.clear();
    _groupItem = new QGraphicsItemGroup();

    foreach (auto geom, _geometryItems) {
        if(wkbFlatten(geom->getGeometryType()) == wkbLineString) {
            OGRLineString* ls = (OGRLineString *) geom;
            QPainterPath path;
            OGRPoint pt;
            ls->getPoint(0,&pt);
            path.moveTo(QPointF(pt.getX(), pt.getY()));
            for(int i = 1; i < ls->getNumPoints(); ++i) {
                ls->getPoint(i,&pt);
                path.lineTo(QPointF(pt.getX(), pt.getY()));
            }
            QGraphicsPathItem* item = new QGraphicsPathItem(path);
            pen.setColor(SHAPEFILE_COL);
            item->setPen(pen);
            addGraphicsItem(item);
        }
        else if(wkbFlatten(geom->getGeometryType()) == wkbPoint) {
            OGRPoint* pt = (OGRPoint *) geom;
            QGraphicsEllipseItem * item = new QGraphicsEllipseItem(pt->getX()-SHAPEFILE_WID/2.0, pt->getY()-SHAPEFILE_WID/2.0, SHAPEFILE_WID, SHAPEFILE_WID);
            item->setBrush(QBrush(SHAPEFILE_COL));
            item->setPen(Qt::NoPen);
            addGraphicsItem(item);
        }
    }

    return _groupItem;
}

QList<std::tuple<QPointF, double, double> > ShapefileLayer::getPoints(int deadline, long long startTime, long long endTime)
{
    Q_UNUSED(deadline)
    Q_UNUSED(startTime)
    Q_UNUSED(endTime)

    QList<std::tuple<QPointF, double, double> > res;
    for(int i = 0; i < _geometryItems.size(); ++i) {
        OGRGeometry* geom = _geometryItems.at(i);
        OGRPoint* point;
        geom->Centroid(point);
        QPointF p(point->getX(), point->getY());
        auto t = std::make_tuple(p, 1.0, -1.0);
        res.append(t);
    }

    return res;
}
