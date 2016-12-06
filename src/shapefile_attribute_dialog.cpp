//
// Created by Benjamin Baron on 31/10/2016.
//

#include "shapefile_attribute_dialog.h"
#include "ui_shapefile_attribute_dialog.h"

ShapefileAttributeDialog::ShapefileAttributeDialog(QWidget* parent) :
        QDialog(parent),
        ui(new Ui::ShapefileAttributeDialog) {
    ui->setupUi(this);
    _model = new QStandardItemModel();
    ui->tableView->setModel(_model);
}

ShapefileAttributeDialog::~ShapefileAttributeDialog() {
    delete ui;
}

void ShapefileAttributeDialog::showShapefileAttributes(Shapefile* shapefile) {
    QList<QString>* featuresHeader = shapefile->getAttributeHeader();
    QHash<int,ShapefileFeature*>* features = shapefile->getFeatures();

    for(int i = 0; i < featuresHeader->size(); ++i) {
        QString attribute = featuresHeader->at(i);
        QStandardItem* item = new QStandardItem(attribute);
        _model->setHorizontalHeaderItem(i,item);
    }

    int j = 0;
    for(auto it = features->begin(); it != features->end(); ++it) {
        ShapefileFeature* feature = it.value();
        for(int i = 0; i < feature->attributes->size(); ++i) {
            QString value = feature->attributes->at(i);
            QStandardItem* item = new QStandardItem(value);
            _model->setItem(j,i,item);
        }
        j++;
    }
}