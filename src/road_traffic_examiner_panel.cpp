//
// Created by Benjamin Baron on 23/09/2016.
//

#include "road_traffic_examiner_panel.h"
#include "ui_road_traffic_examiner_panel.h"
#include "road_traffic.h"

RoadTrafficExaminerPanel::RoadTrafficExaminerPanel(QWidget* parent) :
        QDockWidget(parent),
        ui(new Ui::RoadTrafficExaminerPanel) {

    ui->setupUi(this);
    connectWidgets();
}

RoadTrafficExaminerPanel::~RoadTrafficExaminerPanel() {
    if(ui != nullptr)
        delete ui;
}

void RoadTrafficExaminerPanel::connectWidgets() {
    qDebug() << "RoadTrafficExaminerPanel connectWidgets";
    connect(ui->dateEdit, &QDateTimeEdit::dateChanged, this, &RoadTrafficExaminerPanel::onDateTimeEditChanged);

    connect(ui->radioButton_all, &QRadioButton::toggled, this, &RoadTrafficExaminerPanel::onRadioButtonAllToggled);
    connect(ui->radioButton_day, &QRadioButton::toggled, this, &RoadTrafficExaminerPanel::onRadioButtonDayToggled);
    connect(ui->radioButton_month, &QRadioButton::toggled, this, &RoadTrafficExaminerPanel::onRadioButtonMonthToggled);

    connect(ui->comboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this,
            &RoadTrafficExaminerPanel::onComboBoxDisplayCurrentIndexChanged);
}

void RoadTrafficExaminerPanel::restoreSavedSettings() {
    QSettings settings;

    if(settings.contains(SETTINGS_ROAD_TRAFFIC_EXAMINER_DATE)) {
        ui->dateEdit->setDate(settings.value(SETTINGS_ROAD_TRAFFIC_EXAMINER_DATE).toDate());
    }

    if(settings.contains(SETTINGS_ROAD_TRAFFIC_EXAMINER_DISPLAY)) {
        ui->comboBox->setCurrentText(settings.value(SETTINGS_ROAD_TRAFFIC_EXAMINER_DISPLAY).toString());
    }

    /* Display granularity */
    if(settings.contains(SETTINGS_ROAD_TRAFFIC_EXAMINER_DISPLAY_TYPE)) {
        switch (settings.value(SETTINGS_ROAD_TRAFFIC_EXAMINER_DISPLAY_TYPE).toInt()) {
            case RoadTrafficExaminerDisplayType::AllPeriodRTEType:
                ui->radioButton_all->setChecked(true);
                break;
            case RoadTrafficExaminerDisplayType::DayRTEType:
                ui->radioButton_day->setChecked(true);
                break;
            case RoadTrafficExaminerDisplayType::MonthRTEType:
                ui->radioButton_month->setChecked(true);
                break;
            default:
                break;
        }
    }
}

void RoadTrafficExaminerPanel::showRoadTrafficLinkData(RoadLink* rl) {
    _roadLink = rl;

    /* update the title */
    ui->label_description->setText("Road link "+rl->getId());

    /* fill the valid dates */
    QMap<QDate, RoadTrafficDataAggregate*>* rtd = rl->getRoadTrafficData();

    QDate minDate(QDate::currentDate()), maxDate(1970,0,0);
    for(QDate d: rtd->keys()) {
        if(d < minDate) minDate = d;
        if(d > maxDate) maxDate = d;
        _validDates.insert(d);
    }

    ui->dateEdit->setMinimumDate(minDate);
    ui->dateEdit->setMaximumDate(maxDate.addDays(365));
    ui->dateEdit->setDate(minDate);

    /* Display */
    populateDisplays();

    /* Restore settings */
    restoreSavedSettings();

    updateRoadTrafficLinkData();
}

void RoadTrafficExaminerPanel::populateDisplays() {
    if(!_roadLink->containsDataAtDate(_date)) {
        ui->comboBox->clear(); // remove previous items
        _display = "";
        return;
    }

    // get the previous display selected
    QString previousDisplay = _display;

    // Get the different options to display the road traffic data
    QStringList displays;
    displays << "" << _roadLink->getAllRoadTrafficDataTypes();


    ui->comboBox->clear(); // remove previous items
    ui->comboBox->addItems(displays);

    if(displays.contains(previousDisplay)) {
        ui->comboBox->setCurrentText(previousDisplay);
    }
}

void RoadTrafficExaminerPanel::updateRoadTrafficLinkData() {
    qDebug() << "### update road traffic link data" << _date << "display \"" << _display << "\" Display granularity" << _displayType;
    ui->plot->clearPlottables();

    if(!_date.isValid() || _display == "") {
        return;
    }

    RoadTrafficDataType rtdType = stringToRoadTrafficDataType(_display);
    int maxPeriodLength = _roadLink->getMaxPeriod(rtdType);

    QDate start, end;
    if(_displayType == RoadTrafficExaminerDisplayType::DayRTEType) {
        start = end = _date;
    } else if(_displayType == RoadTrafficExaminerDisplayType::MonthRTEType) {
        QDate startMonth(_date.year(), _date.month(), 1), endMonth(_date.year(), _date.month(), _date.daysInMonth());
        start = qMax(_roadLink->getMinDate(), startMonth);
        end = qMin(_roadLink->getMaxDate(), endMonth);
    } else if(_displayType == RoadTrafficExaminerDisplayType::AllPeriodRTEType) {
        start = _roadLink->getMinDate();
        end = _roadLink->getMaxDate();
    } else {
        return;
    }
    long long nbDays = start.daysTo(end);

    qDebug() << "start" << start << "end" << end << "nbDays" << nbDays;
    if(rtdType != RoadTrafficDataType::SpeedClassRTDType) {
        Series<int> s;

        populateSeries(&s,rtdType,start,nbDays,maxPeriodLength);
        // plot the graph
        s.plot(ui->plot);
        // fill the table
        ui->tableWidget->setItem(0,0,new QTableWidgetItem(QString::number(s.getAverage())));

    } else {
        QMap<RoadTrafficDataType, Series<int>*> s;
        populateSeriesSpeedClasses(&s, rtdType, start, nbDays, maxPeriodLength);
        qDebug() << "\tSeries size" << s.size();
        // plot the graph
        plotSpeedClasses(&s);
    }
}

void RoadTrafficExaminerPanel::populateSeries(Series<int>* s, RoadTrafficDataType rtdType, QDate start,
                                              long long nbDays, int maxPeriodLength) {
    double sum = 0.0;
    int idxCounter = 1;
    for(long long i = 0; i <= nbDays; ++i) {
        QDate date(start.addDays(i));
        RoadTrafficDataAggregate* rd = _roadLink->getRoadTrafficDataAtDate(date);
        int count = 1;
        if (rd == nullptr) {
            for(int j = 1; j <= maxPeriodLength; ++j) {
                s->addValue(idxCounter, 0.0);
                idxCounter++;
                count++;
            }
        } else {
            QMap<int, RoadTrafficData*>* rtd = rd->getRoadTrafficData(rtdType);
            int inflate = qFloor(maxPeriodLength / rtd->size());
            int prevIdx = 0;
            for (auto it = rtd->begin(); it != rtd->end(); it++) {
                int idx = count * inflate;
                for(int j = idx; j > prevIdx; --j) {
                    double value = it.value()->getValue() / inflate;
                    s->addValue(idxCounter, value);
                    idxCounter++;
                    count++;
                    sum += value;
                }
                prevIdx = idx;
            }
        }
    }
}

void RoadTrafficExaminerPanel::populateSeriesSpeedClasses(QMap<RoadTrafficDataType, Series<int>*>* s, RoadTrafficDataType rtdType,
                                                          QDate start, long long nbDays, int maxPeriodLength) {
    // loop for all speed classes
//    qDebug() << "start speed class" << RoadTrafficDataType::Speed0RTDType << "end speed class" << RoadTrafficDataType::Speed14RTDType;
    for(int speedClass = RoadTrafficDataType::Speed0RTDType; speedClass <= RoadTrafficDataType::Speed14RTDType; ++speedClass) {
        Series<int>* series = new Series<int>();
        populateSeries(series, (RoadTrafficDataType) speedClass, start, nbDays, maxPeriodLength);
//        qDebug() << "\tSpeed class" << speedClass << "size" << series->getValues()->size() << series->getAverage();
        s->insert((RoadTrafficDataType) speedClass, series);
    }
}

void RoadTrafficExaminerPanel::plotSpeedClasses(QMap<RoadTrafficDataType, Series<int>*>* s) {
    double max = 0;
    int maxCount = 0;
    for(RoadTrafficDataType rtdType : s->keys()) {
        int count = s->value(rtdType)->getSize();
        if(maxCount < count)
            maxCount = count;
    }

    QVector<double> yCum(maxCount);
    for(int i = 0; i < maxCount; ++i) {
        yCum[i] = 0.0;
    }

    int gi = 0;
    for(RoadTrafficDataType rtdType : s->keys()) {
        Series<int>* series = s->value(rtdType);
        if(series == nullptr)
            continue;

        ui->plot->addGraph();
        QColor color(20+11.0*rtdType, 90*(3.5-rtdType/5.5), 150, 150);
        ui->plot->graph(gi)->setLineStyle(QCPGraph::lsLine);
        ui->plot->graph(gi)->setPen(QPen(color.lighter(200)));
        ui->plot->graph(gi)->setBrush(QBrush(color));
        ui->plot->graph(gi)->setName(speedClassToLabel(rtdType));

        QMap<int, double>* values = s->value(rtdType)->getValues();
        QVector<double> x(maxCount), y(maxCount);
        int i = 0;
        for(auto it = values->begin(); it != values->end(); it++) {
            double val = it.value();
            x[i] = i;
            yCum[i] += val;
            y[i] = yCum[i];
            if(max < yCum[i])
                max = yCum[i];
            i++;
        }

        ui->plot->graph(gi)->setData(x,y);
        if(gi > 0) {
            ui->plot->graph(gi)->setChannelFillGraph(ui->plot->graph(gi-1));
        }
        gi++;
    }
    ui->plot->xAxis->setAutoTicks(true);
    ui->plot->xAxis->setAutoTickLabels(true);
    ui->plot->xAxis->setRange(0, maxCount-1);
    ui->plot->yAxis->setRange(0, max);
    ui->plot->legend->setVisible(true);
    ui->plot->legend->setBrush(QColor(255, 255, 255, 150));
    ui->plot->replot();
}

void RoadTrafficExaminerPanel::onClosePanel() {
    /* save the settings */
    QSettings settings;

    /* save the date */
    settings.setValue(SETTINGS_ROAD_TRAFFIC_EXAMINER_DATE, _date);

    /* save the display mode */
    settings.setValue(SETTINGS_ROAD_TRAFFIC_EXAMINER_DISPLAY, _display);
    settings.setValue(SETTINGS_ROAD_TRAFFIC_EXAMINER_DISPLAY_TYPE, _displayType);
}

void RoadTrafficExaminerPanel::onDateTimeEditChanged(const QDate& date) {
    _date = date;

    if(!_validDates.contains(_date)) {
        qDebug() << "Date" << _date << "is not valid for road link" << _roadLink->getId();
    }

    populateDisplays();
    qDebug() << "### Date changed" << _date << _display;

    updateRoadTrafficLinkData();
}

void RoadTrafficExaminerPanel::onComboBoxDisplayCurrentIndexChanged(const QString& text) {
    _display = text;
    updateRoadTrafficLinkData();
}
void RoadTrafficExaminerPanel::onRadioButtonAllToggled(bool checked) {
    if(checked) _displayType = RoadTrafficExaminerDisplayType::AllPeriodRTEType;
    updateRoadTrafficLinkData();
}
void RoadTrafficExaminerPanel::onRadioButtonMonthToggled(bool checked) {
    if(checked) _displayType = RoadTrafficExaminerDisplayType::MonthRTEType;
    updateRoadTrafficLinkData();
}
void RoadTrafficExaminerPanel::onRadioButtonDayToggled(bool checked) {
    if(checked) _displayType = RoadTrafficExaminerDisplayType::DayRTEType;
    updateRoadTrafficLinkData();
}
