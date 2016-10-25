//
// Created by Benjamin Baron on 19/10/2016.
//

#include "waze_alert_file.h"
#include "proj_factory.h"

bool WazeAlertFile::open(Loader* loader) {

    /* process the Waze alert file first */
    QFile *file = new QFile(_wazeAlertFile);
    if (!file->open(QFile::ReadOnly | QFile::Text))
        return false;

    // skip the header
    file->readLine();

    // date format: "02/23/2015 12:10:00 AM"
    QString dateFormat = "MM/dd/yyyy hh:mm:ss AP";


    // header:
    // inject_date,street,city,roadType,pubMillis,locy,locx,subtype,reliability,uuid,type,reportRating,
    // magvar,country,startTime,startTimeMillis,endTime,endTimeMillis
    while (!file->atEnd()) {
        QString line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
        QStringList fields = line.split(",");
        QString date = fields.at(0);
        QString uuid = fields.at(9);

        double lon = fields.at(6).toDouble();
        double lat = fields.at(5).toDouble();
        int rating = fields.at(11).toInt();
        int roadType = fields.at(3).toInt();
        QString type = fields.at(10);
        QString subType = fields.at(8);
        WazeAlertType alertType = stringToAlertType(type);

        // convert the points to fit with the coordinate system
        double x, y;
        ProjFactory::getInstance().transformCoordinates(lat, lon, &x, &y);

        // convert the date
        QDateTime dateTime = QDateTime::fromString(date, dateFormat);
        long long timestamp = dateTime.toMSecsSinceEpoch();

        WazeAlert* wazeAlert = new WazeAlert(uuid,QPointF(x,y),alertType,subType,timestamp,roadType,rating);
        if(!_alerts.contains(uuid)) {
            _alerts.insert(uuid, new QMap<long long, WazeAlert*>());
        }
        _alerts.value(uuid)->insert(timestamp, wazeAlert);

        // Indicate the load progress of the file
        loader->loadProgressChanged(1.0 - file->bytesAvailable() / (qreal) file->size(), "Loading file...");
    }

    loader->loadProgressChanged(1.0, "Done");

    return true;
}

