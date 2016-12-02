//
// Created by Benjamin Baron on 02/03/16.
//

#include "trace.h"
#include "proj_factory.h"
#include "loader.h"
#include "geometry_index.h"

bool Trace::openTrace(Loader* loader) {
    if(_filename.contains("test")) {
        QFile* file = new QFile(_filename);
        if(!file->open(QFile::ReadOnly | QFile::Text))
            return false;

        while(!file->atEnd()) {
            QString line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
            QStringList fields = line.split(";");
            QString node = fields.at(1);
            long long ts = (long long) fields.at(0).toDouble();
            double lat   = fields.at(3).toDouble();
            double lon   = fields.at(2).toDouble();
//            qDebug() << "(" << node << "," << ts << "," << lat << "," << lon << ")";
            if(ts <= 0)
                continue;
            // convert the points to the local projection
            double x, y;
            ProjFactory::getInstance().transformCoordinates(lat, lon, &x, &y);
//            qDebug() << "adding node" << node << "(" << x << "," << y << "," << ts << ")";
            addPoint(node, ts, x, y);
            loader->loadProgressChanged(1.0 - file->bytesAvailable() / (qreal)file->size(), "");
        }
    } else if (_filename.contains("ONE")) {
        QFile* file = new QFile(_filename);
        if(!file->open(QFile::ReadOnly | QFile::Text))
            return false;

        // read first line
        // minTime maxTime minX maxX minY maxY
        if(!file->atEnd())
            file->readLine();

        while(!file->atEnd()) {
            QString line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
            QStringList fields = line.split(" ");
            QString node = fields.at(1);
            long long ts = fields.at(0).toLongLong();
            double lat   = fields.at(3).toDouble();
            double lon   = fields.at(2).toDouble();
//            qDebug() << "(" << node << "," << ts << "," << lat << "," << lon << ")";
            if(ts <= 0)
                continue;
            // convert the points to the local projection
            double x, y;
            ProjFactory::getInstance().transformCoordinates(lat, lon, &x, &y);
//            qDebug() << "adding node" << node << "(" << x << "," << y << "," << ts << ")";
            addPoint(node, ts, x, y);
            loader->loadProgressChanged(1.0 - file->bytesAvailable() / (qreal)file->size(), "");
        }
    } else if(_filename.contains("cabspotting")) {

        // "filename" is the repertory of the files
        QDirIterator it(_filename, QStringList() << "new_*.txt", QDir::Files, QDirIterator::Subdirectories);
        int count = 0;
        QList<QString> filenames;
        while (it.hasNext()) {
            filenames.append(it.next());
            count++;
        }
        for(int i = 0; i < count; ++i) {
            openNodeTrace(filenames.at(i));
            loader->loadProgressChanged((qreal) i / (qreal) count, "");
        }
    } else if(_filename.contains("gps_logs")) {

        QDirIterator it(_filename, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        int count = 0;
        QList<QString> nodeNames;
        while (it.hasNext()) {
            QString folder = it.next();
            nodeNames.append(folder);
            count++;
        }
        for(int i = 0; i < count; ++i) {
            openDieselNetNodeFolder(nodeNames.at(i));
            loader->loadProgressChanged((qreal) i / (qreal) count, "");
        }
    }

    qDebug() << "[DONE] loading file" << _filename << _startTime << _endTime;
    loader->loadProgressChanged((qreal)1.0, "Done");

    return true;
}

void Trace::openNodeTrace(QString filename) {
    // opens a node trace of format
    // [latitude (double), longitude (double), occupancy (int), time (long long)]
    QFile* file = new QFile(filename);
//    QRegExp rx("^new\\_(.*?)\\.txt$");
    QRegExp rx("new\\_(\\w+).txt");
    rx.indexIn(QFileInfo(filename).fileName());
    QString node = rx.cap(1);

    if(!file->open(QFile::ReadOnly | QFile::Text)) {
        return;
    }
    while(!file->atEnd())
    {
        QString line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
        QStringList fields = line.split(" ");
        double lat = fields.at(0).toDouble();
        double lon = fields.at(1).toDouble();
        long long ts = fields.at(3).toLongLong();
        if(ts <= 0)
            continue;
        // convert the points to the local projection
        double x, y;
        ProjFactory::getInstance().transformCoordinates(lat, lon, &x, &y);
//        qDebug() << "adding node" << node << "(" << x << "," << y << "," << ts << ")";
        addPoint(node, ts, x, y);
    }
}

void Trace::openDieselNetNodeFolder(QString dirname) {
    // read the files of the directory
//    qDebug() << "folder" << dirname;
    QDirIterator it(dirname, QStringList() << "*", QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QString node = QDir(dirname).dirName();
    while (it.hasNext()) {
        openDieselNetNodeTrace(it.next(), node);
    }
}

void Trace::openDieselNetNodeTrace(QString filename, QString node) {
    // get the date
//    qDebug() << "\tfile" << filename;
    QRegExp rx("(\\d{4})\\-(\\d{2})\\-(\\d{2})");  // date
    QRegExp rx1("(\\d{2})\\:(\\d{2})\\:(\\d{2})"); // time
    rx.indexIn(QFileInfo(filename).fileName());
    int year = rx.cap(1).toInt();
    int month = rx.cap(2).toInt();
    int day = rx.cap(3).toInt();

    // read the content of the file
    QFile* file = new QFile(filename);
    if(!file->open(QFile::ReadOnly | QFile::Text)) {
        return;
    }
    while(!file->atEnd()) {
        QString line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
        QStringList fields = line.split(" ");
        QString ts = fields.at(0);
        double lat = fields.at(1).toDouble();
        double lon = fields.at(2).toDouble();

        rx1.indexIn(ts);
        int hh = rx1.cap(1).toInt();
        int mm = rx1.cap(2).toInt();
        int ss = rx1.cap(3).toInt();

        QDateTime d(QDate(year, month, day), QTime(hh, mm, ss));
        long long timestamp = (long long) d.toTime_t();

        if(timestamp <= 0 || lat == 0 || lon == 0)
            continue;
        // convert the points to the local projection
        double x, y;
        ProjFactory::getInstance().transformCoordinates(lat, lon, &x, &y);
//        qDebug() << "\t\t" << year << month << day << hh << mm << ss << " / " << ts << QFileInfo(filename).fileName();
//        qDebug() << "\t\tadding node" << node << "(" << x << "," << y << "," << d.toTime_t() << ")";
        addPoint(node, d.toTime_t(), x, y);
    }
}

double Trace::averageSpeed() {
    if(_averageSpeeds.isEmpty()) {
        // compute the average speed for each node
        for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
            _averageSpeeds.addValue((int) averageSpeed(it.key()));
        }
    }

    return _averageSpeeds.getAverage();
}

double Trace::averageSpeed(const QString& nodeId) {
    QMap<long long, QPointF>* trace = _nodes.value(nodeId);
    int count = 0;
    double sum = 0.0;
    auto jt = trace->begin();
    long long prevTS = jt.key();
    QPointF prevPos = jt.value();
    for(jt++; jt != trace->end(); ++jt) {
        long long curTS = jt.key();
        QPointF curPos = jt.value();
        double distance = qSqrt(qPow(prevPos.x() - curPos.x(), 2) + qPow(prevPos.y() - curPos.y(), 2));
        long long timeDiff = curTS - prevTS;
        sum += distance / timeDiff;
        count++;
//                qDebug() << prevPos << curPos << prevTS << curTS << distance << timeDiff << sum << count;
        prevPos = curPos;
        prevTS = curTS;
    }

    // add the average speed for the current node to the distribution
    return sum / count;
}

GeometryIndex* Trace::makeGeometryIndex(double sampling,
                                         double startTime,
                                         double endTime,
                                         double geometryCellsSize,
                                         GeometryType geometryType,
                                         QString geometryCirclesFile) {
    // build the ogrGeometry index
    QSet<Geometry*> geometries;

    /* build the cells from the trace */

    QSet<QPoint> cellGeometries;
    for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {

        if(it.value()->lastKey() < startTime) {
            continue;
        }

        auto jt = (startTime == -1) ? it.value()->begin() : it.value()->lowerBound(startTime);
        if(jt == it.value()->end()) {
            continue;
        }

        long long prevTimestamp = jt.key(); // previous timestamp
        QPointF prevPos = jt.value();       // previous position
        for(++jt; jt != it.value()->end(); ++jt) {
            // start from the second position
            long long timestamp = jt.key(); // current timestamp
            QPointF pos = jt.value();       // current position

            // number of intermediate positions (with the sampling)
            int nbPos = qMax(1, qCeil((timestamp - prevTimestamp) / sampling));
            for(int i = 1; i <= nbPos; ++i) {
                long long t = prevTimestamp + i*sampling; // get the sampling time
                QPointF p = (timestamp - t)*prevPos + (t - prevTimestamp)*pos;
                p /= (timestamp - prevTimestamp);

                if(endTime != -1 && t > endTime) break;

                QPoint cellIdx(qFloor(p.x() / geometryCellsSize), (int)qFloor(p.y() / geometryCellsSize));
                if(!cellGeometries.contains(cellIdx)) {
                    Geometry* geom = new Cell(cellIdx.x()*geometryCellsSize, cellIdx.y()*geometryCellsSize, geometryCellsSize);
                    cellGeometries.insert(cellIdx);
                    geometries.insert(geom);
                }

                prevPos = pos;
                prevTimestamp = timestamp;
            }
        }
    }

    // add the circles from the given file
    if(geometryType == CircleGeometryType) {
        // build the circles from the given file
        QFile* file = new QFile(geometryCirclesFile);
        if(!file->open(QFile::ReadOnly | QFile::Text))
            return 0;
        while(!file->atEnd()) {
            // line format: "x;y;radius"
            QString line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
            QStringList fields = line.split(";");
            double x = fields.at(0).toDouble();
            double y = fields.at(1).toDouble();
            double radius = fields.at(2).toDouble();
            Geometry* geom = new Circle(x,y,radius);
            geometries.insert(geom);
        }
    }

    for(Geometry* geom : geometries) {
        if(geom->getGeometryType() == CircleGeometryType) {
            qDebug() << geom->getCenter();
        }
    }

    GeometryIndex* geometryIndex = new GeometryIndex(geometries, geometryCellsSize);
    return geometryIndex;
}