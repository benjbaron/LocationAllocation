//
// Created by Benjamin Baron on 21/09/2016.
//

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSettings>
#include "road_traffic_open_dialog.h"

RoadTrafficOpenDialog::RoadTrafficOpenDialog(QWidget* parent, const QString& name) :
QDialog(parent), _name(name) {
    mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    QString settingShapefileName = "savedRoadTrafficOpenDialog-"+name+"-shapefile";
    QString settingDataName      = "savedRoadTrafficOpenDialog-"+name+"-data";

    /* Shapefile */
    QHBoxLayout* shapefileLayout = new QHBoxLayout;
    QLabel* shapefileLabel = new QLabel("Browse the shapefile");
    QLineEdit* shapefileLineEdit = new QLineEdit;
    shapefileLabel->setBuddy(shapefileLineEdit);
    QPushButton* shapefileFileButton = new QPushButton("Browse");

    connect(shapefileFileButton, &QPushButton::clicked, [=] () {
        QSettings settings;
        QFileDialog d(this, "Open a shapefile");
        QString path = d.getOpenFileName(this,
                                         "Open a shapefile",
                                         settings.value(settingShapefileName,
                                                        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString());

        if(path.isEmpty())
            return;

        shapefileLineEdit->setText(path);
    });
    connect(shapefileLineEdit, &QLineEdit::textChanged, [=](QString text) {
        // record the file path
        _shapefilePath = text;
    });
    shapefileLayout->addWidget(shapefileLineEdit);
    shapefileLayout->addWidget(shapefileFileButton);

    QWidget* shapefileExtension = new QWidget;
    shapefileExtension->setLayout(shapefileLayout);
    mainLayout->addWidget(shapefileLabel);
    mainLayout->addWidget(shapefileExtension);

    /* Data */
    QHBoxLayout* dataLayout = new QHBoxLayout;
    QLabel* dataLabel = new QLabel("Browse the data");
    QLineEdit* dataLineEdit = new QLineEdit;
    shapefileLabel->setBuddy(shapefileLineEdit);
    QPushButton* dataButton = new QPushButton("Browse");

    connect(dataButton, &QPushButton::clicked, [=] () {
        QSettings settings;
        QFileDialog d(this, "Open data");
        QString path = d.getOpenFileName(this,
                                         "Open data",
                                         settings.value(settingDataName,
                                                        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString());

        if(path.isEmpty())
            return;

        dataLineEdit->setText(path);
    });
    connect(dataLineEdit, &QLineEdit::textChanged, [=](QString text) {
        // record the file path
        _dataPath = text;
    });
    dataLayout->addWidget(dataLineEdit);
    dataLayout->addWidget(dataButton);

    QWidget* dataExtension = new QWidget;
    dataExtension->setLayout(dataLayout);
    mainLayout->addWidget(dataLabel);
    mainLayout->addWidget(dataExtension);


    /* Button box */
    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &RoadTrafficOpenDialog::done);
    connect(buttonBox, &QDialogButtonBox::rejected, [=]() {
        QDialog::reject();
    });

    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle(name);

    /* Restore previous saved values */
    QSettings settings;
    if(settings.contains(settingShapefileName)) {
        shapefileLineEdit->setText(settings.value(settingShapefileName).toString());
    }
    if(settings.contains(settingDataName)) {
        dataLineEdit->setText(settings.value(settingDataName).toString());
    }
}

void RoadTrafficOpenDialog::done() {
    /* Save both files */
    QSettings settings;
    settings.setValue("savedRoadTrafficOpenDialog-"+_name+"-shapefile", _shapefilePath);
    settings.setValue("savedRoadTrafficOpenDialog-"+_name+"-data", _dataPath);
    QDialog::accept();
}