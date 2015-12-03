#ifndef UTILS_H
#define UTILS_H

#include "qcustomplot.h"

enum WithinOperator { And, Or, None };
enum TravelTimeStat { Med, Avg };

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


struct Allocation {
    Allocation(): point(QPointF()), weight(0.0), demands(QHash<QPointF, double>()), deletedCandidates(QSet<QPointF>()) { }
    Allocation(QPoint p, double w, QHash<QPoint, double>& d, QSet<QPoint>& c, int cellSize) {
        // convert the cells into points
        point = QPointF(QRectF(p.x()*cellSize, p.y()*cellSize, cellSize, cellSize).center());
        weight = w;
        for(auto it = d.begin(); it != d.end(); ++it) {
            QPoint k = it.key();
            QPointF k1(QRectF(k.x()*cellSize, k.y()*cellSize, cellSize, cellSize).center());
            demands.insert(k1,it.value());
        }

        for(QPoint k : c) {
            QPointF k1(QRectF(k.x()*cellSize, k.y()*cellSize, cellSize, cellSize).center());
            deletedCandidates.insert(k1);
        }
    }
    Allocation(QPointF p, double w, QHash<QPointF, double>& d, QSet<QPointF>& c) :
        point(p), weight(w), demands(d), deletedCandidates(c) { }

    QPointF point;
    double weight;
    QHash<QPointF, double> demands;  // demands assigned to the allocated point
    QSet<QPointF> deletedCandidates; // candidates deleted
};

inline uint qHash(const QPointF &key)
{
    return qHash(key.x()) ^ qHash(key.y());
}

#endif // UTILS_H

