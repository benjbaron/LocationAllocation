#include "traceloader.h"

#include <QString>
#include "tracelayer.h"
#include "osrmwrapper.h"

TraceLoader::~TraceLoader() { }

bool TraceLoader::concurrentLoad()
{
    qDebug() << _filename;
    if(_filename.contains("test")) {
        QFile* file = new QFile(_filename);
        if(!file->open(QFile::ReadOnly | QFile::Text))
        {
            return 0;
        }
        while(!file->atEnd())
        {
            QString line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
            QStringList fields = line.split(";");
//            qDebug() << "line" << fields;
            QString node = fields.at(1);
            long long ts = (long long) fields.at(0).toDouble();
            double lat   = fields.at(2).toDouble();
            double lon   = fields.at(3).toDouble();
//            qDebug() << "(" << node << "," << ts << "," << lat << "," << lon << ")";
            if(ts <= 0)
                continue;
            // convert the points to the local projection
            double x, y;
            OSRMWrapper::getInstance().transformCoordinates(lat, lon, &x, &y);
//            qDebug() << "adding node" << node << "(" << x << "," << y << "," << ts << ")";
            ((TraceLayer*)_layer)->addPoint(node, ts, x, y);
        }
    }
    if(_filename.contains("cabspotting")) {
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
            emit loadProgressChanged((qreal) i / (qreal) count);
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
            emit loadProgressChanged((qreal) i / (qreal) count);
        }
    }
    emit loadProgressChanged((qreal)1.0);
    return 1;
}

void TraceLoader::openNodeTrace(QString filename)
{
    // opens a node trace of format
    // [latitude (double), longitude (double), occupancy (int), time (long long)]
    QFile* file = new QFile(filename);
//    QRegExp rx("^new\\_(.*?)\\.txt$");
    QRegExp rx("new\\_(\\w+).txt");
    rx.indexIn(QFileInfo(filename).fileName());
    QString node = rx.cap(1);

    if(!file->open(QFile::ReadOnly | QFile::Text))
    {
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
        OSRMWrapper::getInstance().transformCoordinates(lat, lon, &x, &y);
//        qDebug() << "adding node" << node << "(" << x << "," << y << "," << ts << ")";
        ((TraceLayer*)_layer)->addPoint(node, ts, x, y);
    }
}

void TraceLoader::openDieselNetNodeFolder(QString dirname)
{
    // read the files of the directory
//    qDebug() << "folder" << dirname;
    QDirIterator it(dirname, QStringList() << "*", QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QString node = QDir(dirname).dirName();
    while (it.hasNext()) {
        openDieselNetNodeTrace(it.next(), node);
    }
}

void TraceLoader::openDieselNetNodeTrace(QString filename, QString node)
{
    // get the date
//    qDebug() << "\tfile" << filename;
    QRegExp rx("(\\d{4})\\-(\\d{2})\-(\\d{2})");  // date
    QRegExp rx1("(\\d{2})\\:(\\d{2})\\:(\\d{2})"); // time
    rx.indexIn(QFileInfo(filename).fileName());
    int year = rx.cap(1).toInt();
    int month = rx.cap(2).toInt();
    int day = rx.cap(3).toInt();

    // read the content of the file
    QFile* file = new QFile(filename);
    if(!file->open(QFile::ReadOnly | QFile::Text))
    {
        return;
    }
    while(!file->atEnd())
    {
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
        OSRMWrapper::getInstance().transformCoordinates(lat, lon, &x, &y);
//        qDebug() << "\t\t" << year << month << day << hh << mm << ss << " / " << ts << QFileInfo(filename).fileName();
//        qDebug() << "\t\tadding node" << node << "(" << x << "," << y << "," << d.toTime_t() << ")";
        ((TraceLayer*)_layer)->addPoint(node, d.toTime_t(), x, y);
    }
}
