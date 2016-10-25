//
// Created by Benjamin Baron on 23/09/2016.
//

#include "road_traffic_examiner_panel.h"
#include "ui_road_traffic_examiner_panel.h"
#include "road_traffic.h"

Q_DECLARE_METATYPE(QList<int>)

RoadTrafficExaminerPanel::RoadTrafficExaminerPanel(QWidget* parent) :
    QDockWidget(parent),
    ui(new Ui::RoadTrafficExaminerPanel) {
        ui->setupUi(this);
}

RoadTrafficExaminerPanel::~RoadTrafficExaminerPanel() {
    delete ui;
}

void RoadTrafficExaminerPanel::showRoadTrafficLinkData(RoadLink* rl) {
    _roadLink = rl;

    /* update the title */
    ui->label_description->setText("Road link "+rl->getId());

    /* fill the date comboboxes */
    QMap<QDate, QMap<int,RoadTrafficData*>*>* rtd = rl->getRoadTrafficData();
    QSet<int> days_set, months_set, years_set;
    QList<int> days, months, years;

    for(QDate d: rtd->keys()) {
        days_set.insert(d.day());
        months_set.insert(d.month());
        years_set.insert(d.year());
    }

    /* Sort the lists */
    days    = days_set.toList();    qSort(days);
    months  = months_set.toList();  qSort(months);
    years   = years_set.toList();   qSort(years);

    /* clear previous comboboxes */
    ui->comboBox_day->clear();
    ui->comboBox_month->clear();
    ui->comboBox_year->clear();

    /* add items to comboboxes */
    ui->comboBox_day->addItem("");
    for(int d : days) {
        ui->comboBox_day->addItem(QString::number(d));
    }
    ui->comboBox_month->addItem("");
    for(int m : months) {
        ui->comboBox_month->addItem(QString::number(m));
    }
    ui->comboBox_year->addItem("");
    for(int y : years) {
        ui->comboBox_year->addItem(QString::number(y));
        qDebug() << "add month" << y;
    }

    /* connect their behaviors */
    ui->comboBox_day->disconnect();
    connect(ui->comboBox_day, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            [=](const QString& text){
        _day = text.toInt();
        updateRoadTrafficLinkData();
    });
    ui->comboBox_month->disconnect();
    connect(ui->comboBox_month, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            [=](const QString& text){
        _month = text.toInt();
        updateRoadTrafficLinkData();
    });
    ui->comboBox_year->disconnect();
    connect(ui->comboBox_year, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            [=](const QString& text){
        _year = text.toInt();
        updateRoadTrafficLinkData();
    });

    /* fill the last date saved */
    QSettings settings;
    QString name = "savedDateRoadTrafficExaminer";
    if(settings.contains(name)) {
        QList<int> l = settings.value(name).value<QList<int> >();
        qDebug() << "### Retrieve list" << l;
        if(l.size() == 3) {
            _year = l.at(0);
            ui->comboBox_year->setCurrentText(QString::number(_year));
            _month =  l.at(1);
            ui->comboBox_month->setCurrentText(QString::number(_month));
            _day = l.at(2);
            ui->comboBox_day->setCurrentText(QString::number(_day));
        }
    }

    /* Display */
    QStringList displays;
    displays << "" << RTE_DISPLAY_JOURNEY_TIME << RTE_DISPLAY_SPEED << RTE_DISPLAY_TRAFFIC_FLOW;
    ui->comboBox->clear(); // remove previous items
    ui->comboBox->addItems(displays);
    ui->comboBox->disconnect();
    connect(ui->comboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            [=](QString text){
        _display = text;
        updateRoadTrafficLinkData();
    });

    name = "savedDisplayRoadTrafficExaminer";
    if(settings.contains(name)) {
        _display = settings.value(name).toString();
        ui->comboBox->setCurrentText(_display);
    }

    /* Display granularity */
    connect(ui->radioButton_all, &QRadioButton::toggled, [=](bool checked) {
        if(checked) _displayType = RTE_DISPLAY_TYPE::ALL;
        updateRoadTrafficLinkData();
    });
    connect(ui->radioButton_day, &QRadioButton::toggled, [=](bool checked) {
        if(checked) _displayType = RTE_DISPLAY_TYPE::DAY;
        updateRoadTrafficLinkData();
    });
    connect(ui->radioButton_month, &QRadioButton::toggled, [=](bool checked) {
        if(checked) _displayType = RTE_DISPLAY_TYPE::MONTH;
        updateRoadTrafficLinkData();
    });

    name = "savedDisplayTypeRoadTrafficExaminer";
    if(settings.contains(name)) {
        switch (settings.value(name).toInt()) {
            case RTE_DISPLAY_TYPE::ALL:
                _displayType = RTE_DISPLAY_TYPE::ALL;
                ui->radioButton_all->setChecked(true);
                break;
            case RTE_DISPLAY_TYPE::DAY:
                _displayType = RTE_DISPLAY_TYPE::DAY;
                ui->radioButton_day->setChecked(true);
                break;
            case RTE_DISPLAY_TYPE::MONTH:
                _displayType = RTE_DISPLAY_TYPE::MONTH;
                ui->radioButton_month->setChecked(true);
                break;
            default:
                break;
        }
    }
}

void RoadTrafficExaminerPanel::updateRoadTrafficLinkData() {
    qDebug() << "### update road traffic link data" << _year << _month << _day;
    QDate date(_year, _month, _day);
    if(!date.isValid())
        return;

    qDebug() << "\t" << date.toString("yyyy MM dd");
    QMap<int, RoadTrafficData*>* rd = _roadLink->getRoadTrafficDataFromDate(date);

    if(rd == nullptr)
        return;
    if(_display != "" && _displayType == RTE_DISPLAY_TYPE::DAY) {
        ui->plot->clearPlottables();
        QVector<double> x(rd->size()), y(rd->size());
        int i = 0;
        double min = 1e10, max = 0;
        for(auto it = rd->begin(); it != rd->end(); it++) {
            double val;
            if(_display == RTE_DISPLAY_JOURNEY_TIME) {
                val = it.value()->journeyTime;
            } else if(_display == RTE_DISPLAY_SPEED) {
                val = it.value()->speed;
            } else if(_display == RTE_DISPLAY_TRAFFIC_FLOW) {
                val = it.value()->flow;
            }

            x[i] = i;
            y[i] = val;
            i++;
            if(min > val)
                min = val;
            if(max < val)
                max = val;

            qDebug() << i << "\t" << val;
        }
        ui->plot->addGraph();
        ui->plot->graph(0)->setData(x,y);
        ui->plot->xAxis->setAutoTicks(true);
        ui->plot->xAxis->setAutoTickLabels(true);
        ui->plot->xAxis->setRange(rd->firstKey(), rd->lastKey());
        ui->plot->yAxis->setRange(min, max);
        ui->plot->replot();
    }

    if(_display != "" && _displayType == RTE_DISPLAY_TYPE::MONTH) {
        ui->plot->clearPlottables();

        int vectorSize = 0;
        for(int i = 1; i <= date.daysInMonth(); i++) {
            QDate dateCurrent(_year, _month, i);
            QMap<int, RoadTrafficData *>* rdCurrent = _roadLink->getRoadTrafficDataFromDate(dateCurrent);
            if(rdCurrent == nullptr)
                continue;

            vectorSize += rdCurrent->size();
        }

        QVector<double> x(vectorSize), y(vectorSize);
        int idx = 0;
        double min = 1e10, max = 0;
        for(int i = 1; i <= date.daysInMonth(); i++) {
            QDate dateCurrent(_year, _month, i);
            QMap<int, RoadTrafficData*>* rdCurrent = _roadLink->getRoadTrafficDataFromDate(dateCurrent);
            if(rdCurrent == nullptr)
                continue;

            for(auto it = rdCurrent->begin(); it != rdCurrent->end(); it++) {
                double val;
                if(_display == RTE_DISPLAY_JOURNEY_TIME) {
                    val = it.value()->journeyTime;
                } else if(_display == RTE_DISPLAY_SPEED) {
                    val = it.value()->speed;
                } else if(_display == RTE_DISPLAY_TRAFFIC_FLOW) {
                    val = it.value()->flow;
                }

                x[idx] = idx;
                y[idx] = val;
                idx++;
                if(min > val)
                    min = val;
                if(max < val)
                    max = val;
            }
        }

        ui->plot->addGraph();
        ui->plot->graph(0)->setData(x,y);
        ui->plot->xAxis->setAutoTicks(true);
        ui->plot->xAxis->setAutoTickLabels(true);
        ui->plot->xAxis->setRange(0, idx);
        ui->plot->yAxis->setRange(min, max);
        ui->plot->replot();
    }

    double sumFlows = 0.0;
    double sumSpeeds = 0.0;
    double sumJourneyTimes = 0.0;
    for(auto it = rd->begin(); it != rd->end(); it++) {
        sumFlows += it.value()->flow;
        sumJourneyTimes += it.value()->speed;
        sumSpeeds += it.value()->journeyTime;
    }
    qDebug() << "\t" << sumFlows / rd->size() << sumJourneyTimes / rd->size() << sumSpeeds / rd->size();
}

void RoadTrafficExaminerPanel::onClosePanel() {
    /* save the settings */
    QSettings settings;

    /* save the date */
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
    QString name = "savedDateRoadTrafficExaminer";
    QList<int> l;
    l << _year << _month << _day;
    settings.setValue(name, QVariant::fromValue(l));

    qDebug() << "### Save list" << l;

    /* save the display mode */
    name = "savedDisplayRoadTrafficExaminer";
    settings.setValue(name, _display);

    name = "savedDisplayTypeRoadTrafficExaminer";
    settings.setValue(name, _displayType);

}