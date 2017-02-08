//
// Created by Benjamin Baron on 29/11/2016.
//

#include "road_traffic_waze_data_examiner_panel.h"
#include "ui_road_traffic_examiner_panel.h"
#include "waze_alert_road_traffic_layer.h"
#include "waze_alert_road_traffic.h"

RoadTrafficWazeDataExaminerPanel::RoadTrafficWazeDataExaminerPanel(QWidget* parent,
                                                                   WazeAlertRoadTraffic* wart,
                                                                   WazeAlertRoadTrafficLayer* wartl) :
        RoadTrafficExaminerPanel(parent), _wazeAlertRoadTraffic(wart), _wazeAlertRoadTrafficLayer(wartl) {
    setupUi();
    connectWidgets();
}

RoadTrafficWazeDataExaminerPanel::~RoadTrafficWazeDataExaminerPanel() {
    RoadTrafficExaminerPanel::~RoadTrafficExaminerPanel();
}

void RoadTrafficWazeDataExaminerPanel::setupUi() {
    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    QLabel* wazeAlertLabel = new QLabel(ui->dockWidgetContents);
    wazeAlertLabel->setText("Waze alert type");
    QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(wazeAlertLabel->sizePolicy().hasHeightForWidth());
    wazeAlertLabel->setSizePolicy(sizePolicy2);
    wazeAlertLabel->setMargin(5);

    horizontalLayout->addWidget(wazeAlertLabel);

    _alertTypesComboBox = new QComboBox(ui->dockWidgetContents);
    QStringList comboBoxItems;
    comboBoxItems << RTWDE_DISPLAY_ALL << RTWDE_DISPLAY_JAM
                  << RTWDE_DISPLAY_ROADCLOSED << RTWDE_DISPLAY_WEATHERHAZARD << RTWDE_DISPLAY_ACCIDENT;
    _alertTypesComboBox->addItems(comboBoxItems);

    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(_alertTypesComboBox->sizePolicy().hasHeightForWidth());
    _alertTypesComboBox->setSizePolicy(sizePolicy);

    horizontalLayout->addWidget(_alertTypesComboBox);

    ui->gridLayout_2->addLayout(horizontalLayout, 18, 0, 2, 1);

    horizontalLayout = new QHBoxLayout();
    QLabel* wazeSubAlertLabel = new QLabel(ui->dockWidgetContents);
    wazeSubAlertLabel->setText("Waze alert subtype");
    sizePolicy2.setHeightForWidth(wazeSubAlertLabel->sizePolicy().hasHeightForWidth());
    wazeSubAlertLabel->setSizePolicy(sizePolicy2);
    wazeSubAlertLabel->setMargin(5);

    horizontalLayout->addWidget(wazeSubAlertLabel);

    _alertSubTypesComboBox = new QComboBox(ui->dockWidgetContents);
    comboBoxItems.clear();
    comboBoxItems << RTWDE_DISPLAY_ALL;
    _alertSubTypesComboBox->addItems(comboBoxItems);

    sizePolicy.setHeightForWidth(_alertSubTypesComboBox->sizePolicy().hasHeightForWidth());
    _alertSubTypesComboBox->setSizePolicy(sizePolicy);

    horizontalLayout->addWidget(_alertSubTypesComboBox);

    ui->gridLayout_2->addLayout(horizontalLayout, 20, 0, 2, 1);


    _wazePlot = new QCustomPlot(ui->dockWidgetContents);
    QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(_wazePlot->sizePolicy().hasHeightForWidth());
    _wazePlot->setSizePolicy(sizePolicy3);

    ui->gridLayout_2->addWidget(_wazePlot, 22, 0, 1, 1);

    QLabel* wazeLabel = new QLabel(ui->dockWidgetContents);
    wazeLabel->setText("Waze data");
    ui->gridLayout_2->addWidget(wazeLabel, 16, 0, 1, 1);
}

void RoadTrafficWazeDataExaminerPanel::connectWidgets() {
    qDebug() << "RoadTrafficWazeDataExaminerPanel connectWidgets";
    connect(ui->dateEdit, &QDateTimeEdit::dateChanged, this, &RoadTrafficWazeDataExaminerPanel::onDateTimeEditChanged);

    connect(_alertTypesComboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &RoadTrafficWazeDataExaminerPanel::onComboBoxAlertTypeChanged);

    connect(_alertSubTypesComboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &RoadTrafficWazeDataExaminerPanel::onComboBoxAlertSubTypeChanged);



    connect(ui->radioButton_all, &QRadioButton::toggled, this, &RoadTrafficWazeDataExaminerPanel::onRadioButtonAllToggled);
    connect(ui->radioButton_day, &QRadioButton::toggled, this, &RoadTrafficWazeDataExaminerPanel::onRadioButtonDayToggled);
    connect(ui->radioButton_month, &QRadioButton::toggled, this, &RoadTrafficWazeDataExaminerPanel::onRadioButtonMonthToggled);

    connect(ui->comboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this,
            &RoadTrafficWazeDataExaminerPanel::onComboBoxDisplayCurrentIndexChanged);
}

void RoadTrafficWazeDataExaminerPanel::showRoadTrafficLinkWazeData(RoadLinkWazeAlerts* rlwa) {
    _roadLinkWazeData = rlwa;

    RoadTrafficExaminerPanel::showRoadTrafficLinkData(rlwa->roadLink);

    // get the saved value from settings
    QSettings settings;
    if(settings.contains(SETTINGS_ROAD_TRAFFIC_WAZE_DATA_EXAMINER_ALERT_TYPE_DISPLAY)) {
        _alertTypesComboBox->setCurrentText(settings.value(SETTINGS_ROAD_TRAFFIC_WAZE_DATA_EXAMINER_ALERT_TYPE_DISPLAY).toString());
    }
    if(settings.contains(SETTINGS_ROAD_TRAFFIC_WAZE_DATA_EXAMINER_ALERT_SUBTYPE_DISPLAY)) {
        _alertSubTypesComboBox->setCurrentText(settings.value(SETTINGS_ROAD_TRAFFIC_WAZE_DATA_EXAMINER_ALERT_SUBTYPE_DISPLAY).toString());
    }

    updateWazeData();
}

void RoadTrafficWazeDataExaminerPanel::onClosePanel() {
    RoadTrafficExaminerPanel::onClosePanel();

    /* save the settings */
    QSettings settings;
    settings.setValue(SETTINGS_ROAD_TRAFFIC_WAZE_DATA_EXAMINER_ALERT_TYPE_DISPLAY, _wazeAlertType);
    settings.setValue(SETTINGS_ROAD_TRAFFIC_WAZE_DATA_EXAMINER_ALERT_SUBTYPE_DISPLAY, _wazeAlertSubType);
}

void RoadTrafficWazeDataExaminerPanel::updateWazeData() {
    qDebug() << "update Waze data" << _date << _wazeAlertType << _wazeAlertSubType << _display;

    if (!_date.isValid() || !_roadLink->containsDataAtDate(_date))
        return;

    // number of bins per day
    RoadTrafficDataType rtdType = stringToRoadTrafficDataType(_display);
    int nbBins = _roadLink->getMaxPeriod(rtdType);
    if(nbBins == 0)
        nbBins = 24; // every hour

    // compute the start and end dates
    QTime startDay(0, 0, 0), endDay(23, 59, 59);
    QDateTime start, end;
    if (_displayType == RoadTrafficExaminerDisplayType::AllPeriodRTEType) {
        start.setDate(_roadLink->getMinDate());
        start.setTime(startDay);
        end.setDate(_roadLink->getMaxDate());
        end.setTime(endDay);
    } else if (_displayType == RoadTrafficExaminerDisplayType::MonthRTEType) {
        QDate startMonth(_date.year(), _date.month(), 1), endMonth(_date.year(), _date.month(), _date.daysInMonth());
        start.setDate(qMax(_roadLink->getMinDate(), startMonth));
        start.setTime(startDay);
        end.setDate(qMin(_roadLink->getMaxDate(), endMonth));
        end.setTime(endDay);
    } else if (_displayType == RoadTrafficExaminerDisplayType::DayRTEType) {
        start.setDate(_date);
        start.setTime(startDay);
        end.setDate(_date);
        end.setTime(endDay);
    }


    Series<int> s;
    int binSize = qFloor((1000.0 * 3600 * 24) / nbBins);
    auto it = _roadLinkWazeData->alerts.lowerBound(start.toMSecsSinceEpoch());

    qDebug() << "\tstart" << start << "end" << end << "bin size" << binSize;
    qDebug() << "nb total alerts" << _roadLinkWazeData->alerts.size();

    int idx = 1;
    int sum = 0;
    for(long long d = start.toMSecsSinceEpoch(); d <= end.toMSecsSinceEpoch(); d += binSize) {
        int count = 0;
        while(it.key() < d) {
            WazeAlertType alertType = stringToAlertType(_wazeAlertType);
            WazeAlertSubType alertSubType = stringToAlertSubType(_wazeAlertSubType);
            if(alertType == WazeAlertType::WAZE_TYPE_NONE
               || ((alertType == it.value()->type)
                    && (alertSubType == WazeAlertSubType::WAZE_SUBTYPE_NONE
                        || alertSubType == it.value()->subType))) {
                count++;
            }
            it++;
        }
        s.addValue(idx, count);
        sum += count;
        idx++;
    }
    s.plot(_wazePlot);
    // fill the table
    ui->tableWidget->setItem(1,0,new QTableWidgetItem(QString::number(sum)));
}

void RoadTrafficWazeDataExaminerPanel::onDateTimeEditChanged(const QDate& date) {
    RoadTrafficExaminerPanel::onDateTimeEditChanged(date);
    updateWazeData();
}

void RoadTrafficWazeDataExaminerPanel::onComboBoxDisplayCurrentIndexChanged(const QString& text) {
    RoadTrafficExaminerPanel::onComboBoxDisplayCurrentIndexChanged(text);
    updateWazeData();
}

void RoadTrafficWazeDataExaminerPanel::onComboBoxAlertTypeChanged(const QString& text) {
    _wazeAlertType = text;
    WazeAlertType alertType = stringToAlertType(_wazeAlertType);

    QStringList comboBoxItems;
    comboBoxItems << RTWDE_DISPLAY_ALL;
    if(alertType != WazeAlertType::WAZE_TYPE_NONE) {
        QSet<WazeAlertSubType>* alertSubTypes = _wazeAlertRoadTraffic->getWazeAlertData()->getAlertSubTypes(alertType);
        for(WazeAlertSubType subType : *alertSubTypes) {
            if(subType == WazeAlertSubType::WAZE_SUBTYPE_NONE)
                continue;

            comboBoxItems << alertSubTypeToString(subType);
        }
    }
    _alertSubTypesComboBox->clear();
    _alertSubTypesComboBox->addItems(comboBoxItems);

    updateWazeData();
}
void RoadTrafficWazeDataExaminerPanel::onComboBoxAlertSubTypeChanged(const QString& text) {
    _wazeAlertSubType = text;
    updateWazeData();
}
void RoadTrafficWazeDataExaminerPanel::onRadioButtonAllToggled(bool checked) {
    RoadTrafficExaminerPanel::onRadioButtonAllToggled(checked);
    updateWazeData();
}
void RoadTrafficWazeDataExaminerPanel::onRadioButtonMonthToggled(bool checked) {
    RoadTrafficExaminerPanel::onRadioButtonMonthToggled(checked);
    updateWazeData();
}
void RoadTrafficWazeDataExaminerPanel::onRadioButtonDayToggled(bool checked) {
    RoadTrafficExaminerPanel::onRadioButtonDayToggled(checked);
    updateWazeData();
}
