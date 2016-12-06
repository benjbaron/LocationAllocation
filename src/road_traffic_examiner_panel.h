//
// Created by Benjamin Baron on 23/09/2016.
//

#ifndef LOCALL_ROAD_TRAFFIC_EXAMINER_PANEL_H
#define LOCALL_ROAD_TRAFFIC_EXAMINER_PANEL_H

#include <QDockWidget>
#include <QDate>
#include <QSet>
#include "constants.h"

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
    virtual void onClosePanel();
    virtual void connectWidgets();

protected slots:
    virtual void onDateTimeEditChanged(const QDate& date);
    virtual void onComboBoxDisplayCurrentIndexChanged(const QString& text);
    virtual void onRadioButtonAllToggled(bool checked);
    virtual void onRadioButtonMonthToggled(bool checked);
    virtual void onRadioButtonDayToggled(bool checked);

protected:
    void updateRoadTrafficLinkData();
    void populateDisplays();
    void restoreSavedSettings();

    Ui::RoadTrafficExaminerPanel* ui;
    RoadLink* _roadLink;
    QDate _date = QDate();
    QString _display = "";
    RoadTrafficExaminerDisplayType _displayType = RoadTrafficExaminerDisplayType::NoneRTEType;
    QSet<QDate> _validDates;
};


#endif //LOCALL_ROAD_TRAFFIC_EXAMINER_PANEL_H
