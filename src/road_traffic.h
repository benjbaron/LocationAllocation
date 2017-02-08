//
// Created by Benjamin Baron on 21/09/2016.
//

#ifndef LOCALL_ROAD_TRAFFIC_H
#define LOCALL_ROAD_TRAFFIC_H


#include <QString>
#include "loader.h"
#include "road_traffic_open_dialog.h"

class RoadLink;

struct RoadTrafficData {
    RoadTrafficData(QString id, RoadLink* roadLink, QDate date = QDate(), int timePeriod = -1,
                    RoadTrafficDataType rtdType = NoneRTDType, double rtdValue = 0.0) {
        this->id = id;
        this->roadLink = roadLink;
        this->date = date;
        this->timePeriod = timePeriod;
        this->rtdType = rtdType;
        this->rtdValue = rtdValue;
        this->count = (timePeriod == -1) ? 0 : 1;
    }
    RoadTrafficData(RoadTrafficData* rtd) {
        this->id = rtd->id;
        this->roadLink = rtd->roadLink;
        this->date = rtd->date;
        this->timePeriod = rtd->timePeriod;
        this->rtdType = rtd->rtdType;
        this->rtdValue = rtd->rtdValue;
        this->count = rtd->count;
    }

    void computeMeanValues(RoadTrafficData* rtdNew) {
        /* add data from rtdNew to current rtd */
        rtdValue = (rtdValue * count + rtdNew->rtdValue) / (count + 1);
        count++;
    }

    double getValue() {
        return rtdValue;
    }

    QString id;
    RoadLink* roadLink;
    QDate date;
    int timePeriod;
    int count = 0;
    RoadTrafficDataType rtdType;
    double rtdValue;
};


struct RoadTrafficDataAggregate {
    RoadTrafficDataAggregate(RoadLink* rl, const QDate& d = QDate()) {
        this->roadLink = rl;
        this->date = d;
    }

    void addRoadTrafficData(RoadTrafficData* rtd) {
        RoadTrafficDataType rtdType = rtd->rtdType;
        int timePeriod = rtd->timePeriod;
        if(!roadTrafficData.contains(rtdType)) {
            roadTrafficData.insert(rtdType, new QMap<int, RoadTrafficData*>());
        }
        roadTrafficData.value(rtdType)->insert(timePeriod, rtd);
        computeMeanValues(rtd);
    }

    QStringList getAllRoadTrafficDataTypes() {
        QSet<QString> list;
        for(auto it = meanRoadTrafficData.begin(); it != meanRoadTrafficData.end(); ++it) {
            list << roadTrafficDataTypeToString(it.key());
        }
        return list.toList();
    }

    void computeMeanValues(RoadTrafficData* rtd) {
        RoadTrafficDataType rtdType = rtd->rtdType;
        if(!meanRoadTrafficData.contains(rtdType)) {
            meanRoadTrafficData.insert(rtdType, new RoadTrafficData(rtd->id, rtd->roadLink, rtd->date));
        }
        meanRoadTrafficData.value(rtdType)->computeMeanValues(rtd);
    }

    QMap<int, RoadTrafficData*>* getRoadTrafficData(RoadTrafficDataType rtdType) {
        if(roadTrafficData.contains(rtdType))
            return roadTrafficData.value(rtdType);
        return nullptr;
    }

    double getValue(RoadTrafficDataType rtdType, int timePeriod) {
        if(timePeriod == -1 && meanRoadTrafficData.contains(rtdType)) {
            return meanRoadTrafficData.value(rtdType)->getValue();
        }
        if(timePeriod != -1 && roadTrafficData.contains(rtdType))
            return roadTrafficData.value(rtdType)->value(timePeriod)->getValue();
        return 0.0;
    }

    double meanValue(RoadTrafficDataType rtdType) {
        if(meanRoadTrafficData.contains(rtdType))
            return meanRoadTrafficData.value(rtdType)->getValue();
        return 0.0;
    }
    int getSize(RoadTrafficDataType rtdType) {
        if(roadTrafficData.contains(rtdType))
            return roadTrafficData.value(rtdType)->size();
        return 0;
    }

    RoadLink* roadLink;
    QDate date;
    // indexed by timePeriod
    QHash<RoadTrafficDataType, QMap<int, RoadTrafficData*>*> roadTrafficData;
    // mean aggregate of the road traffic data per date
    QHash<RoadTrafficDataType, RoadTrafficData*> meanRoadTrafficData;
};


class RoadLink {
public:
    RoadLink(QString id, ShapefileFeature* shapefileFeature) :
            _id(id), _shapefileFeature(shapefileFeature),
            _meanRoadTrafficData(this) {}

    QString toString() {
        return "id: \"" + _id + "\" with " + _shapefileFeature->toString() + " features";
    }

    QString getId() {
        return _id;
    }

    void setLinkLength(double linkLength) {
        _linkLength = linkLength;
    }

    double getLinkLength() {
        return _linkLength;
    }

    void addRoadTrafficData(QDate d, int timePeriod, RoadTrafficData* rtd) {
        if(!_roadTrafficData.contains(d)) {
            _roadTrafficData.insert(d, new RoadTrafficDataAggregate(this, d));
        }
        _roadTrafficData.value(d)->addRoadTrafficData(rtd);
        _meanRoadTrafficData.computeMeanValues(rtd);
    }

    QMap<QDate,RoadTrafficDataAggregate*>* getRoadTrafficData() {
        return &_roadTrafficData;
    }

    RoadTrafficDataAggregate* getRoadTrafficDataAtDate(const QDate &d) {
        if(d.isValid() && _roadTrafficData.contains(d))
            return _roadTrafficData.value(d);
        return nullptr;
    }

    bool containsDataAtDate(const QDate& d) {
        return _roadTrafficData.contains(d);
    }

    double getMeanRoadTrafficDataPerDate(const QDate& d, RoadTrafficDataType rtdType) {
        if(d.isValid() && _roadTrafficData.contains(d))
            return _roadTrafficData.value(d)->meanValue(rtdType);
        return 0.0;
    }

    RoadTrafficDataAggregate* getMeanRoadTrafficData() {
        return &_meanRoadTrafficData;
    }

    ShapefileFeature* getShapefileFeature() {
        return _shapefileFeature;
    }
    int getMaxPeriod(RoadTrafficDataType rtdType) {
        int maxPeriod = 0;
        if(rtdType != RoadTrafficDataType::SpeedClassRTDType) {
            for(auto it = _roadTrafficData.begin(); it != _roadTrafficData.end(); ++it) {
                int nbPeriods = it.value()->getSize(rtdType);
                if(nbPeriods > maxPeriod) maxPeriod = nbPeriods;
            }
        } else {
            for (int speedClass = RoadTrafficDataType::Speed0RTDType;
                 speedClass <= RoadTrafficDataType::Speed14RTDType; ++speedClass) {
                int mp = getMaxPeriod((RoadTrafficDataType) speedClass);
                if(mp > maxPeriod) maxPeriod = mp;
            }
        }
        return maxPeriod;
    }
    QStringList getAllRoadTrafficDataTypes() {
        QStringList list = _meanRoadTrafficData.getAllRoadTrafficDataTypes();
        qDebug() << "\t[list|RoadLink]" << list;

        return list;
    }
    QDate getMinDate() {
        return _roadTrafficData.firstKey();
    }
    QDate getMaxDate() {
        return _roadTrafficData.lastKey();
    }

private:
    QString _id;
    double _linkLength = -1;
    ShapefileFeature* _shapefileFeature;
    // indexed by time period
    QMap<QDate, RoadTrafficDataAggregate*> _roadTrafficData;
    // mean aggregate of the road traffic data for the whole period
    RoadTrafficDataAggregate _meanRoadTrafficData;
};


class RoadTraffic : public Shapefile {
public:
    RoadTraffic(const QString& shapefilePath, const QString& dataPath, const QString& additionalPath,
                ShapefileIndexes* shapefileIdx, RoadTrafficDataIndexes* dataIdx):
        Shapefile(shapefilePath, shapefileIdx), _dataPath(dataPath),
        _additionalPath(additionalPath), _dataIdx(dataIdx) { }

    bool open(Loader* loader);

    /* getters and setters */
    QHash<QString, RoadLink*>* getRoadLinks() {
        return &_roadLinks;
    }

    RoadLink* getRoadLink(const QString& idx) {
        return _roadLinks.value(idx);
    }

    RoadLink* getRoadLink(int idx) {
        /* Get the roadlink corresponding to the shapefile index */
        int joinIdx = indexes()->join;
        QString rlIdx = getFeature(idx)->attributes->at(joinIdx);
        if(_roadLinks.contains(rlIdx))
            return _roadLinks.value(rlIdx);

        return nullptr;
    }

private:
    /* open different road traffic formats */
    bool openDataset(const QString& roadTrafficFile, Loader* loader);

    void addTrafficData(QString id, RoadTrafficData* td) {
        if(!_roadLinks.contains(id)) {
            ShapefileFeature* feature = getFeature(id);
            _roadLinks.insert(id, new RoadLink(id, feature));
            _featureToRoadLink.insert(feature->id, id);
        }
        _roadLinks.value(id)->addRoadTrafficData(td->date, td->timePeriod, td);
    }


protected:
    const QString _dataPath;
    const QString _additionalPath;
    RoadTrafficDataIndexes* _dataIdx;
    QHash<QString, RoadLink*> _roadLinks;
    QHash<int, QString> _featureToRoadLink;

};

#endif //LOCALL_ROAD_TRAFFIC_H
