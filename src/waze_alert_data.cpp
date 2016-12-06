//
// Created by Benjamin Baron on 19/10/2016.
//

#include "waze_alert_data.h"
#include "proj_factory.h"
#include "csv_parser.h"

bool WazeAlertData::open(Loader* loader) {
    QVector<QMap<QString, QString>> wazeAlertsList = QVector<QMap<QString, QString>>();

    loader->loadProgressChanged(0.0, "Loading file...");
    CSVParser::parseCSV(_wazeAlertFile, wazeAlertsList, ",");

    // date format: "02/23/2015 12:10:00 AM"
    QString dateFormat = "MM/dd/yyyy hh:mm:ss AP";

    QString fOutName = QFileInfo(_wazeAlertFile).baseName()+"-out.txt";
    QFile fOut(fOutName );
    if(!fOut.open(QFile::WriteOnly)) {
        qDebug() << "Unable to write in file "<< fOutName ;
        return false;
    }
    QTextStream out(&fOut);

    // header:
    // inject_date,street,city,roadType,pubMillis,locy,locx,subtype,reliability,uuid,type,reportRating,
    // magvar,country,startTime,startTimeMillis,endTime,endTimeMillis
    int count = 0;
    for(auto alert : wazeAlertsList) {
        QString date = alert.value("inject_date");
        QString uuid = alert.value("uuid");

        double lat = alert.value("locy").toDouble();
        double lon = alert.value("locx").toDouble();
        int rating = alert.value("reportRating").toInt();
        int roadType = alert.value("roadType").toInt();
        QString type = alert.value("type");
        QString subType = alert.value("subtype");
        int magvar = alert.value("magvar").toInt();
        QString street = alert.value("street");

        qDebug() << uuid << lat << lon << magvar << street;

        WazeAlertType alertType = stringToAlertType(type);

        // convert the points to fit with the coordinate system
        double x, y;
        ProjFactory::getInstance().transformCoordinates(lat, lon, &x, &y);

        out << uuid << ";" << QString::number(lat,'f',4) << ";" << QString::number(lon,'f',4)
            << ";" << QString::number(x,'f',4) << ";" << QString::number(y,'f',4) << "\n";

        // convert the date
        QDateTime dateTime = QDateTime::fromString(date, dateFormat);
        long long timestamp = dateTime.toMSecsSinceEpoch();

        WazeAlert* wazeAlert = new WazeAlert(uuid,QPointF(x,y),alertType,subType,timestamp,roadType,rating,magvar,street);
        if(!_alerts.contains(uuid)) {
            _alerts.insert(uuid, new QMap<long long, WazeAlert*>());
        }
        _alerts.value(uuid)->insert(timestamp, wazeAlert);

        // Indicate the load progress of the file
        double progressCount = count / (qreal) wazeAlertsList.size();
        loader->loadProgressChanged(progressCount, "Loading file...");
        count++;
    }

    fOut.close();

    loader->loadProgressChanged(1.0, "Done");

    return true;
}

