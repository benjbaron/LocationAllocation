//
// Created by Benjamin Baron on 29/11/2016.
//

#ifndef LOCALL_ROAD_TRAFFIC_WAZE_DATA_EXAMINER_PANEL_H
#define LOCALL_ROAD_TRAFFIC_WAZE_DATA_EXAMINER_PANEL_H


#include "road_traffic_examiner_panel.h"

// forward class declarations
class WazeAlertRoadTraffic;
class WazeAlertRoadTrafficLayer;
struct RoadLinkWazeAlerts;

class RoadTrafficWazeDataExaminerPanel : public RoadTrafficExaminerPanel {
    Q_OBJECT
public:
    explicit RoadTrafficWazeDataExaminerPanel(QWidget* parent = 0,
                                              WazeAlertRoadTraffic* wart = nullptr,
                                              WazeAlertRoadTrafficLayer* wartl = nullptr);
    ~RoadTrafficWazeDataExaminerPanel();

    virtual void showRoadTrafficLinkWazeData(RoadLinkWazeAlerts* rlwa);
    virtual void onClosePanel();
    virtual void connectWidgets();

private slots:
    virtual void onComboBoxDisplayCurrentIndexChanged(const QString& text);
    virtual void onDateTimeEditChanged(const QDate& date);
    virtual void onComboBoxAlertTypeChanged(const QString& text);
    virtual void onComboBoxAlertSubTypeChanged(const QString& text);
    virtual void onRadioButtonAllToggled(bool checked);
    virtual void onRadioButtonMonthToggled(bool checked);
    virtual void onRadioButtonDayToggled(bool checked);

private:
    void updateWazeData();
    void setupUi();
    RoadLinkWazeAlerts* _roadLinkWazeData;
    WazeAlertRoadTraffic* _wazeAlertRoadTraffic;
    WazeAlertRoadTrafficLayer* _wazeAlertRoadTrafficLayer;
    QString _wazeAlertType;
    QString _wazeAlertSubType;
    QCustomPlot* _wazePlot;
    QComboBox* _alertTypesComboBox;
    QComboBox* _alertSubTypesComboBox;
};


#endif //LOCALL_ROAD_TRAFFIC_WAZE_DATA_EXAMINER_PANEL_H
