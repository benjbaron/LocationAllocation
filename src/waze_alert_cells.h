//
// Created by Benjamin Baron on 20/10/2016.
//

#ifndef LOCALL_WAZE_ALERT_CELLS_H
#define LOCALL_WAZE_ALERT_CELLS_H


#include "waze_alert_data.h"

struct WazeCellValue {
    WazeCellValue(Geometry* c) {
        cell = c;
    }
    void addWazeAlert(WazeAlert* wa) {
        if(wa) {
            alerts.insert(wa->timestamp, wa);
            nodes.insert(wa->uuid);
        }
    }
    Geometry* cell;
    QSet<QString> nodes; // nodes that visited the cell
    QMap<long long, WazeAlert*> alerts; // alerts contained in the cell
    QColor color;
};


class WazeAlertCells : public QObject {
    Q_OBJECT
public:
    WazeAlertCells(WazeAlertData* wazeAlertFile, double geometryCellsSize = 100.0) :
            _wazeAlertFile(wazeAlertFile), _geometryCellsSize(geometryCellsSize) { }

    bool computeCells();
    void setGeometryCellSize(double geometryCellsSize) {
        _geometryCellsSize = geometryCellsSize;
    }
    void clearCells() {
        _wazeCells.clear();
    }
    QHash<QPoint,WazeCellValue*>* getCells() {
        return &_wazeCells;
    }
    WazeCellValue* getWazeCellValue(Geometry* geom) {
        if(!_geomToIndex.contains(geom) && !_wazeCells.contains(_geomToIndex.value(geom)))
            return nullptr;
        return _wazeCells.value(_geomToIndex.value(geom));
    }

    double computeGetisOrdG(QPoint index);
    QColor selectColorGetisOrdG(double zScore);

private:
    WazeAlertData* _wazeAlertFile;
    QHash<QPoint,WazeCellValue*> _wazeCells;
    QHash<Geometry*,QPoint> _geomToIndex;
    double _geometryCellsSize;
};


#endif //LOCALL_WAZE_ALERT_CELLS_H
