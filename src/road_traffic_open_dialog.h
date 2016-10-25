//
// Created by Benjamin Baron on 21/09/2016.
//

#ifndef LOCALL_ROAD_TRAFFIC_OPEN_DIALOG_H
#define LOCALL_ROAD_TRAFFIC_OPEN_DIALOG_H


#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>

class RoadTrafficOpenDialog: public QDialog {
    Q_OBJECT

public:
    RoadTrafficOpenDialog(QWidget* parent = 0, const QString& name = "");
    QString getShapefilePath() {
        return _shapefilePath;
    }
    QString getDataPath() {
        return _dataPath;
    }

private slots:
    void done();

private:
    QString _name;
    QString _shapefilePath;
    QString _dataPath;
    QDialogButtonBox* buttonBox;
    QVBoxLayout* mainLayout;

};


#endif //LOCALL_ROAD_TRAFFIC_OPEN_DIALOG_H
