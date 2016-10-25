//
// Created by Benjamin Baron on 23/10/2016.
//

#ifndef LOCALL_WAZE_ALERT_CELLS_PANEL_H
#define LOCALL_WAZE_ALERT_CELLS_PANEL_H

#include <QDockWidget>
#include <QDateTime>
#include "waze_alert_file.h"

const QString WAZE_ALERT_ALL           = "All alerts";
const QString WAZE_ALERT_JAM           = "Jam alerts";
const QString WAZE_ALERT_WEATHERHAZARD = "Weather hazard alerts";
const QString WAZE_ALERT_ROAD_CLOSED   = "Road closed alerts";
const QString WAZE_ALERT_ACCIDENT      = "Accident alerts";



// forward class declarations
class QCustomPlot;
struct WazeCellValue;

namespace Ui {
    class WazeAlertCellsPanel;
}

class WazeAlertCellsPanel : public QDockWidget {
    Q_OBJECT

public:
    explicit WazeAlertCellsPanel(QWidget* parent = 0);
    ~WazeAlertCellsPanel();

    void showWazeAlertCellData(WazeCellValue* wcv);
    void onClosePanel();

private:
    void updateWazeAlert();

    Ui::WazeAlertCellsPanel* ui;
    WazeCellValue* _wazeCellValue = nullptr;

    QDateTime _startDT;
    QDateTime _endDT;
    QDateTime _minDT;
    QDateTime _maxDT;
    WazeAlertType _showWazeAlertType;
    QString _showWazeAlertTypeString = "";

    WazeAlertType stringToAlertType(const QString& s) {
        if(s == WAZE_ALERT_JAM) return WazeAlertType::WAZE_TYPE_JAM;
        else if(s == WAZE_ALERT_WEATHERHAZARD) return WazeAlertType::WAZE_TYPE_WEATHERHAZARD;
        else if(s == WAZE_ALERT_ROAD_CLOSED) return WazeAlertType::WAZE_TYPE_ROAD_CLOSED;
        else if(s == WAZE_ALERT_ACCIDENT) return WazeAlertType::WAZE_TYPE_ACCIDENT;
        else return WazeAlertType::WAZE_TYPE_NONE;
    }
};


#endif //LOCALL_WAZE_ALERT_CELLS_PANEL_H
