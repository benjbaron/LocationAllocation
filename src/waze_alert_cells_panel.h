//
// Created by Benjamin Baron on 23/10/2016.
//

#ifndef LOCALL_WAZE_ALERT_CELLS_PANEL_H
#define LOCALL_WAZE_ALERT_CELLS_PANEL_H

#include <QDockWidget>
#include <QDateTime>
#include "waze_alert_data.h"

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
};


#endif //LOCALL_WAZE_ALERT_CELLS_PANEL_H
