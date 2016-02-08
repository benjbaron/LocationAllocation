#ifndef UTILS_H
#define UTILS_H

#include "qcustomplot.h"

enum WithinOperator { And, Or, NoneWithin };
enum TravelTimeStat { NoneTT, Med, Avg };
enum DistanceStat   { NoneD, Auto, FixedD };


/* Define the names of the different allocations */
const QString LOCATION_ALLOCATION_MEHTOD_NAME = "Location allocation";
const QString PAGE_RANK_MEHTOD_NAME = "Page Rank";
const QString K_MEANS_MEHTOD_NAME = "k-means";
const QString RANDOM_METHOD_NAME = "random";

static double euclideanDistance(double x1, double y1, double x2, double y2) {
    return qSqrt(qPow(x1 - x2,2) + qPow(y1 - y2,2));
}
static double euclideanDistance(QPointF a, QPointF b) {
    return euclideanDistance(a.x(), a.y(), b.x(), b.y());
}

class Distribution
{
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
                    double propotion = (double) (sum+1) / _count;
                    if(propotion >= 0.5) {
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
    Allocation(): geom(NULL), weight(0.0), demands(QHash<Geometry*, double>()), deletedCandidates(QSet<Geometry*>()) { }
    Allocation(Geometry* p, double w, const QHash<Geometry*, double>& d = QHash<Geometry*, double>(), const QSet<Geometry*>& c = QSet<Geometry*>(), int rank = -1) :
        geom(p), weight(w), demands(d), deletedCandidates(c), rank(rank) { }

    Geometry* geom;
    double weight;
    QHash<Geometry*, double> demands;  // demands assigned to the allocated point
    QSet<Geometry*> deletedCandidates; // candidates deleted
    int rank;
};

inline uint qHash(const QPointF &key)
{
    return qHash(key.x()) ^ qHash(key.y());
}

static bool toggleBoldFont(QLineEdit *lineEdit, bool isValid)
{
    QFont prevFont(lineEdit->font()); // Get previous font
    if(isValid) {
        prevFont.setBold(false);
        lineEdit->setFont(prevFont);
    } else {
        prevFont.setBold(true);
        lineEdit->setFont(prevFont);
    }
    return isValid;
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


#endif // UTILS_H

