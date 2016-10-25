//
// Created by Benjamin Baron on 23/09/2016.
//

#ifndef LOCALL_ROAD_TRAFFIC_EXAMINER_PANEL_H
#define LOCALL_ROAD_TRAFFIC_EXAMINER_PANEL_H

#include <QDockWidget>

const QString RTE_DISPLAY_JOURNEY_TIME = "Journey time";
const QString RTE_DISPLAY_SPEED = "Speed";
const QString RTE_DISPLAY_TRAFFIC_FLOW = "Traffic flow";

enum RTE_DISPLAY_TYPE { DAY, MONTH, ALL, NONE };

// forward class declarations
class QCustomPlot;
class RoadLink;

namespace Ui {
    class RoadTrafficExaminerPanel;
}

class RoadTrafficExaminerPanel : public QDockWidget {
    Q_OBJECT

public:
    explicit RoadTrafficExaminerPanel(QWidget* parent = 0);
    ~RoadTrafficExaminerPanel();

    void showRoadTrafficLinkData(RoadLink* rl);
    void onClosePanel();
    void plotFrequencies(QList<long long> frequencies, QCustomPlot* customPlot, long long bins) {}

private:
    void updateRoadTrafficLinkData();

    Ui::RoadTrafficExaminerPanel* ui;
    RoadLink* _roadLink;
    int _day = -1;
    int _month = -1;
    int _year = -1;
    QString _display = "";
    RTE_DISPLAY_TYPE _displayType = RTE_DISPLAY_TYPE::NONE;
};


#endif //LOCALL_ROAD_TRAFFIC_EXAMINER_PANEL_H
