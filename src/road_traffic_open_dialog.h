//
// Created by Benjamin Baron on 21/09/2016.
//

#ifndef LOCALL_ROAD_TRAFFIC_OPEN_DIALOG_H
#define LOCALL_ROAD_TRAFFIC_OPEN_DIALOG_H


#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include "shapefile.h"

struct RoadTrafficDataIndexes {
    RoadTrafficDataIndexes(int join = -1, int desc = -1,
                           int date = -1, int tp = -1,
                           int jt = -1, int len = -1,
                           int f = -1, int s = -1) {
        this->join = join;
        this->description = desc;
        this->date = date;
        this->timePeriod = tp;
        this->journeyTime = jt;
        this->length = len;
        this->flow = f;
        this->speed = s;
    }
    void clear() {
        this->join = -1;
        this->description = -1;
        this->date = -1;
        this->timePeriod = -1;
        this->journeyTime = -1;
        this->length = -1;
        this->flow = -1;
        this->speed = -1;
    }
    int join;
    int description;
    int date;
    int timePeriod;
    int journeyTime;
    int length;
    int flow;
    int speed;
};

namespace Ui {
    class RoadTrafficOpenDialog;
}

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
    RoadTrafficDataIndexes* getDataIdx() {
        return _dataIdx;
    }
    ShapefileIndexes* getRoadLinkIdx() {
        return _roadLinkIdx;
    }

protected slots:
    void done();

protected:
    Ui::RoadTrafficOpenDialog* ui;

    QString _name;
    QString _shapefilePath;
    QString _dataPath;
    RoadTrafficDataIndexes* _dataIdx;
    ShapefileIndexes* _roadLinkIdx;

    QStringList getCSVHeaders(const QString& file);
    QStringList getShapefileHeaders(const QString& file);
    QStringList getHeaders(const QString& file);
    virtual bool checkConsistency();
    virtual void saveFields();
};


#endif //LOCALL_ROAD_TRAFFIC_OPEN_DIALOG_H
