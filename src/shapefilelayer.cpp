#include "shapefilelayer.h"

#include "constants.h"
#include "utils.h"
#include "NumberDialog.h"

QGraphicsItemGroup* ShapefileLayer::draw()
{
    QPen pen = QPen(SHAPEFILE_COL);
    pen.setWidth(SHAPEFILE_WID);
    pen.setCosmetic(true);

    qDeleteAll(_graphicsItems);
    _graphicsItems.clear();
    _groupItem = new QGraphicsItemGroup();

    foreach (auto geom, _geometryItems) {
        if(geom && wkbFlatten(geom->getGeometryType()) == wkbLineString) {
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
        else if(geom && wkbFlatten(geom->getGeometryType()) == wkbPoint) {
            OGRPoint* pt = (OGRPoint *) geom;
            QGraphicsEllipseItem* item = new QGraphicsEllipseItem(pt->getX()-SHAPEFILE_WID/2.0, pt->getY()-SHAPEFILE_WID/2.0, SHAPEFILE_WID, SHAPEFILE_WID);
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

QSet<QPointF> ShapefileLayer::getIntersections(double maxAngle) {
    //  returns all the intersections among the OGRGeometry LineStrings
    QSet<QPointF> intersections;
    for(OGRGeometry* geom1 : _geometryItems) {
        if(wkbFlatten(geom1->getGeometryType()) == wkbLineString) {
            for(OGRGeometry* geom2 : _geometryItems) {
                // discard the geometries that are not linestrings
                if(geom2 == geom1 || wkbFlatten(geom2->getGeometryType()) != wkbLineString)
                    continue;
                // get the intersection between the two linestrings
                // TODO Remove the intersections between
                OGRGeometry * geom = geom1->Intersection(geom2);
                if(!geom->IsEmpty()) {
                    if (wkbFlatten(geom->getGeometryType()) == wkbPoint) {
                        OGRPoint* pt = (OGRPoint*) geom;
                        double angle = getAngleAtIntersection((OGRLineString*) geom1, (OGRLineString*) geom2, pt);
//                        if(angle > maxAngle)
                            intersections.insert(QPointF(pt->getX(), pt->getY()));
                    } else if(wkbFlatten(geom->getGeometryType()) == wkbMultiPoint) {
                        OGRMultiPoint* mPt = (OGRMultiPoint*) geom;
                        for(int i = 0; i < mPt->getNumGeometries(); ++i) {
                            OGRGeometry* mGeom = mPt->getGeometryRef(i);
                            if (wkbFlatten(mGeom->getGeometryType()) == wkbPoint) {
                                OGRPoint* pt  = (OGRPoint*) mGeom->clone();
                                double angle = getAngleAtIntersection((OGRLineString*) geom1, (OGRLineString*) geom2, pt);
//                                if(angle > maxAngle)
                                    intersections.insert(QPointF(pt->getX(), pt->getY()));
                            }
                        }
                    }
                }
            }
        }
    }

    return intersections;
}


double ShapefileLayer::getAngleAtIntersection(OGRLineString* ls1, OGRLineString* ls2, OGRPoint* pt) {
    // returns the angle of the intersection between geom1 and geom2 at pt

    // get the points of the intersection point of each linestring
    OGRPoint pt1a, pt1b, pt2a, pt2b;
    bool found1 = getSubLineContainingPoint(ls1, pt, &pt1a, &pt1b);
    bool found2 = getSubLineContainingPoint(ls2, pt, &pt2a, &pt2b);

    qDebug() << "found 1" << found1 << "2" << found2;

    double maxAngle = 0;
    // if we have found the sublines of the intersection point
    if(found1 && found2) {
        double x = pt->getX(), y = pt->getY();
        QLineF l1a, l1b, l2a, l2b;

        if(pt1a.IsEmpty()) {
            l1a = QLineF();
        } else {
            l1a = QLineF(pt1a.getX(), pt1a.getY(), x, y);
        }
        if(pt1b.IsEmpty()) {
            l1b = QLineF();
        } else {
            l1b = QLineF(x, y, pt1b.getX(), pt1b.getY());
        }
        if(pt2a.IsEmpty()) {
            l2a = QLineF();
        } else {
            l2a = QLineF(pt2a.getX(), pt2a.getY(), x, y);
        }
        if(pt2b.IsEmpty()) {
            l2b = QLineF();
        } else {
            l2b = QLineF(x, y, pt2b.getX(), pt2b.getY());
        }

        qDebug() << l1a.angle(l2a) << l1a.angle(l2b) << l1b.angle(l2a) <<  l1b.angle(l2b);

        if(!l1a.isNull() && !l2a.isNull()) {
            double angle = l1a.angle(l2a);
            if(angle > maxAngle) maxAngle = angle;
        }
        if(!l1a.isNull() && !l2b.isNull()) {
            double angle = l1a.angle(l2b);
            if(angle > maxAngle) maxAngle = angle;
        }
        if(!l1b.isNull() && !l2a.isNull()) {
            double angle = l1b.angle(l2a);
            if(angle > maxAngle) maxAngle = angle;
        }
        if(!l1b.isNull() && !l2b.isNull()) {
            double angle = l1b.angle(l2b);
            if(angle > maxAngle) maxAngle = angle;
        }
    }
    return maxAngle;
}

void ShapefileLayer::computeIntersections() {
    // run the intersection detection and generate a PointLayer
    QSet<QPointF> intersections = getIntersections();
    QString layerName =  getName() + " / intersections";
    _pointLayer = new PointLayer(_parent, layerName, intersections.toList());
    // add the layer to the mainwindow
    getParent()->createLayer(layerName, _pointLayer);

    // add an action to the menu
    QAction* action = _shapefileMenu->addAction("Export intersection points");
    connect(action, &QAction::triggered, this, &ShapefileLayer::exportIntersectionPoints);
}

bool ShapefileLayer::getSubLineContainingPoint(OGRLineString *ls, OGRPoint *pt, OGRPoint *ptBefore, OGRPoint *ptAfter) {
    // gets the sublines of ls that contains pt
    bool found = false;
    OGRPoint prevPt;
    ls->getPoint(0, &prevPt);
    for(int i = 1; i < ls->getNumPoints(); ++i) {
        OGRPoint curPt;
        ls->getPoint(i, &curPt);
        if(isOnLine(&prevPt, &curPt, pt)) {
            *ptBefore = prevPt;
            *ptAfter  = curPt;
            found = true;
            break;
        }
    }
    return found;
}

bool ShapefileLayer::isOnLine(OGRPoint *a, OGRPoint *b, OGRPoint *c) {
    // returns true if c is between a and b
    return euclideanDistance(a->getX(), a->getY(), c->getX(), c->getY())
           + euclideanDistance(c->getX(), c->getY(), b->getX(), b->getY())
           == euclideanDistance(a->getX(), a->getY(), b->getX(), b->getY());
}

void ShapefileLayer::exportIntersectionPoints() {
    if(!_pointLayer) return;

    // choose file
    QString filename = QFileDialog::getSaveFileName(0,
                                                    tr("Save the intersection points"),
                                                    QString(),
                                                    tr("CSV file (*.csv)"));

    if(filename.isEmpty())
        return;

    // choose radius
    NumberDialog numDiag(_parent, "Radius");
    int ret = numDiag.exec(); // synchornous
    if (ret == QDialog::Rejected) {
        return;
    }
    int radius = numDiag.getNumber();

    qDebug() << "Exporting" << _pointLayer->get_points().size() << "intersection points in" << filename;
    QFile file(filename);
    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "Unable to write in file "<< filename;
        return;
    }

    QTextStream out(&file);
    for(const QPointF p : _pointLayer->get_points()) {
        out << QString::number(p.x(), 'f', 4) << ";"
            << QString::number(p.y(), 'f', 4) << ";"
            << QString::number(radius) << "\n";
    }
    file.close();

    qDebug() << "[DONE] export intersection points";
}
