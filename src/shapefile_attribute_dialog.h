//
// Created by Benjamin Baron on 31/10/2016.
//

#ifndef LOCALL_SHAPEFILE_ATTRIBUTE_DIALOG_H
#define LOCALL_SHAPEFILE_ATTRIBUTE_DIALOG_H

#include <QDialog>
#include <QStandardItemModel>

class ShapefileLayer;
class Shapefile;

namespace Ui {
    class ShapefileAttributeDialog;
}

class ShapefileAttributeDialog : public QDialog {
    Q_OBJECT

public:
    explicit ShapefileAttributeDialog(QWidget* parent = 0, ShapefileLayer* shapefileLayer = nullptr);
    ~ShapefileAttributeDialog();

    void showShapefileAttributes(Shapefile* shapefile);

signals:
    void rowSelected(int);

private:
    Ui::ShapefileAttributeDialog* ui;
    QStandardItemModel* _model = nullptr;
    ShapefileLayer* _shapefileLayer = nullptr;
    QHash<int,int> _featureIdxToRowIdx;
    int counter = 0;
};


#endif //LOCALL_SHAPEFILE_ATTRIBUTE_DIALOG_H
