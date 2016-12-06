//
// Created by Benjamin Baron on 21/09/2016.
//

#include "road_traffic.h"

bool RoadTraffic::open(Loader* loader) {

    /* First, load the shapefile */
    qDebug() << "load the shapefile";
    Shapefile::open(nullptr);
    if(loader != nullptr)
        loader->loadProgressChanged(0.3, "Shapefile loaded");

    /* continue by loading road traffic data */
    qDebug() << "load the road traffic data";
    bool ret = openDataset(_dataPath, loader);

    qDebug() << "loaded road traffic with" << _roadLinks.size() << "roadlinks";

    if(loader != nullptr)
        loader->loadProgressChanged(1.0, "Done");

    return ret;
}

bool RoadTraffic::openDataset(const QString& roadTrafficFile, Loader* loader) {
    /* process the data file second */
    QFile file(_dataPath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    // skip the header
    QString line = QString(file.readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
    // get the most probable CSV delimiter
    QString del = determineCSVDelimiter(line);

    while (!file.atEnd()) {
        line = QString(file.readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
        QStringList fields = line.split(del);
        QString joinIdx = fields.at(_dataIdx->join);
        QString description = _dataIdx->description == -1 ? "" : fields.at(_dataIdx->description);
        QDate date = QDate::fromString(fields.at(_dataIdx->date).split(" ")[0], "yyyy-MM-dd"); // format "[yyyy-MM-dd] HH:MM:SS"
        int timePeriod = fields.at(_dataIdx->timePeriod).toInt();          // [0, 95]
        double journeyTime = _dataIdx->journeyTime == -1 ? -1.0 : fields.at(_dataIdx->journeyTime).toDouble();   // in seconds
        double speed = _dataIdx->speed == -1 ? -1.0 : fields.at(_dataIdx->speed).toDouble();       // in km/h
        double length = _dataIdx->length == -1 ? -1.0 : fields.at(_dataIdx->length).toDouble();    // km
        double flow = _dataIdx->flow == -1 ? -1.0 : fields.at(_dataIdx->flow).toDouble();          // number of observed vehicles

//        qDebug() << _dataIdx->join << _dataIdx->description << _dataIdx->date
//                 <<  _dataIdx->timePeriod << _dataIdx->journeyTime << _dataIdx->speed << _dataIdx->length << _dataIdx->flow;

//        qDebug() << joinIdx << description << date << timePeriod << journeyTime << speed << length << flow;


        if(flow != -1.0) {
            addTrafficData(joinIdx, new RoadTrafficData(joinIdx, _roadLinks.value(joinIdx),
                                                        date, timePeriod, RoadTrafficDataType::FlowRTDType, flow));
        }
        if(journeyTime != -1.0) {
            addTrafficData(joinIdx, new RoadTrafficData(joinIdx, _roadLinks.value(joinIdx),
                                                        date, timePeriod, RoadTrafficDataType::JourneyTimeRTDType, journeyTime));
        }
        if(speed != -1.0) {
            addTrafficData(joinIdx, new RoadTrafficData(joinIdx, _roadLinks.value(joinIdx),
                                                        date, timePeriod, RoadTrafficDataType::SpeedRTDType, speed));
        }

        _roadLinks.value(joinIdx)->setLinkLength(length);

        // Indicate the load progress of the file
        if(loader != nullptr) {
            loader->loadProgressChanged(0.3 + 0.6*(1.0 - file.bytesAvailable() / (qreal) file.size()), "Loading road traffic data...");
        }
    }

    file.close();

    return true;
}

