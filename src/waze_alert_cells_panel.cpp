//
// Created by Benjamin Baron on 23/10/2016.
//

#include "waze_alert_cells_panel.h"
#include "ui_waze_alert_cells_panel.h"
#include "waze_alert_cells.h"




WazeAlertCellsPanel::WazeAlertCellsPanel(QWidget* parent) :
        QDockWidget(parent),
        ui(new Ui::WazeAlertCellsPanel) {
    ui->setupUi(this);
}

WazeAlertCellsPanel::~WazeAlertCellsPanel() {
    delete ui;
}

void WazeAlertCellsPanel::showWazeAlertCellData(WazeCellValue* wcv) {
    _wazeCellValue = wcv;

    ui->dateTimeEdit_start->clear();
    ui->dateTimeEdit_end->clear();

    /* fill the date comboboxes (QDateTimeEdit) */
    _minDT = QDateTime::fromMSecsSinceEpoch(wcv->alerts.firstKey());
    _maxDT = QDateTime::fromMSecsSinceEpoch(wcv->alerts.lastKey());
    _startDT = _minDT;
    _endDT = _maxDT;

    connect(ui->dateTimeEdit_start, &QDateTimeEdit::dateTimeChanged, [=](const QDateTime &datetime) {
        qDebug() << "changed start datetime" << _startDT << "->" << datetime;
        _startDT = datetime;
        ui->dateTimeEdit_end->setMinimumDateTime(_startDT);
        updateWazeAlert();
    });
    connect(ui->dateTimeEdit_end, &QDateTimeEdit::dateTimeChanged, [=](const QDateTime &datetime) {
        qDebug() << "changed end datetime" << _endDT << "->" << datetime;
        _endDT = datetime;
        ui->dateTimeEdit_start->setMaximumDateTime(_endDT);

        updateWazeAlert();
    });
    connect(ui->pushButton_add_hour, &QPushButton::pressed, [=](){
        ui->dateTimeEdit_start->setDateTime(_startDT.addSecs(3600));
        updateWazeAlert();
    });
    connect(ui->pushButton_sub_hour, &QPushButton::pressed, [=](){
        ui->dateTimeEdit_end->setDateTime(_endDT.addSecs(-3600));
        updateWazeAlert();
    });
    connect(ui->pushButton_all, &QPushButton::pressed, [=](){
        ui->dateTimeEdit_start->setDateTime(_minDT);
        ui->dateTimeEdit_end->setDateTime(_maxDT);
        updateWazeAlert();
    });

    ui->dateTimeEdit_start->setDateTime(_minDT);
    ui->dateTimeEdit_start->setDateTimeRange(_minDT, _maxDT);

    ui->dateTimeEdit_end->setDateTime(_maxDT);
    ui->dateTimeEdit_end->setDateTimeRange(_minDT, _maxDT);


    /* Display */
    QStringList display;
    display  << RTWDE_DISPLAY_ALL
             << RTWDE_DISPLAY_JAM
             << RTWDE_DISPLAY_WEATHERHAZARD
             << RTWDE_DISPLAY_ROADCLOSED
             << RTWDE_DISPLAY_ACCIDENT;

    ui->comboBox_alert_types->clear(); // remove previous items
    ui->comboBox_alert_types->addItems(display);
    ui->comboBox_alert_types->disconnect();
    connect(ui->comboBox_alert_types, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            [=](QString text){
        _showWazeAlertType = stringToAlertType(text);
        _showWazeAlertTypeString = text;
        updateWazeAlert();
    });
}

void WazeAlertCellsPanel::onClosePanel() {

}

void WazeAlertCellsPanel::updateWazeAlert() {

    if(_startDT.isNull() || _endDT.isNull())
        return;


    long long start = _startDT.toMSecsSinceEpoch();
    long long end   = _endDT.toMSecsSinceEpoch();

    QMap<long long, WazeAlert*> values = _wazeCellValue->alerts;

    qDebug() << "start" << start << "end" << end << "alert type"
             << _showWazeAlertType << _showWazeAlertTypeString
             << "first" << values.firstKey() << "last" << values.lastKey();

    QList<long long> frequencies;
    for(auto it = values.begin(); it != values.end(); ++it) {
        if(it.key() < start)
            continue;

        if(it.key() > end)
            break;

        if(_showWazeAlertTypeString == RTWDE_DISPLAY_ALL || it.value()->type == _showWazeAlertType) {
            frequencies.append(it.key());
        }
    }

    plotFrequencies(frequencies, ui->plot, 3600000);
}