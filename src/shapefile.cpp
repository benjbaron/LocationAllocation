//
// Created by Benjamin Baron on 26/10/2016.
//

#include <ogrsf_frmts.h>
#include "shapefile.h"
#include "geometry_index.h"
#include "proj_factory.h"
#include <geos/io/WKTWriter.h>
#include <geos/linearref/LocationIndexedLine.h>

QSet<QPointF> Shapefile::getIntersections(double maxAngle) {
    //  returns all the intersections among the OGRGeometry LineStrings
    QSet<QPointF> intersections;
    for(ShapefileFeature* feature1: _features) {
        OGRGeometry* geom1 = feature1->ogrGeometry;
        if(wkbFlatten(geom1->getGeometryType()) == wkbLineString) {
            for(ShapefileFeature* feature2: _features) {
                OGRGeometry* geom2 = feature2->ogrGeometry;
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

GeometryIndex* Shapefile::makeGeometryIndex(int cellSize) {
    QSet<Geometry*> geometries;
    for(auto it = _features.begin(); it != _features.end(); ++it) {
        geometries.insert(it.value()->geometry);
    }
    return new GeometryIndex(geometries, cellSize);
}


double Shapefile::getAngleAtIntersection(OGRLineString* ls1, OGRLineString* ls2, OGRPoint* pt) {
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


bool Shapefile::getSubLineContainingPoint(OGRLineString *ls, OGRPoint *pt, OGRPoint *ptBefore, OGRPoint *ptAfter) {
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

bool Shapefile::isOnLine(OGRPoint *a, OGRPoint *b, OGRPoint *c) {
    // returns true if c is between a and b
    return euclideanDistance(a->getX(), a->getY(), c->getX(), c->getY())
           + euclideanDistance(c->getX(), c->getY(), b->getX(), b->getY())
           == euclideanDistance(a->getX(), a->getY(), b->getX(), b->getY());
}


bool Shapefile::loadWKT(Loader* loader) {

    // see http://gdal.org/1.11/ogr/ogr_apitut.html
    QFile* file = new QFile(_filename);
    if(!file->open(QFile::ReadOnly | QFile::Text))
        return false;

    OGRSpatialReference *poTarget = new OGRSpatialReference();
    poTarget->importFromProj4(ProjFactory::getInstance().getOutputProj());

    while(!file->atEnd()) {
        QStringList line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
        if(line.isEmpty()) continue;
        char* wktLine = line.at(0).toLatin1().data();
        OGRGeometry *poGeometry;
        OGRGeometryFactory::createFromWkt(&wktLine, poTarget, &poGeometry);
        addGeometry(poGeometry);
        if(loader != nullptr) {
            loader->loadProgressChanged(1.0 - file->bytesAvailable() / (qreal) file->size(), "");
        }
    }

    if(loader != nullptr) {
        loader->loadProgressChanged((qreal)1.0, "Done");
    }
    qDebug() << "loaded wkt file" << QFileInfo(_filename).fileName() << "with" <<
             nbFeatures() << "features";
    return true;
}

bool Shapefile::loadCSV(Loader* loader) {
    /* process the shapefile first */
    QFile file(_filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    OGRSpatialReference *poTarget = new OGRSpatialReference();
    poTarget->importFromProj4(ProjFactory::getInstance().getOutputProj());

    // read the header
    QString line = QString(file.readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
    QStringList fields = line.split(",");
    _attributes.append(fields.at(_indexes->join));
    if(_indexes->description != -1) {
        _attributes.append(fields.at(_indexes->description));
    }

    while (!file.atEnd()) {
        line = QString(file.readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
        fields = line.split(",");
        QString joinIdx = fields.at(_indexes->join);
        QString description = _indexes->description == -1 ? "" : fields.at(_indexes->description);
        double lon1 = fields.at(_indexes->x1).toDouble();
        double lat1 = fields.at(_indexes->y1).toDouble();
        double lon2 = fields.at(_indexes->x2).toDouble();
        double lat2 = fields.at(_indexes->y2).toDouble();

        // create a WKT Linestring from the two pairs of coordinates
        char* wktLine = QString("LINESTRING(%1 %2,%3 %4)").arg(QString::number(lon1),
                                                               QString::number(lat1),
                                                               QString::number(lon2),
                                                               QString::number(lat2)).toLatin1().data();

        OGRGeometry *poGeometry;
        OGRGeometryFactory::createFromWkt(&wktLine, poTarget, &poGeometry);
        QList<QString>* list = new QList<QString>();
        list->append(joinIdx);
        if(_indexes->description == -1) {
            list->append(description);
        }
        addGeometry(poGeometry, list);

        // Indicate the load progress of the file
        if(loader != nullptr) {
            loader->loadProgressChanged(0.3 * (1.0 - file.bytesAvailable() / (qreal) file.size()), "Loading shapefile...");
        }
    }

    file.close();
    return true;
}

bool Shapefile::loadShapefile(Loader* loader) {
    OGRRegisterAll();
    OGRDataSource* poDS;

    poDS = OGRSFDriverRegistrar::Open(_filename.toStdString().c_str(), FALSE);
    if( poDS == NULL ) {
        qWarning() << "Open failed for " << _filename;
        return false;
    }

    // Set the accepted feature fields for
//    QSet<QString> acceptedHWFieldsSet = QSet<QString>();
//    acceptedHWFieldsSet << "primary_link" << "tertiary_link" << "trunk_link" << "motorway" << "road" <<  "secondary_link" << "tertiary" << "motorway_link" << "secondary" << "trunk" << "primary";

    for(int i = 0; i < poDS->GetLayerCount(); ++i) {
        OGRLayer* poLayer = poDS->GetLayer(i);
        qDebug() << "Loading layer" << QString::fromStdString(poLayer->GetName()) << "...";
        OGRFeature* poFeature;
        OGRSpatialReference* poTarget = new OGRSpatialReference();
        poTarget->importFromProj4(ProjFactory::getInstance().getOutputProj());

        poLayer->ResetReading();

        int nrofFeatures = 0;
        while( (poFeature = poLayer->GetNextFeature()) != NULL ) {

            OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();
            /* populate the fields */
            QList<QString>* list = new QList<QString>();
            for(int iField = 0; iField < poFDefn->GetFieldCount(); iField++ ) {
                if(nrofFeatures == 0) {
                    // Populate the list of field names
                    OGRFieldDefn* poFieldDefn = poFDefn->GetFieldDefn( iField );
                    _attributes.append(poFieldDefn->GetNameRef());
                }
                list->append(poFeature->GetFieldAsString(iField));
            }

            OGRGeometry* poGeometry = poFeature->GetGeometryRef();
            if( poGeometry != NULL
                && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint) {
                poGeometry->transformTo(poTarget);
                OGRPoint* pt = (OGRPoint*) poGeometry;
                addGeometry(pt, list);
            }
            else if (poGeometry != NULL
                     && wkbFlatten(poGeometry->getGeometryType()) == wkbLineString) {
                poGeometry->transformTo(poTarget);
                OGRLineString* ls = (OGRLineString*) poGeometry;
                addGeometry(ls, list);
            }
            else if (poGeometry != NULL
                     && wkbFlatten(poGeometry->getGeometryType()) == wkbMultiLineString) {
                poGeometry->transformTo(poTarget);
                OGRMultiLineString* mls = (OGRMultiLineString*) poGeometry;
                for(int i = 0; i < mls->getNumGeometries(); ++i) {
                    OGRLineString* ls = (OGRLineString*) mls->getGeometryRef(i);
                    addGeometry(ls, list);
                }
            }
            else if (poGeometry != NULL
                     && wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon) {
                poGeometry->transformTo(poTarget);
                OGRPolygon* poly = (OGRPolygon*) poGeometry;
                addGeometry(poly, list);
            } else if (poGeometry != NULL
                       && wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon){
                OGRMultiPolygon* multiPolygon = (OGRMultiPolygon*) poGeometry;
                for(int i = 0; i < multiPolygon->getNumGeometries(); ++i) {
                    OGRPolygon* poly = (OGRPolygon*) multiPolygon->getGeometryRef(i);
                    addGeometry(poly, list);
                }
            } else {
                if(poGeometry)
                    qDebug() << QString::fromLatin1(poGeometry->getGeometryName());
            }

            if(loader != nullptr && nrofFeatures % 100 == 0) {
                qreal loadProgress = nrofFeatures / (qreal) poLayer->GetFeatureCount();
                loader->loadProgressChanged(loadProgress, "Loading features...");
            }
            nrofFeatures++;
        }
    }

    if(loader != nullptr)
        loader->loadProgressChanged((qreal)1.0, "Done");
    // Do not delete any structure as they will be used later
    qDebug() << "loaded shapefile" << QFileInfo(_filename).fileName() << "with" <<
             nbFeatures() << "features";
    return true;
}


bool Shapefile::projectPoints(Loader *loader, QHash<QPointF, ProjectedPoint*>* points) {

    double size = points->size();
    int count = 0;
    LineStringIndex lsIndex(this, 100);
    for(auto it = points->begin(); it != points->end(); ++it) {
        ProjectedPoint* ptStruct = it.value();

        int oId; // id of the original linestring
        OGRLineString* nLS; // pointer to the modified linestring that includes the projected point
        QPointF pt;

        QPointF oPt = ptStruct->originalPoint;

        bool ret = lsIndex.projectOnClosestLineString(oPt.x(),oPt.y(),100,oId,&nLS,&pt,ptStruct);
        if(ret) {
            ptStruct->projectedPoint = pt;
            ptStruct->ls.insert(oId);

            // update the shapefile
            _features[oId]->ogrGeometry = nLS;

        } else {
            qDebug() << QString("could not project point (%1,%2) on the linestring").arg(QString::number(oPt.x()),QString::number(oPt.y()));
        }

        if(loader != nullptr) {
            loader->loadProgressChanged(count / size, "");
        }
        count++;
    }
    if(loader != nullptr) {
        loader->loadProgressChanged(1.0, "Done");
    }
    return true;
}

bool Shapefile::exportWKT(Loader* loader, QString output) {
    QFile file(output);
    if(!file.open(QFile::WriteOnly)) {
        if(loader != nullptr) {
            loader->loadProgressChanged(1.0, "Done");
        }
        qWarning() << "Unable to write in file" << output;
        return false;
    }

    double size = _features.size();
    int count = 0;
    QTextStream out(&file);
    geos::io::WKTWriter writer;
    writer.setRoundingPrecision(3);
    for(ShapefileFeature* feature: _features) {
        OGRGeometry* geom = feature->ogrGeometry;
        // convert linestring to geos
        OGRLineString* ogrls = (OGRLineString*) geom;
        geos::geom::LineString* ls;
        convertFromOGRtoGEOS(ogrls, ls);

        out << QString::fromStdString(writer.write(ls)) << "\n";
        if(loader != nullptr) {
            loader->loadProgressChanged(count / size, "");
        }
        count++;
    }
    if(loader != nullptr) {
        loader->loadProgressChanged(1.0, "Done");
    }

    return true;
}

bool Shapefile::open(Loader* loader) {
    // get the format of the file to load - either *.wkt or *.shp
    QStringList fileSplits = _filename.split(".", QString::SkipEmptyParts);
    QString fileFormat = fileSplits.at(fileSplits.size()-1);

    if(fileFormat == "shp") {
        return loadShapefile(loader);
    } else if(fileFormat == "wkt") {
        return loadWKT(loader);
    } else if(fileFormat == "csv" || fileFormat == "txt") {
        return loadCSV(loader);
    } else {
        if(loader != nullptr) {
            loader->loadProgressChanged((qreal) 1.0, "Done");
        }
        return false;
    }
}


bool Shapefile::removeNotConnectedComponents(Loader *loader) {
    // compute the connected components
    QHash<int,ConnectedComponent> connectedComponents; // <ccId, cc>
    int ccCount = 0;
    double size = _features.size();
    for(auto it = _features.begin(); it != _features.end(); ++it) {
        OGRLineString* ls = (OGRLineString*) it.value()->ogrGeometry;
        ConnectedComponent cc(ccCount);
        cc.addGeomId(it.key());
        QSet<int> ccToMerge;
        for(int j = 0; j < ls->getNumPoints(); ++j) {
            OGRPoint p;
            ls->getPoint(j, &p);
            QPointF pt(p.getX(), p.getY());
            cc.addPoint(pt);

            // get the components that share this point
            for(auto jt = connectedComponents.begin(); jt != connectedComponents.end(); ++jt) {
                if(jt.value().containsPoint(pt)) {
                    ccToMerge.insert(jt.key());
                }
            }
        }

        // merge the connected components to merge
        for(int id : ccToMerge) {
            cc.unite(connectedComponents.value(id));
        }

        // remove the components that were merged
        for(int id : ccToMerge) {
            connectedComponents.remove(id);
        }

        // add the new component to the list
        connectedComponents.insert(ccCount,cc);

        ccCount++;

        loader->loadProgressChanged(0.8*(ccCount / size), "");
    }

    loader->loadProgressChanged(0.8, "Get the max component");
    // get the connected component that has the maximum number of points
    int id = -1;
    int maxCount = -1;
    for(auto it = connectedComponents.begin(); it != connectedComponents.end(); ++it) {
        ConnectedComponent cc = it.value();
        int nbPoints = cc.nbPoints();
        if(nbPoints > maxCount) {
            maxCount = nbPoints;
            id = it.key();
        }
    }

    loader->loadProgressChanged(0.9, "Remove the other components");

    // remove the connected components that are not the biggest one
    for(auto it = connectedComponents.begin(); it != connectedComponents.end(); ++it) {
        if(it.key() == id) continue;
        // remove the OGRGeometries from the shapefile
        ConnectedComponent cc = it.value();
        for(int geomId : cc.geomIds) {
            _features.remove(geomId);
        }
    }

    loader->loadProgressChanged(1.0, "Done");
    return true;
}


LineStringIndex::LineStringIndex(Shapefile* shapefile, double cellSize):
        _cellSize(cellSize), _shapefile(shapefile) {
    /* construct the grid from the shapefile's geometries */

    // populate the ogrGeometry set with the given geometries
    QHash<int,ShapefileFeature*>* features = shapefile->getFeatures();
    for(auto it = features->begin(); it != features->end(); ++it) {
        OGRGeometry* geom = it.value()->ogrGeometry;
        int idx = it.key(); // id of the OGRGeometry

        QSet<QPoint> points;
        Bounds b = getBoundingBox(geom);
        getGridCellsWithinDistance(b, &points);
        for(QPoint cellIdx : points) {
            if(!_lineStringGrid.contains(cellIdx))
                _lineStringGrid.insert(cellIdx, new QSet<int>());
            _lineStringGrid.value(cellIdx)->insert(idx);
        }
    }
}


bool LineStringIndex::projectOnClosestLineString(double x, double y, double distance,
                                                 int& id, OGRLineString** modified,
                                                 QPointF* projectedPoint, ProjectedPoint* projPt) {

    // get all the LineStrings that are within the distance
    Bounds b(x-distance, x+distance, y-distance, y+distance);
    QSet<QPoint> points;
    getGridCellsWithinDistance(b, &points);
    QSet<int> linestringIds;
    for(QPoint cellIdx : points) {
        projPt->cells.insert(cellIdx);
        if(_lineStringGrid.contains(cellIdx))
            linestringIds.unite(_lineStringGrid.value(cellIdx)->toList().toSet());
    }

    // get the point's coordinates
    geos::geom::Coordinate pt(x,y);
    double minDist = distance;
    int minId = -1;
    geos::geom::LineString* minLineString = nullptr;
    geos::linearref::LinearLocation minLoc;
    geos::geom::Coordinate minpt;

    geos::linearref::LinearLocation loc;

    // try to project the point on the closest LineString
    for(int lsId : linestringIds) {
        OGRLineString* ogrls = (OGRLineString*) _shapefile->getGeometry(lsId);

        // convert to GEOS linestring
        geos::geom::LineString* ls;
        convertFromOGRtoGEOS(ogrls, ls);

        projPt->ls.insert(lsId);

        // project the point onto the GEOS linestring
        geos::linearref::LocationIndexedLine lineRef(ls);
        loc = lineRef.project(pt);
        geos::geom::Coordinate ptProj = loc.getCoordinate(ls);

        double dist = pt.distance(ptProj);
        if(dist < minDist) {
            minId = lsId;
            minLineString = ls;
            minDist = dist;
            minLoc = loc;
            minpt = ptProj;
        }
    }

    // modify the LineString to include the projected point
    if(minLineString != nullptr) {
        // get the index of the location
        uint idx = minLoc.getSegmentIndex();

        // add the projected point to the LineString
        geos::geom::CoordinateSequence* seq = minLineString->getCoordinates();
        seq->add(idx+1, minpt, false);

        delete minLineString;
        geos::geom::GeometryFactory globalFactory;
        minLineString = globalFactory.createLineString(seq);

        // save the resulting GEOS LineString into an OGR LineString
        OGRLineString* resLineString = new OGRLineString();
        convertFromGEOStoOGR(minLineString, resLineString);

        // update the original and modified linestring pointers passed as arguments
        id = minId;
        *modified = resLineString;
        *projectedPoint = QPointF(minpt.x, minpt.y);
        projPt->projectedId = minId;
        projPt->projectedLs = resLineString;

        return true;
    }

    return false;
}

Bounds LineStringIndex::getBoundingBox(OGRGeometry *geom) {
    OGREnvelope envelope;
    geom->getEnvelope(&envelope);
    return Bounds(envelope.MinX, envelope.MaxX, envelope.MinY, envelope.MaxY);
}

void LineStringIndex::getGridCellsWithinDistance(Bounds b, QSet<QPoint>* points) {
    QPointF topLeft       = b.getTopLeft();
    QPointF bottomRight   = b.getBottomRight();
    QPoint topLeftIdx     = getGridCellAt(topLeft);
    QPoint bottomRightIdx = getGridCellAt(bottomRight);
    for(int i = topLeftIdx.x(); i <= bottomRightIdx.x(); ++i) {
        for(int j = topLeftIdx.y(); j <= bottomRightIdx.y(); ++j) {
            QPoint cellIdx(i,j);
            points->insert(cellIdx);
        }
    }
}
