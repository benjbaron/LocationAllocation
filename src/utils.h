#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <ogr_geometry.h>
#include "qcustomplot.h"
#include <geos/geom/LineString.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/linearref/LinearLocation.h>
#include <geos/linearref/LocationIndexedLine.h>
#include <geos/algorithm/Angle.h>
#include "constants.h"
#include "geometries.h"

static void convertFromOGRtoGEOS(OGRLineString* in, geos::geom::LineString*& out) {
    geos::geom::CoordinateSequence* coordinates = new geos::geom::CoordinateArraySequence();
    for(int i = 0; i < in->getNumPoints(); ++i) {
        OGRPoint pt;
        in->getPoint(i, &pt);
        geos::geom::Coordinate coord(pt.getX(), pt.getY());
//        qDebug() << "\t" << pt.getX() << pt.getY() << QString::fromStdString(coord.toString());
        coordinates->add(coord);
    }
    geos::geom::GeometryFactory globalFactory;
    out = globalFactory.createLineString(coordinates);
}

static void convertFromGEOStoOGR(geos::geom::LineString *in, OGRLineString *out) {
    out->empty(); // empty the linestring
    geos::geom::CoordinateSequence* seq = in->getCoordinates();

    for(int i = 0; i < seq->getSize(); ++i) {
        geos::geom::Coordinate coord = seq->getAt(i);
        out->addPoint(coord.x, coord.y);
    }
}

static QString determineCSVDelimiter(const QString& line) {
    QList<QString> delimiters = {";",",","\t"};
    int maxFieldCount = 0;
    QString maxFieldDel = "";
    for(QString del : delimiters) {
        int len = line.split(del).size();
        if(len > maxFieldCount) {
            maxFieldCount = len;
            maxFieldDel = del;
        }
    }
    return maxFieldDel;
}

template<typename T>
class Series {
public:
    Series() {}
    Series(const QMap<T, double>& values):
        _values(values) {}
    void addValue(T idx, double val) {
        _values.insert(idx, val);
        _average = (_average * _count + val)/(_count+1);
        _count++;
    }
    bool isEmpty() { return _count == 0; }
    double getAverage() { return _average; }

    void plot(QCustomPlot* plot) {
        plot->clearPlottables();
        QVector<double> x(_count), y(_count);
        int i = 0;
        double min = 1e10, max = 0;
        for(auto it = _values.begin(); it != _values.end(); it++) {
            double val = it.value();

            x[i] = i;
            y[i] = val;
            i++;
            if(min > val)
                min = val;
            if(max < val)
                max = val;
        }
        plot->addGraph();
        plot->graph(0)->setData(x,y);
        plot->xAxis->setAutoTicks(true);
        plot->xAxis->setAutoTickLabels(true);
        plot->xAxis->setRange(0, _count-1);
        plot->yAxis->setRange(min, max);
        plot->replot();
    }

    QMap<T, double>* getValues() {
        return &_values;
    }
    int getSize() {
        return _values.size();
    }

private:
    QMap<T,double> _values; // ordered values
    int _count = 0;
    double _average = 0;
};

class Distribution {
public:
    Distribution() {}
    void addValue(int v) {
        _values[v]++;
        _cummulativeSum += v;
        _average = (_average * _count + v)/(_count+1);
        _count++;
    }

    double probability(double x) {
        if(x < 0) { return 0; }
        auto it = _values.begin();
        int sum = 0;
        while(it != _values.end() && it.value() <= x) {
            sum += it.key() * it.value();
            it++;
        }
        return sum / _cummulativeSum;
    }

    bool isEmpty() { return _count == 0; }

    double getAverage() { return _average; }
    double getMedian() {
        if(_median == 0) {
            auto it = _values.begin();
            int sum = 0;
            int prevVal = -1;
            while(it != _values.end()) {
                int n = it.value();
                double prevProportion = (double) sum / _count;
                if(prevProportion <= 0.5) {
                    double proportion = (double) (sum+1) / _count;
                    if(proportion >= 0.5) {
                        _median = (it.key() + prevVal) / 2.0;
                    }
                    double nextProportion = (double) (sum+n) / _count;
                    if(nextProportion >= 0.5) {
                        _median = it.key();
                        break;
                    }
                }

                sum += it.value();
                prevVal = it.key();
                it++;
            }
        }
        return _median;
    }

    void plot(QCustomPlot* customPlot) {
        customPlot->clearPlottables();
        int vectorSize = _values.size();
        if(vectorSize == 0)
            return;

        if(_values.firstKey() > 0) {
            vectorSize++;
            if(_values.firstKey()-1 != 0)
                vectorSize++;
        }

        QVector<double> x(vectorSize), y(vectorSize);
        int i = 0;
        if(_values.firstKey() > 0) {
            x[i] = y[i] = 0.0;
            i++;
            if(_values.firstKey()-1 != 0) {
                x[i] = _values.firstKey()-1;
                y[i] = 0.0;
                i++;
            }
        }
        double cummulativeValue = 0;
        for(auto it = _values.begin(); it != _values.end(); ++it) {
            x[i] = it.key();
            cummulativeValue += (double) it.value() / _count;
            y[i] = cummulativeValue;
//            qDebug() << i << x[i] << y[i] << it.value() << _count;
            i++;
        }

        // create graph and assign data to it:
        customPlot->addGraph();
        customPlot->graph(0)->setData(x, y);
        // give the axes some labels:
//        customPlot->xAxis->setLabel("x");
//        customPlot->yAxis->setLabel("y");
        // set axes ranges, so we see all data:
        customPlot->xAxis->setAutoTicks(true);
        customPlot->xAxis->setAutoTickLabels(true);
        customPlot->xAxis->setRange(qMin(0,_values.firstKey()), _values.lastKey());
        customPlot->yAxis->setRange(0, 1.0);
        customPlot->replot();
    }

private:
    QMap<int,int> _values; // ordered values
    int _cummulativeSum = 0;
    double _count = 0;
    double _average = 0;
    double _median = 0;
};


class Geometry;
struct Allocation {
    Allocation(): geom(NULL), weight(0.0), demands(QHash<Geometry*, double>()),
                  backends(QHash<Geometry*, double>()),
                  deletedCandidates(QSet<Geometry*>()) { }
    Allocation(Geometry* p, double w, double bw, double iw, int rank = -1,
               const QHash<Geometry*, double>& d = QHash<Geometry*, double>(),
               const QHash<Geometry*, double>& b = QHash<Geometry*, double>(),
               const QSet<Geometry*>& c = QSet<Geometry*>()) :
        geom(p), weight(w), backendWeight(bw), incomingWeight(iw), rank(rank),
        demands(d), backends(b), deletedCandidates(c) { }
    double getWeight() const { return weight + backendWeight; }
    double getWeightRank() const { return backendWeight; }
    Geometry* geom;
    double weight;
    double backendWeight;
    double incomingWeight;
    QHash<Geometry*, double> demands;  // demands assigned to the allocated point
    QSet<Geometry*> deletedCandidates; // candidates deleted
    QHash<Geometry*, double> backends; // backend node weights
    int rank;
};

inline uint qHash(const QPointF &key) {
    return qHash(key.x()) ^ qHash(key.y());
}

static bool toggleBoldFont(QLineEdit *lineEdit, bool isValid) {
    QFont prevFont(lineEdit->font()); // Get previous font
    prevFont.setBold(!isValid);
    lineEdit->setFont(prevFont);
    return isValid;
}

static bool toggleBoldFont(QLabel *label, bool isValid) {
    QFont prevFont(label->font()); // Get previous font
    prevFont.setBold(!isValid);
    label->setFont(prevFont);
    return isValid;
}

static QPointF interpolatePoint(long long time, QPointF ptBefore, long long timeBefore, QPointF ptAfter, long long timeAfter) {
    return ( (timeAfter - time)*ptBefore + (time - timeBefore)*ptAfter) / (timeAfter - timeBefore);
}

static void verticalFlipQGraphicsItem(QGraphicsItem* item) {
    QTransform transform(item->transform());

    qreal m11 = transform.m11();    // Horizontal scaling
    qreal m12 = transform.m12();    // Vertical shearing
    qreal m13 = transform.m13();    // Horizontal Projection
    qreal m21 = transform.m21();    // Horizontal shearing
    qreal m22 = transform.m22();    // vertical scaling
    qreal m23 = transform.m23();    // Vertical Projection
    qreal m31 = transform.m31();    // Horizontal Position (DX)
    qreal m32 = transform.m32();    // Vertical Position (DY)
    qreal m33 = transform.m33();    // Additional Projection Factor

    // We need this in a minute
    qreal scale = m11;

    // Horizontal flip
    m11 = -m11;

    // Re-position back to origin
    if(m31 > 0)
        m31 = 0;
    else
        m31 = (item->boundingRect().width() * scale);

    // Write back to the matrix
    transform.setMatrix(m11, m12, m13, m21, m22, m23, m31, m32, m33);

    // Set the items transformation
    item->setTransform(transform);
}

static void printConsoleProgressBar(double progress, QString const &msg = QString()) {
    int barWidth = 40;

    std::cout << "\e[2K[";
    int pos = int(barWidth * progress);
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "% " << msg.toUtf8().constData() << "\r";
    std::cout.flush();
}

static Geometry* OGRGeometryToGeometry(OGRGeometry* ogr_geom) {
    if(ogr_geom && wkbFlatten(ogr_geom->getGeometryType()) == wkbLineString) {
        OGRLineString* ls = (OGRLineString*) ogr_geom;

        QPainterPath path;
        OGRPoint pt;
        ls->getPoint(0,&pt);
        path.moveTo(QPointF(pt.getX(), pt.getY()));
        for(int i = 1; i < ls->getNumPoints(); ++i) {
            ls->getPoint(i,&pt);
            path.lineTo(QPointF(pt.getX(), pt.getY()));
        }
        return new Path(path);
    }
    else if(ogr_geom && wkbFlatten(ogr_geom->getGeometryType()) == wkbPoint) {
        OGRPoint* pt = (OGRPoint*) ogr_geom;
        return new Circle(pt->getX()-SHAPEFILE_WID/2.0, (pt->getY()-SHAPEFILE_WID/2.0), SHAPEFILE_WID);
    }
    else if(ogr_geom && wkbFlatten(ogr_geom->getGeometryType()) == wkbPolygon) {
        OGRPolygon* poly = (OGRPolygon*) ogr_geom;
        QPolygonF polygon;
        OGRPoint pt;
        poly->getExteriorRing()->getPoint(0, &pt);
        for(int i = 0; i < poly->getExteriorRing()->getNumPoints(); ++i) {
            poly->getExteriorRing()->getPoint(i, &pt);
            polygon.append(QPointF(pt.getX(), pt.getY()));
        }
        return new Polygon(polygon);
    } else {
        return nullptr;
    }
}

static QPointF projectPointOnLineString(OGRLineString* ogrls, QPointF p, double* angle = nullptr, QLineF* lineSegment = nullptr) {
    geos::geom::Coordinate pt(p.x(), p.y());

    // convert to GEOS linestring
    geos::geom::LineString* ls;
    convertFromOGRtoGEOS(ogrls, ls);

    // project the point onto the GEOS linestring
    geos::linearref::LocationIndexedLine lineRef(ls);
    geos::linearref::LinearLocation loc = lineRef.project(pt);
    geos::geom::LineSegment* seg = loc.getSegment(ls).get();
    lineSegment->setP1(QPointF(seg->p0.x, seg->p0.y));
    lineSegment->setP2(QPointF(seg->p1.x, seg->p1.y));
    *angle = ((int) geos::algorithm::Angle::toDegrees(loc.getSegment(ls)->angle()) - 90) % 360 ;

    geos::geom::Coordinate ptProj = loc.getCoordinate(ls);

    return QPointF(ptProj.x, ptProj.y);
}

static void plotFrequencies(const QList<long long>& frequencies, QCustomPlot* customPlot, long long binSize) {
    // clear the other plottable graphs
    customPlot->clearPlottables();
    for(int i = 0; i < customPlot->plottableCount(); ++i) {
        customPlot->removePlottable(i);
    }
    long long min = MaxTime;
    long long max = 0;
    for(long long f : frequencies) {
        if(f < min) min = f;
        if(f > max) max = f;
    }
    int vectorSize = qMax(1, qCeil((double) (max - min) / binSize));
    QVector<double> ticks(vectorSize), y(vectorSize);
    QVector<QString> labels(vectorSize);
    for(long long f : frequencies) {
        int i = qFloor((f - min) / (double) binSize);
        if(i == vectorSize) i = ticks.size()-1;
        y[i] += 1;
    }

    double maxValue = 0.0;
    for(int i = 0; i < vectorSize; ++i) {
        if(y[i] > maxValue) maxValue = y[i];
//        qDebug() << i << labels[i] << y[i];
        ticks[i] = i+1;
        if(y[i] > 0)
            labels[i] = QString::number(i);
        else labels[i] = "";

    }

    QCPBars *bars = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    customPlot->addPlottable(bars);
    customPlot->xAxis->setAutoTicks(false);
    customPlot->xAxis->setAutoTickLabels(false);
    customPlot->xAxis->setTickVector(ticks);
    customPlot->xAxis->setTickVectorLabels(labels);
    customPlot->xAxis->setRange(0, ticks.size()+1);
    customPlot->yAxis->setRange(0, maxValue);

    bars->setData(ticks, y);
    customPlot->replot();
}


#endif // UTILS_H

