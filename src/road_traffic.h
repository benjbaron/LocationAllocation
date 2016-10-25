//
// Created by Benjamin Baron on 21/09/2016.
//

#ifndef LOCALL_ROAD_TRAFFIC_H
#define LOCALL_ROAD_TRAFFIC_H


#include <QString>
#include "loader.h"

enum RoadTrafficType { RT_GB, RT_ENG };

struct RoadTrafficData {
    RoadTrafficData(QString id, QString desc, QDate date = QDate(), int timePeriod = -1, double journeyTime = 0.0,
                     double speed = 0.0, int dataQuality = -1, double length = 0.0, double flow = 0.0) {
        this->id = id;
        this->description = desc;
        this->date = date;
        this->timePeriod = timePeriod;
        this->speed = speed;
        this->journeyTime = journeyTime;
        this->dataQuality = dataQuality;
        this->length = length;
        this->flow = flow;
        this->count = (dataQuality == -1) ? 0 : 1;
    }
    RoadTrafficData(RoadTrafficData* rtd) {
        this->id = rtd->id;
        this->description = rtd->description;
        this->date = rtd->date;
        this->timePeriod = rtd->timePeriod;
        this->speed = rtd->speed;
        this->journeyTime = rtd->journeyTime;
        this->dataQuality = rtd->dataQuality;
        this->length = rtd->length;
        this->flow = rtd->flow;
        this->count = rtd->count;
    }

    void computeMeanValues(RoadTrafficData* rtdNew) {
        /* add data from rtdNew to current rtd */
        speed       = (speed       * count + rtdNew->speed)       / (count + 1);
        journeyTime = (journeyTime * count + rtdNew->journeyTime) / (count + 1);
        flow        = (flow        * count + rtdNew->flow)        / (count + 1);
        count++;
    }

    QString id;
    QString description;
    QDate date;
    int timePeriod;
    double journeyTime = 0.0;
    double speed = 0.0;
    int dataQuality = 0;
    double length = 0.0;
    double flow = 0.0;
    int count = 0;
};

class RoadLink {
public:
    RoadLink(QString id, QString description, double x1, double y1, double x2, double y2):
            _id(id), _description(description), _p1(x1,y1), _p2(x2,y2),
            _meanRoadTrafficData(id,description) {}

    QString toString() {
        return "id: " + _id +
               " (" + QString::number(_p1.x()) +
               ", " + QString::number(_p1.y()) +
               ") -> (" + QString::number(_p2.x()) +
               ", " + QString::number(_p2.y()) +
               ")";
    }

    QLineF getLine() {
        return QLineF(_p1.x(), _p1.y(), _p2.x(), _p2.y());
    }

    QString getId() {
        return _id;
    }

    void addRoadTrafficData(QDate d, int timePeriod, RoadTrafficData* td) {
        if(!_roadTrafficData.contains(d)) {
            _roadTrafficData.insert(d, new QMap<int,RoadTrafficData*>());
            _meanRoadTrafficDataPerDate.insert(d, new RoadTrafficData(_id,_description,d));

        }
        _roadTrafficData.value(d)->insert(timePeriod, td);
        _meanRoadTrafficDataPerDate.value(d)->computeMeanValues(td);
        _meanRoadTrafficData.computeMeanValues(td);
    }

    QMap<QDate,QMap<int,RoadTrafficData*>*>* getRoadTrafficData() {
        return &_roadTrafficData;
    }

    QMap<int,RoadTrafficData*>* getRoadTrafficDataFromDate(QDate d) {
        if(!_roadTrafficData.contains(d))
            return nullptr;
        return _roadTrafficData.value(d);
    }

    RoadTrafficData* getMeanRoadTrafficDataPerDate(QDate d) {
        if(_meanRoadTrafficDataPerDate.contains(d))
            return _meanRoadTrafficDataPerDate.value(d);
        return nullptr;
    }

    RoadTrafficData* getMeanRoadTrafficData() {
        return &_meanRoadTrafficData;
    }

private:
    QPointF _p1, _p2;
    QString _id, _description;
    QMap<QDate, QMap<int,RoadTrafficData*>*> _roadTrafficData;
    QMap<QDate, RoadTrafficData*> _meanRoadTrafficDataPerDate;
    RoadTrafficData _meanRoadTrafficData;
};


class RoadNet {
public:
    RoadNet() { }
    ~RoadNet() { qDeleteAll(_roadLinks); }
    void addLink(QString id, QString description, double x1, double y1, double x2, double y2) {
        if(!_roadLinks.contains(id)) {
            RoadLink* roadLink = new RoadLink(id, description, x1, y1, x2, y2);
            _roadLinks.insert(id, roadLink);
        }
    }

    void addTrafficData(QString id, RoadTrafficData* td) {
        if(_roadLinks.contains(id)) {
            _roadLinks.value(id)->addRoadTrafficData(td->date, td->timePeriod, td);
        } else {
            qDebug() << "Network does not contain link " << id;
        }
    }

    bool isEmpty() {
        return _roadLinks.isEmpty();
    }

    void getRoadLinks(QHash<QString, RoadLink*>* roadLinks) {
        *roadLinks = _roadLinks;
    }

private:
    QHash<QString, RoadLink*> _roadLinks;
};


class RoadTraffic {
public:
    RoadTraffic(const QString& shapefilePath, const QString& dataPath, RoadTrafficType type):
            _shapefilePath(shapefilePath), _dataPath(dataPath) {
        // instantiate the road network object
        _roadNet = new RoadNet;
    }

    virtual bool open(Loader* loader);

    /* getters and setters */
    void getLinks(QHash<QString, RoadLink*>* links) {
        _roadNet->getRoadLinks(links);
    }

private:
    /* open different road traffic formats */
    bool openEnglandDataset(const QString& linkCoordinates, const QString& roadTraffic, Loader* loader);
    bool openGBDataset(const QString& shapefile, const QString& roadTraffic, Loader* loader) {
        return true;
    }



protected:
    const QString _shapefilePath;
    const QString _dataPath;
    RoadNet* _roadNet;
};

#endif //LOCALL_ROAD_TRAFFIC_H
