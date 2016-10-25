//
// Created by Benjamin Baron on 20/10/2016.
//

#include "waze_alert_cells.h"
#include "geometries.h"

bool WazeAlertCells::computeCells() {
    qDebug() << "Compute the cells";

    QHash<QString, QMap<long long, WazeAlert*>*>* alerts = _wazeAlertFile->getAlerts();

    if(alerts == nullptr)
        return false;

    // add the successive alerts to the cells
    for(auto it = alerts->begin(); it != alerts->end(); ++it) {
        for(auto jt = it.value()->begin(); jt != it.value()->end(); ++jt) {
            QPointF p = jt.value()->pos;
            QPoint cellIdx(qFloor(p.x() / _geometryCellsSize), qFloor(p.y() / _geometryCellsSize));
            if(!_wazeCells.contains(cellIdx)) {
                Geometry* geom = new Cell(cellIdx.x()*_geometryCellsSize, cellIdx.y()*_geometryCellsSize, _geometryCellsSize);
                _wazeCells.insert(cellIdx, new WazeCellValue(geom));
                _geomToIndex.insert(geom, cellIdx);
            }
            _wazeCells.value(cellIdx)->addWazeAlert(jt.value());
        }
    }

    return true;
}


double WazeAlertCells::computeGetisOrdG(QPoint index) {
    // Getis Ord G spatial stats

    Geometry* geom_i = _wazeCells.value(index)->cell;
    qreal sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0, sum5 = 0.0;
    int n = _wazeCells.size();
    for(auto it = _wazeCells.begin(); it != _wazeCells.end(); ++it) {
        int count_j = it.value()->alerts.size();
        Geometry* geom_j = it.value()->cell;
        int weight = (int) (1.0 / (0.01 + (qreal) qSqrt(qPow(geom_i->getCenter().x() - geom_j->getCenter().x(), 2) +
                                                        qPow(geom_i->getCenter().y() - geom_j->getCenter().y(), 2))));
        sum1 += weight * count_j;
        sum2 += weight;
        sum3 += qPow(weight,2);
        sum4 += count_j;
        sum5 += qPow(count_j,2);
    }

    // TODO compute p-scores

    qreal mean = sum4 / n;
    qreal S = qSqrt(sum5 / n - qPow(mean, 2));

    // return z-score
    return (sum1 - mean * sum2) / (S * qSqrt((n * sum3 - qPow(sum2,2)) / (n-1)));
}

QColor WazeAlertCells::selectColorGetisOrdG(double zScore) {
    if(zScore >= 3.291) return QColor("#720206");
    else if(zScore >= 2.576) return QColor("#f33f1c");
    else if(zScore >= 1.960) return QColor("#f37b22");
    else if(zScore >= 1.645) return QColor("#fffe38");
    else return QColor("#cccccc");
}