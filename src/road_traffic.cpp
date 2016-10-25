//
// Created by Benjamin Baron on 21/09/2016.
//

#include "road_traffic.h"
#include "proj_factory.h"

bool RoadTraffic::open(Loader* loader) {

    if(_shapefilePath.contains("ENG") && _dataPath.contains("ENG")) {
        bool ret = openEnglandDataset(_shapefilePath, _dataPath, loader);
        loader->loadProgressChanged(1.0, "Done");
        return ret;
    }

    return false;
}

bool RoadTraffic::openEnglandDataset(const QString& linkCoordinates, const QString& roadTraffic, Loader* loader) {

    /* process the shapefile first */
    QFile *file = new QFile(_shapefilePath);
    if (!file->open(QFile::ReadOnly | QFile::Text))
        return false;

    // skip the header
    file->readLine();

    while (!file->atEnd()) {
        QString line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
        QStringList fields = line.split(",");
        QString id = fields.at(0);
        QString description = fields.at(1);
        double lon1 = fields.at(4).toDouble();
        double lat1 = fields.at(5).toDouble();
        double lon2 = fields.at(6).toDouble();
        double lat2 = fields.at(7).toDouble();

        // convert the points to fit with the coordinate system
        double x1, y1, x2, y2;
        ProjFactory::getInstance().transformCoordinates(lat1, lon1, &x1, &y1);
        ProjFactory::getInstance().transformCoordinates(lat2, lon2, &x2, &y2);

        _roadNet->addLink(id, description, x1, y1, x2, y2);

        // Indicate the load progress of the file
        loader->loadProgressChanged(0.3 * (1.0 - file->bytesAvailable() / (qreal) file->size()), "");
    }

    file->close();

    /* process the data file second */
    file = new QFile(_dataPath);
    if (!file->open(QFile::ReadOnly | QFile::Text))
        return false;

    // skip the header
    file->readLine();

    while (!file->atEnd()) {
        QString line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
        QStringList fields = line.split(",");
        QString id = fields.at(0);                      // matches the link id of the shapefile
        QString description = fields.at(1);
        QDate date = QDate::fromString(fields.at(2).split(" ")[0], "yyyy-MM-dd"); // format "[yyyy-MM-dd] HH:MM:SS"
        int timePeriod = fields.at(3).toInt();          // [0, 95]
        double journeyTime = fields.at(4).toDouble();   // in seconds
        double speed = fields.at(5).toDouble();         // in km/h
        int dataQuality = fields.at(6).toInt();         // 1 (best) -> 5 (worst)
        double length = fields.at(7).toDouble();        // km
        double flow = fields.at(8).toDouble();          // number of observed vehicles

        RoadTrafficData* td = new RoadTrafficData(id, description, date, timePeriod,
                                                  journeyTime, speed, dataQuality, length, flow);
        _roadNet->addTrafficData(id, td);

        // Indicate the load progress of the file
        loader->loadProgressChanged(0.3 + 0.6*(1.0 - file->bytesAvailable() / (qreal) file->size()), "");
    }

    file->close();

    return true;
}