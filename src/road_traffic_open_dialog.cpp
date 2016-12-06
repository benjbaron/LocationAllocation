//
// Created by Benjamin Baron on 21/09/2016.
//

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSettings>
#include <ogrsf_frmts.h>
#include "road_traffic_open_dialog.h"
#include "ui_road_traffic_open_dialog.h"

RoadTrafficOpenDialog::RoadTrafficOpenDialog(QWidget* parent, const QString& name) :
        QDialog(parent), _name(name), _dataIdx(new RoadTrafficDataIndexes), _roadLinkIdx(new ShapefileIndexes),
        ui(new Ui::RoadTrafficOpenDialog) {
    ui->setupUi(this);

    QString settingShapefileName = "savedRoadTrafficOpenDialog-"+name+"-shapefile";
    QString settingDataName      = "savedRoadTrafficOpenDialog-"+name+"-data";

    /* Shapefile */
    ui->shapefileLabel->setBuddy(ui->shapefileLineEdit);
    ui->shapefileGridWidget->setVisible(false);

    connect(ui->shapefileFileButton, &QPushButton::clicked, [=] () {
        QSettings settings;
        QFileDialog d(this, "Open a shapefile");
        QString path = d.getOpenFileName(this,
                                         "Open a shapefile",
                                         settings.value(settingShapefileName,
                                                        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString());

        if(path.isEmpty())
            return;

        ui->shapefileLineEdit->setText(path);
    });
    connect(ui->shapefileLineEdit, &QLineEdit::textChanged, [=](QString text) {
        // record the file path
        _shapefilePath = text;

        // clear the field
        ui->shapefileComboBox->clear();
        ui->shapefileDescriptionComboBox->clear();
        ui->shapefileX1ComboBox->clear();
        ui->shapefileY1ComboBox->clear();
        ui->shapefileX2ComboBox->clear();
        ui->shapefileY2ComboBox->clear();

        _roadLinkIdx->clear();

        // populate the fields
        QStringList headers = getHeaders(text);
        if(!headers.isEmpty()) {
            toggleBoldFont(ui->shapefileLineEdit, true);
            ui->shapefileComboBox->addItems(headers);

            // select the saved header identifier (with settings)
            QSettings settings;
            QString settingName = settingShapefileName+"-"+QFileInfo(text).fileName();
            if(settings.contains(settingName+"-join")) {
                ui->shapefileComboBox->setCurrentIndex(settings.value(settingName+"-join").toInt());
            }

            QFileInfo info(text);
            if(info.suffix() == "csv" || info.suffix() == "txt") {
                ui->shapefileGridWidget->setVisible(true);

                ui->shapefileDescriptionComboBox->addItems(headers);
                ui->shapefileX1ComboBox->addItems(headers);
                ui->shapefileY1ComboBox->addItems(headers);
                ui->shapefileX2ComboBox->addItems(headers);
                ui->shapefileY2ComboBox->addItems(headers);

                // select the saved header identifier (with settings)
                settingName = settingShapefileName+"-"+QFileInfo(text).fileName();
                if(settings.contains(settingName+"-desc")) {
                    ui->shapefileDescriptionComboBox->setCurrentIndex(settings.value(settingName+"-desc").toInt());
                }
                if(settings.contains(settingName+"-x1")) {
                    ui->shapefileX1ComboBox->setCurrentIndex(settings.value(settingName+"-x1").toInt());
                }
                if(settings.contains(settingName+"-y1")) {
                    ui->shapefileY1ComboBox->setCurrentIndex(settings.value(settingName+"-y1").toInt());
                }
                if(settings.contains(settingName+"-x2")) {
                    ui->shapefileX2ComboBox->setCurrentIndex(settings.value(settingName+"-x2").toInt());
                }
                if(settings.contains(settingName+"-y2")) {
                    ui->shapefileY2ComboBox->setCurrentIndex(settings.value(settingName+"-y2").toInt());
                }
            } else {
                ui->shapefileGridWidget->setVisible(false);
            }

        } else {
            toggleBoldFont(ui->shapefileLineEdit, false);
            ui->shapefileGridWidget->setVisible(false);
        }

        checkConsistency();
    });

    connect(ui->shapefileComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _roadLinkIdx->join = idx - 1;
        checkConsistency();
    });
    connect(ui->shapefileDescriptionComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _roadLinkIdx->description = idx - 1;
        checkConsistency();
    });
    connect(ui->shapefileX1ComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _roadLinkIdx->x1 = idx - 1;
        checkConsistency();
    });
    connect(ui->shapefileY1ComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _roadLinkIdx->y1 = idx - 1;
        checkConsistency();
    });
    connect(ui->shapefileX2ComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _roadLinkIdx->x2= idx - 1;
        checkConsistency();
    });
    connect(ui->shapefileY2ComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _roadLinkIdx->y2 = idx - 1;
        checkConsistency();
    });

    /* Data */
    ui->shapefileLabel->setBuddy(ui->shapefileLineEdit);

    connect(ui->dataButton, &QPushButton::clicked, [=] () {
        QSettings settings;
        QFileDialog d(this, "Open data");
        QString path = d.getOpenFileName(this,
                                         "Open data",
                                         settings.value(settingDataName,
                                                        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString());

        if(path.isEmpty())
            return;

        ui->dataLineEdit->setText(path);
    });
    connect(ui->dataLineEdit, &QLineEdit::textChanged, [=](QString text) {
        // record the file path
        _dataPath = text;

        // clear fields
        _dataIdx->clear();
        ui->dataComboBox->clear();
        ui->dataDescriptionComboBox->clear();
        ui->dataDateComboBox->clear();
        ui->dataTimePeriodComboBox->clear();
        ui->dataSpeedComboBox->clear();
        ui->dataJourneyTimeComboBox->clear();
        ui->dataFlowComboBox->clear();
        ui->dataLengthComboBox->clear();

        // populate the fields
        QStringList headers = getHeaders(text);
        if(!headers.isEmpty()) {
            toggleBoldFont(ui->dataLineEdit, true);
            ui->dataComboBox->addItems(headers);
            ui->dataDescriptionComboBox->addItems(headers);
            ui->dataDateComboBox->addItems(headers);
            ui->dataTimePeriodComboBox->addItems(headers);
            ui->dataSpeedComboBox->addItems(headers);
            ui->dataJourneyTimeComboBox->addItems(headers);
            ui->dataFlowComboBox->addItems(headers);
            ui->dataLengthComboBox->addItems(headers);

            // select the saved header identifier (with settings)
            QSettings settings;
            QString settingName = settingDataName+"-"+QFileInfo(text).fileName();
            if(settings.contains(settingName+"-join")) {
                ui->dataComboBox->setCurrentIndex(settings.value(settingName+"-join").toInt());
            }
            if(settings.contains(settingName+"-desc")) {
                ui->dataDescriptionComboBox->setCurrentIndex(settings.value(settingName+"-desc").toInt());
            }
            if(settings.contains(settingName+"-date")) {
                ui->dataDateComboBox->setCurrentIndex(settings.value(settingName+"-date").toInt());
            }
            if(settings.contains(settingName+"-tp")) {
                ui->dataTimePeriodComboBox->setCurrentIndex(settings.value(settingName+"-tp").toInt());
            }
            if(settings.contains(settingName+"-speed")) {
                ui->dataSpeedComboBox->setCurrentIndex(settings.value(settingName+"-speed").toInt());
            }
            if(settings.contains(settingName+"-jt")) {
                ui->dataJourneyTimeComboBox->setCurrentIndex(settings.value(settingName+"-jt").toInt());
            }
            if(settings.contains(settingName+"-flow")) {
                ui->dataFlowComboBox->setCurrentIndex(settings.value(settingName+"-flow").toInt());
            }
            if(settings.contains(settingName+"-len")) {
                ui->dataLengthComboBox->setCurrentIndex(settings.value(settingName+"-len").toInt());
            }
        } else {
            toggleBoldFont(ui->dataLineEdit, false);
        }

        checkConsistency();
    });

    connect(ui->dataComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _dataIdx->join = idx-1;
        checkConsistency();
    });
    connect(ui->dataDescriptionComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _dataIdx->description = idx-1;
        checkConsistency();
    });
    connect(ui->dataDateComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _dataIdx->date = idx-1;
        checkConsistency();
    });
    connect(ui->dataTimePeriodComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _dataIdx->timePeriod = idx-1;
        checkConsistency();
    });
    connect(ui->dataSpeedComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _dataIdx->speed = idx-1;
        checkConsistency();
    });
    connect(ui->dataJourneyTimeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _dataIdx->journeyTime = idx-1;
        checkConsistency();
    });
    connect(ui->dataFlowComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _dataIdx->flow = idx-1;
        checkConsistency();
    });
    connect(ui->dataLengthComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx) {
        _dataIdx->length = idx-1;
        checkConsistency();
    });

    /* Button box */
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &RoadTrafficOpenDialog::done);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, [=]() {
        QDialog::reject();
    });


    /* Restore previous saved values */
    QSettings settings;
    if(settings.contains(settingShapefileName)) {
        ui->shapefileLineEdit->setText(settings.value(settingShapefileName).toString());
    }
    if(settings.contains(settingDataName)) {
        ui->dataLineEdit->setText(settings.value(settingDataName).toString());
    }

    checkConsistency();
}

bool RoadTrafficOpenDialog::checkConsistency() {
    bool flag = false;

    if(_roadLinkIdx->join == -1 || _dataIdx->date == -1
       || _dataIdx->join == -1 || _dataIdx->timePeriod == -1) {
        flag = true;
    }

    if(ui->shapefileGridWidget->isVisible() &&
            (_roadLinkIdx->join == -1 || _roadLinkIdx->x1 == -1
             || _roadLinkIdx->y1 == -1 || _roadLinkIdx->x2 == -1
             || _roadLinkIdx->y2 == -1)) {
        flag = true;
    }

    // Disable the "OK" button depending on the final value of flag
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(flag);
    return flag;
}

void RoadTrafficOpenDialog::saveFields() {
    /* Save both files */
    QSettings settings;
    QString settingShapefileName = "savedRoadTrafficOpenDialog-"+_name+"-shapefile";
    QString settingDataName      = "savedRoadTrafficOpenDialog-"+_name+"-data";

    settings.setValue(settingShapefileName, _shapefilePath);
    settings.setValue(settingDataName, _dataPath);

    QString settingName = settingShapefileName+"-"+QFileInfo(_shapefilePath).fileName();
    settings.setValue(settingName+"-join", ui->shapefileComboBox->currentIndex());
    settings.setValue(settingName+"-desc", ui->shapefileDescriptionComboBox->currentIndex());
    settings.setValue(settingName+"-x1", ui->shapefileX1ComboBox->currentIndex());
    settings.setValue(settingName+"-y1", ui->shapefileY1ComboBox->currentIndex());
    settings.setValue(settingName+"-x2", ui->shapefileX2ComboBox->currentIndex());
    settings.setValue(settingName+"-y2", ui->shapefileY2ComboBox->currentIndex());

    settingName = settingDataName+"-"+QFileInfo(_dataPath).fileName();
    settings.setValue(settingName+"-join", ui->dataComboBox->currentIndex());
    settings.setValue(settingName+"-desc", ui->dataDescriptionComboBox->currentIndex());
    settings.setValue(settingName+"-date", ui->dataDateComboBox->currentIndex());
    settings.setValue(settingName+"-tp", ui->dataTimePeriodComboBox->currentIndex());
    settings.setValue(settingName+"-speed", ui->dataSpeedComboBox->currentIndex());
    settings.setValue(settingName+"-jt", ui->dataJourneyTimeComboBox->currentIndex());
    settings.setValue(settingName+"-flow", ui->dataFlowComboBox->currentIndex());
    settings.setValue(settingName+"-len", ui->dataLengthComboBox->currentIndex());
}

void RoadTrafficOpenDialog::done() {
    saveFields();
    QDialog::accept();
}

QStringList RoadTrafficOpenDialog::getCSVHeaders(const QString& file) {
    /* process the file */
    QFile f(file);
    if (!f.open(QFile::ReadOnly | QFile::Text))
        return QStringList();

    // read the header
    QString line = QString(f.readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
    // get the most probable CSV delimiter
    QString del = determineCSVDelimiter(line);
    QStringList fields;
    fields << "" << line.split(del);
    return fields;
}

QStringList RoadTrafficOpenDialog::getShapefileHeaders(const QString& file) {
    /* process the shapefile */
    OGRRegisterAll();
    OGRDataSource* poDS;


    poDS = OGRSFDriverRegistrar::Open(file.toStdString().c_str(), FALSE);
    if( poDS == NULL ) {
        qWarning() << "Open failed for " << file;
        return QStringList();
    }

    if(poDS->GetLayerCount() == 0) {
        qWarning() << "No layers for " << file;
        return QStringList();
    }

    OGRLayer* poLayer = poDS->GetLayer(0);
    poLayer->ResetReading();
    OGRFeature* poFeature = poLayer->GetNextFeature();
    if(poFeature == NULL) {
        qWarning() << "No features for " << file;
        return QStringList();
    }

    QStringList fields;
    OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();
    fields << "";
    /* populate the fields */
    for(int iField = 0; iField < poFDefn->GetFieldCount(); iField++ ) {
        OGRFieldDefn* poFieldDefn = poFDefn->GetFieldDefn( iField );
        fields.append(poFieldDefn->GetNameRef());
    }

    return fields;
}

QStringList RoadTrafficOpenDialog::getHeaders(const QString& file) {
    // get file extension
    QFileInfo info(file);
    if(info.suffix() == "csv" || info.suffix() == "txt") {
        // CSV file
        return getCSVHeaders(file);
    }
    else if(info.suffix() == "shp") {
        // Shapefile file
        return getShapefileHeaders(file);
    }

    return QStringList();
}