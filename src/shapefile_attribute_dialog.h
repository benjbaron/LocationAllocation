//
// Created by Benjamin Baron on 31/10/2016.
//

#ifndef LOCALL_SHAPEFILE_ATTRIBUTE_DIALOG_H
#define LOCALL_SHAPEFILE_ATTRIBUTE_DIALOG_H

#include <QDialog>
#include "shapefile.h"

namespace Ui {
    class ShapefileAttributeDialog;
}

class ShapefileAttributeDialog : public QDialog {
    Q_OBJECT

public:
    explicit ShapefileAttributeDialog(QWidget* parent = 0);
    ~ShapefileAttributeDialog();

    void showShapefileAttributes(Shapefile* shapefile);

private:
    Ui::ShapefileAttributeDialog* ui;
    QStandardItemModel* _model;
};


#endif //LOCALL_SHAPEFILE_ATTRIBUTE_DIALOG_H
