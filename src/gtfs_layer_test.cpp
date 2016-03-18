//
// Created by Benjamin Baron on 14/03/16.
//

#include <geos/geom/GeometryFactory.h>
#include <geos/operation/overlay/snap/GeometrySnapper.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/linearref/LinearLocation.h>
#include <geos/io/WKTWriter.h>
#include <qstring.h>
#include <qpoint.h>
#include <qmap.h>


void computeGeometry(QMap<QString, const QList<QPointF>*>& points) {
    geos::geom::GeometryFactory* global_factory = new geos::geom::GeometryFactory();
    std::vector<geos::geom::Geometry*> mlsVector;
    for(auto it = points.begin(); it != points.end(); ++it) {

        std::cout << "\n### " << it.key().toUtf8().constData() << " ###" << std::endl;

        geos::geom::CoordinateSequence* cl = new geos::geom::CoordinateArraySequence();
        for(QPointF pt : *(it.value())) {
            cl->add(geos::geom::Coordinate(pt.x(), pt.y()));
        }

        // create the new LS
        geos::geom::LineString* ls = global_factory->createLineString(cl);

        // remove the parts of the LS that are shared by the previous recorded LS
        geos::geom::Geometry* result = ls;
        geos::geom::Geometry* resultIntersections = nullptr;
        for(int i = 0; i < mlsVector.size(); ++i) {
            geos::geom::Geometry* lsOther = mlsVector.at(i);
            geos::geom::Geometry* intersection = ls->intersection(lsOther);

            if(intersection->getNumGeometries() > 0) {
                result = result->difference(intersection);
                mlsVector[i] = lsOther->difference(intersection);
                std::cout << "\t\tResult difference\n\t\t" << lsOther->toString() << std::endl;
                if(resultIntersections != nullptr)
                    resultIntersections->Union(intersection);
                else resultIntersections = intersection;
            }
        }

        std::cout << "\tResult difference\n" << result->toString() << std::endl;
        if(resultIntersections != nullptr)
        std::cout << "\tResult intersection\n" << resultIntersections->toString() << std::endl;


        // insert the resulting LS (with the intersection parts)
        if(resultIntersections != nullptr)
            result = result->Union(resultIntersections);
        if(result->getGeometryTypeId() == geos::geom::GEOS_MULTILINESTRING) {
            for(int i = 0; i < result->getNumGeometries(); ++i) {
                geos::geom::Geometry* geom = const_cast<geos::geom::Geometry*>(result->getGeometryN(i));
                if(geom->getGeometryTypeId() == geos::geom::GEOS_LINESTRING) {
                    mlsVector.push_back(geom);
                }
            }
        } else if(result->getGeometryTypeId() == geos::geom::GEOS_LINESTRING) {
            mlsVector.push_back(result);
        }
    }

    geos::geom::MultiLineString* mls = global_factory->createMultiLineString(mlsVector);
    std::cout << mls->toString() << std::endl;

    geos::io::WKTWriter writer;
    writer.setRoundingPrecision(3); // remove unnecessary zeros
    for(int i = 0; i < mls->getNumGeometries(); ++i) {
        const geos::geom::Geometry* geom = mls->getGeometryN(i);
        if(geom->getGeometryTypeId() == geos::geom::GEOS_LINESTRING) {
            std::cout << writer.write(geom) << std::endl;
        }
    }
}


int main(int argc, char *argv[]) {
    QMap<QString, const QList<QPointF>*> points;
    QList<QPointF> pt1, pt2, pt3;
    pt1 << QPoint(0,0) << QPoint(1,2) << QPointF(2.5,2) << QPoint(3,2) << QPoint(4,0);
    pt2 << QPoint(0,3) << QPoint(1,2) << QPoint(2,2)  << QPointF(2.5,2)   << QPoint(3,2) << QPoint(4,3);
    pt3 << QPoint(3,2) << QPoint(4,0) << QPoint(6,0);


    points.insert("points 1", &pt1);
    points.insert("points 2", &pt2);
    points.insert("points 3", &pt3);
    computeGeometry(points);
}