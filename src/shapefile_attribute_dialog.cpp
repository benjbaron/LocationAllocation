//
// Created by Benjamin Baron on 31/10/2016.
//

#include "shapefile_attribute_dialog.h"
#include "ui_shapefile_attribute_dialog.h"
#include "shapefile.h"
#include "shapefile_layer.h"

ShapefileAttributeDialog::ShapefileAttributeDialog(QWidget* parent, ShapefileLayer* shapefileLayer) :
        QDialog(parent), _shapefileLayer(shapefileLayer),
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

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->verticalHeader()->hide();

    QStandardItem* item = new QStandardItem("Index");
    _model->setHorizontalHeaderItem(0,item);
    for(int i = 0; i < featuresHeader->size(); ++i) {
        QString attribute = featuresHeader->at(i);
        item = new QStandardItem(attribute);
        _model->setHorizontalHeaderItem(i+1,item);
    }

    int j = 0;
    // populate the rows
    for(auto it = features->begin(); it != features->end(); ++it) {
        // add feature index
        int idx = it.key();
        ShapefileFeature* feature = it.value();

        // populate the columns of the row
        item = new QStandardItem(QString::number(idx));
        _model->setItem(j,0,item);
        _featureIdxToRowIdx.insert(idx,j);
        for(int i = 0; i < feature->attributes->size(); ++i) {
            QString value = feature->attributes->at(i);
            item = new QStandardItem(value);
            _model->setItem(j,i+1,item);
        }
        j++;
    }

    // connect the table selection
    connect(ui->tableView->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            [=](const QModelIndex &current, const QModelIndex &previous) {
        qDebug() << "row selected before" << previous.row() << "->" << current.row();
        if(current.row() == previous.row()) {
            return;
        }

        //get the new shape index
        int idx = ui->tableView->model()->data(current.sibling(current.row(), 0)).toInt();
        qDebug() << "\trowSelected" << idx;

        // emit a signal
        emit rowSelected(idx);
    });

    // connect the shapefile feature selection
    connect(_shapefileLayer, &ShapefileLayer::featureSelected, [=](int idx) {
        int rowIdx = _featureIdxToRowIdx.value(idx);
        qDebug() << "row to select" << rowIdx << "from feature id" << idx;
        ui->tableView->selectRow(rowIdx);
    });
}