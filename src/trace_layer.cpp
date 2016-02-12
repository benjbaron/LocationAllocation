#include "trace_layer.h"

#include "proj_factory.h"
#include "intermediate_pos_layer.h"
#include "spatial_stats_dialog.h"
#include "spatial_stats.h"

QGraphicsItemGroup *TraceLayer::draw() {
    int radius = 10;
    _groupItem = new QGraphicsItemGroup();

    for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
        for(auto jt = it.value()->begin(); jt != it.value()->end(); ++jt) {
            double x = jt.value().x();
            double y = jt.value().y();

            QGraphicsEllipseItem* item = new QGraphicsEllipseItem(x-radius, -1*(y-radius), radius*2, -1*(radius*2));
//            item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

            item->setBrush(QBrush(QColor(0,0,0)));
            item->setPen(Qt::NoPen);

            addGraphicsItem(item);
        }
    }

    return _groupItem;
}

double TraceLayer::getAverageSpeed()
{
    if(_averageSpeeds.isEmpty()) {
        // compute the average speed for each node
        for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
            int count = 0;
            double sum = 0.0;

            auto jt = it.value()->begin();
            long long prevTS = jt.key();
            QPointF prevPos = jt.value();
            for(jt++; jt != it.value()->end(); ++jt) {
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
            int value = (int) (sum / count);
            _averageSpeeds.addValue(value);
        }
    }

    return _averageSpeeds.getAverage();
}

bool TraceLayer::load(Loader* loader)
{
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
            emit loader->loadProgressChanged(1.0 - file->bytesAvailable() / (qreal)file->size());
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
            emit loader->loadProgressChanged((qreal) i / (qreal) count);
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
            emit loader->loadProgressChanged((qreal) i / (qreal) count);
        }
    }
    qDebug() << "[DONE] loading file";
    emit loader->loadProgressChanged((qreal)1.0);
    return 1;
}

void TraceLayer::openNodeTrace(QString filename)
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
        ProjFactory::getInstance().transformCoordinates(lat, lon, &x, &y);
//        qDebug() << "adding node" << node << "(" << x << "," << y << "," << ts << ")";
        addPoint(node, ts, x, y);
    }
}

void TraceLayer::openDieselNetNodeFolder(QString dirname)
{
    // read the files of the directory
//    qDebug() << "folder" << dirname;
    QDirIterator it(dirname, QStringList() << "*", QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QString node = QDir(dirname).dirName();
    while (it.hasNext()) {
        openDieselNetNodeTrace(it.next(), node);
    }
}

void TraceLayer::openDieselNetNodeTrace(QString filename, QString node)
{
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
        ProjFactory::getInstance().transformCoordinates(lat, lon, &x, &y);
//        qDebug() << "\t\t" << year << month << day << hh << mm << ss << " / " << ts << QFileInfo(filename).fileName();
//        qDebug() << "\t\tadding node" << node << "(" << x << "," << y << "," << d.toTime_t() << ")";
        addPoint(node, d.toTime_t(), x, y);
    }
}

void TraceLayer::exportLayer(QString output)
{
    emit loadProgressChanged((qreal) 0.0);
    const char *pszDriverName = "ESRI Shapefile";
    OGRSFDriver *poDriver;

    OGRRegisterAll();
    OGRDataSource *poDS;

    poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(
                    pszDriverName );
    if( poDriver == NULL )
    {
        printf( "%s driver not available.\n", pszDriverName );
        emit loadProgressChanged((qreal) 1.0);
        exit( 1 );
    }

    poDS = poDriver->CreateDataSource( output.toLatin1().data(), NULL );
    if( poDS == NULL )
    {
        qWarning() << "Creation of file " + output + "failed.";
    }

    OGRSpatialReference *srs = new OGRSpatialReference();
    srs->importFromProj4(_parent->getProjOut().toLatin1().data());

    OGRLayer *poLayerTrace;
    poLayerTrace = poDS->CreateLayer( "trace", srs, wkbPoint, NULL );
    if(poLayerTrace == NULL) {
        qWarning() << "Trace Layer creation failed.";
    }

    OGRFieldDefn oFieldWeight( "Weight", OFTReal );
    oFieldWeight.SetWidth(32);
    if( poLayerTrace->CreateField( &oFieldWeight ) != OGRERR_NONE )
    {
        qWarning() <<  "Creating Weight field failed.";
    }

    OGRFieldDefn oFieldDistance( "Distance", OFTReal );
    oFieldDistance.SetWidth(32);
    if( poLayerTrace->CreateField( &oFieldDistance ) != OGRERR_NONE )
    {
        qWarning() <<  "Creating Distance field failed.";
    }

    OGRFieldDefn oFieldTimestamp( "Timestamp", OFTInteger );
    oFieldTimestamp.SetWidth(32);
    if( poLayerTrace->CreateField( &oFieldTimestamp ) != OGRERR_NONE )
    {
        qWarning() <<  "Creating Timestamp field failed.";
    }

    OGRFieldDefn oFieldNode( "Node", OFTString );
    oFieldNode.SetWidth(32);
    if( poLayerTrace->CreateField( &oFieldNode ) != OGRERR_NONE )
    {
        qWarning() <<  "Creating Node field failed.";
    }

    int count = 0, size = _nodes.size();
    for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
        const char* node = it.key().toLatin1().data();
        for(auto jt = it.value()->begin(); jt != it.value()->end(); jt++) {
            int timestamp = (int) jt.key();
            QPointF pt = jt.value();

            OGRFeature *poFeature = OGRFeature::CreateFeature( poLayerTrace->GetLayerDefn() );
            poFeature->SetField("Weight", 1.0);
            poFeature->SetField("Distance", 0.0);
            poFeature->SetField("Timestamp", timestamp);
            poFeature->SetField("Node", node);

            OGRPoint point(pt.x(), pt.y());
            poFeature->SetGeometry( &point );

            if( poLayerTrace->CreateFeature( poFeature ) != OGRERR_NONE )
            {
               qWarning() << "Failed to create feature in shapefile.";
            }
            OGRFeature::DestroyFeature( poFeature );
        }
        emit loadProgressChanged((qreal) ++count / (qreal) size);
    }
    emit loadProgressChanged((qreal) 1.0);
    qDebug() << "[Writing] DONE";
    OGRDataSource::DestroyDataSource( poDS );
}

void TraceLayer::exportLayerONE(QString output)
{
    emit loadProgressChanged((qreal) 0.0);
    // <timestamp, <node id, positions, state(UP|DOWN)>
    QMap<long long, QMap<int, QPair<QPointF,bool>>> points;
    QMap<int,QPair<QPointF,long long>> initPos;

    int nodeCounter = 0;
    double minTime = 1e10;
    double maxTime = 0;
    double minX = 1e10;
    double maxX = 0;
    double minY = 1e10;
    double maxY = 0;

    qDebug() << "[START] Writing in file" << output;
    int size = _nodes.size();
    for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
        auto jt = it.value()->begin();
        long long currentTimestamp = jt.key();
        QPointF currentPoint = jt.value();
        initPos.insert(nodeCounter, qMakePair(currentPoint,currentTimestamp));

        if(currentTimestamp < minTime) minTime = currentTimestamp;
        if(currentTimestamp > maxTime) maxTime = currentTimestamp;
        if(currentPoint.x() < minX) minX = currentPoint.x();
        if(currentPoint.x() > maxX) maxX = currentPoint.x();
        if(currentPoint.y() < minY) minY = currentPoint.y();
        if(currentPoint.y() > maxY) maxY = currentPoint.y();

        jt++;
        for(; jt != it.value()->end(); ++jt) {
            long long nextTimestamp = jt.key();
            QPointF nextPoint = jt.value();

            // determine state
            bool state = true; // UP
            if((nextTimestamp - currentTimestamp) > 300) {
                state = false; // DOWN
            }
            // add current point
            if(!points.contains(currentTimestamp)) {
                points.insert(currentTimestamp, QMap<int, QPair<QPointF,bool>>());
            }
            points[currentTimestamp][nodeCounter] = qMakePair(currentPoint, state);

            // update the boundaries
            if(nextTimestamp < minTime) minTime = nextTimestamp;
            if(nextTimestamp > maxTime) maxTime = nextTimestamp;
            if(nextPoint.x() < minX) minX = nextPoint.x();
            if(nextPoint.x() > maxX) maxX = nextPoint.x();
            if(nextPoint.y() < minY) minY = nextPoint.y();
            if(nextPoint.y() > maxY) maxY = nextPoint.y();

            currentTimestamp = nextTimestamp;
            currentPoint = nextPoint;
        }
        //insert last timestamp
        if(!points.contains(currentTimestamp)) {
            points.insert(currentTimestamp, QMap<int, QPair<QPointF,bool>>());
        }
        points[currentTimestamp][nodeCounter] = qMakePair(currentPoint, false);

        nodeCounter++;
        emit loadProgressChanged(0.5 * ((qreal) nodeCounter / (qreal) size));
    }
    emit loadProgressChanged((qreal) 0.5);

    QFile file(output);
    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "Unable to write in file "<< output;
        emit loadProgressChanged((qreal) 1.0);
        return;
    }

    QTextStream out(&file);
    // write first line
    // minTime maxTime minX maxX minY maxY [minZ maxZ]
    QString::number(minTime, 'f', 2);
    out << QString::number(minTime, 'f', 0) << " "
        << QString::number(maxTime, 'f', 0) << " "
        << QString::number(minX - 500, 'f', 0) << " "
        << QString::number(maxX + 500, 'f', 0) << " "
        << QString::number(minY - 500, 'f', 0) << " "
        << QString::number(maxY + 500, 'f', 0) << "\n";

    // display all nodes state
    auto it = points.begin();
    long long firstTimestamp = it.key();
    for(auto jt = initPos.begin(); jt != initPos.end(); ++jt) {
        int nodeID = jt.key();
        QPointF pos = jt.value().first;
        long long timestamp = jt.value().second;
        out << QString::number(firstTimestamp, 'f', 0) << " "
            << QString::number(nodeID, 'f', 0) << " "
            << QString::number(pos.x(), 'f', 0) << " "
            << QString::number(pos.y(), 'f', 0) << " "
            << ((timestamp == firstTimestamp) ? "UP" : "DOWN") << "\n";
    }
    it++;

    // iterate through the timestamps
    int count = 0;
    size = points.size();
    for(; it != points.end(); ++it) {
        long long timestamp = it.key();
        for(auto jt = it.value().begin(); jt != it.value().end(); ++jt) {
            // int,QPointF,bool
            int nodeID = jt.key();
            QPointF pos = jt.value().first;
            bool state = jt.value().second;
            // write line
            // time id xPos yPos
            out << QString::number(timestamp, 'f', 0) << " "
                << QString::number(nodeID, 'f', 0) << " "
                << QString::number(pos.x(), 'f', 4) << " "
                << QString::number(pos.y(), 'f', 4) << " "
                << (state ? "UP" : "DOWN") << "\n";
        }
        emit loadProgressChanged(0.5 + ((qreal) ++count / (qreal) size));
    }
    emit loadProgressChanged((qreal) 1.0);
    file.close();
    qDebug() << "[DONE] finished writing in file" << output;
}

void TraceLayer::exportLayerGrid(QString output, int cellSize, long long duration)
{
    emit loadProgressChanged((qreal) 0.0);
    // populate the cells
    QHash<QPoint, int> cells;
    int count = 0, size = cells.size();
    for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
        auto trace = it.value();
        long long startTime = trace->firstKey();
        for(auto jt = trace->begin(); jt != trace->end() && jt.key() - startTime <= duration; ++jt) {
            QPointF p(jt.value());
            QPoint cell(qFloor(p.x() / cellSize), qFloor(p.y() / cellSize));
            if(!cells.contains(cell)) {
                cells.insert(cell, 0);
            }
            cells[cell]++;
        }
        emit loadProgressChanged(0.5 * ((qreal) ++count / (qreal) size));
    }
    emit loadProgressChanged((qreal) 0.5);

    // export the points with spatial bining
    qDebug() << "[Writing] output grid cells";

    const char *pszDriverName = "ESRI Shapefile";
    OGRSFDriver *poDriver;

    OGRRegisterAll();
    OGRDataSource *poDS;

    poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(
                    pszDriverName );
    if( poDriver == NULL )
    {
        printf( "%s driver not available.\n", pszDriverName );
        emit loadProgressChanged((qreal) 1.0);
        exit( 1 );
    }

    poDS = poDriver->CreateDataSource( output.toLatin1().data(), NULL );
    if( poDS == NULL )
    {
        qWarning() << "Creation of output file failed.";
    }

    OGRSpatialReference *srs = new OGRSpatialReference();
    srs->importFromProj4(ProjFactory::getInstance().getOutputProj());

    OGRLayer *poLayerGrid;
    poLayerGrid = poDS->CreateLayer( "grid", srs, wkbPolygon, NULL );
    if( poLayerGrid == NULL )
    {
        qWarning() << "Grid Layer creation failed.";
    }

    OGRFieldDefn oFieldCount( "Count", OFTInteger );
    oFieldCount.SetWidth(32);
    if( poLayerGrid->CreateField( &oFieldCount ) != OGRERR_NONE )
    {
        qWarning() <<  "Creating Count field failed.";
    }

    int cellCount = 0;
    cellSize = cells.size();
    for(auto it = cells.begin(); it != cells.end(); ++it) {
        QPoint p = it.key();
        int count = it.value();
        QRect cell(p.x()*cellSize, p.y()*cellSize, cellSize, cellSize);

        OGRFeature *poFeature = OGRFeature::CreateFeature( poLayerGrid->GetLayerDefn() );

        poFeature->SetField( "Count", count );
        OGRLinearRing  oRing;
        OGRPolygon oPoly;

        oRing.addPoint(cell.bottomLeft().x(),  cell.bottomLeft().y());
        oRing.addPoint(cell.bottomRight().x(), cell.bottomRight().y());
        oRing.addPoint(cell.topRight().x(),    cell.topRight().y());
        oRing.addPoint(cell.topLeft().x(),     cell.topLeft().y());
        oRing.addPoint(cell.bottomLeft().x(),  cell.bottomLeft().y());

        oPoly.addRing( &oRing );

        poFeature->SetGeometry( &oPoly );

        if( poLayerGrid->CreateFeature( poFeature ) != OGRERR_NONE )
        {
           qWarning() << "Failed to create feature in shapefile.";
        }
        OGRFeature::DestroyFeature( poFeature );
        emit loadProgressChanged((qreal) ++cellCount / (qreal) cellSize);
    }

    OGRDataSource::DestroyDataSource( poDS );
    emit loadProgressChanged((qreal) 1.0);

    qDebug() << "[Writing] DONE writing" << cells.size() << "cells.";
}


void TraceLayer::exportLayerText(QString output, long long duration)
{
    double minTime = 1e10;
    double maxTime = 0;
    double minX = 1e10;
    double maxX = 0;
    double minY = 1e10;
    double maxY = 0;

    emit loadProgressChanged((qreal) 0.0);
    QFile file(output);
    if(!file.open(QFile::WriteOnly))
    {
        emit loadProgressChanged((qreal) 1.0);
        qDebug() << "Unable to write in file "<< output;
        return;
    }
    QTextStream out(&file);
    int count = 0, size = _nodes.size();
    // loop for all the nodes to get the points within t0 (the begining of the trace) and t0 + duration
    for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
        auto trace = it.value();
        long long startTime = trace->firstKey();
        for(auto jt = trace->begin(); jt != trace->end() && jt.key() - startTime <= duration; ++jt) {
            QPointF p(jt.value());
            out << QString::number(p.x(), 'f', 0) << "," << QString::number(p.y()) << "\n";
            if(p.x() < minX) minX = p.x();
            if(p.x() > maxX) maxX = p.x();
            if(p.y() < minY) minY = p.y();
            if(p.y() > maxY) maxY = p.y();
        }
        emit loadProgressChanged((qreal) ++count / (qreal) size);
    }
    file.close();
    emit loadProgressChanged((qreal) 1.0);

    qDebug() << "[DONE] finished writing in file" << output;
    qDebug() << "C("<<minX<<","<<maxX<<"), c("<<minY<<","<<maxY<<")";
}

void TraceLayer::addBarMenuItems() {
    _menu = new QMenu("Trace");
    QAction* actionShowIntermediatePoints = _menu->addAction("Show intermediate points");
    QAction* actionExportTraceTxt = _menu->addAction("Export trace (text)");
    QAction* actionExportTraceShp = _menu->addAction("Export trace (shapefile)");
    QAction* actionExportONETrace = _menu->addAction("Export trace (ONE)");
    QAction* actionExportTraceGrid = _menu->addAction("Export trace grid");
    _menu->addSeparator();
    QAction* actionSpatialStats = _menu->addAction("Spatial statistics");

    _parent->addMenu(_menu);

    connect(actionShowIntermediatePoints, &QAction::triggered, [=](bool checked){
        qDebug() << "Show intermediate points of" << getName();
        QString name = "Intermediate positions";
        IntermediatePosLayer* layer   = new IntermediatePosLayer(_parent, name, this);
        Loader loader(layer);
        _parent->createLayer(name, layer, &loader);
    });

    connect(actionExportTraceTxt, &QAction::triggered, [=](bool checked){
        QString filename = QFileDialog::getSaveFileName(0,
                                                        tr("Save the Trace layer"),
                                                        QString(),
                                                        tr("CSV file (*.csv)"));

        if(filename.isEmpty())
            return;

        qDebug() << "Exporting text of" << filename;
        ProgressDialog progressDialog(_parent, "Writing text "+QFileInfo(filename).fileName());
        connect(this, &TraceLayer::loadProgressChanged, &progressDialog, &ProgressDialog::updateProgress);
        QtConcurrent::run([&](QString filename){
            exportLayerText(filename);
        }, filename);
        progressDialog.exec();
    });

    connect(actionExportTraceShp, &QAction::triggered, [=](bool checked){
        QString filename = QFileDialog::getSaveFileName(0,
                                                        tr("Save the Trace layer"),
                                                        QString(),
                                                        tr("Shapefile file (*.shp)"));

        if(filename.isEmpty())
            return;

        qDebug() << "Exporting trace shapefile" << filename;
        ProgressDialog progressDialog(_parent, "Writing Shp "+QFileInfo(filename).fileName());
        connect(this, &TraceLayer::loadProgressChanged, &progressDialog, &ProgressDialog::updateProgress);
        QtConcurrent::run([&](QString filename){
            exportLayer(filename);
        }, filename);
        progressDialog.exec();
    });

    connect(actionExportONETrace, &QAction::triggered, [=](bool checked){
        QString filename = QFileDialog::getSaveFileName(0,
                                                        tr("Save the Trace layer"),
                                                        QString(),
                                                        tr("Text file (*.txt)"));

        if(filename.isEmpty())
            return;

        qDebug() << "Exporting ONE trace" << filename;
        ProgressDialog progressDialog(_parent, "Writing ONE "+QFileInfo(filename).fileName());
        connect(this, &TraceLayer::loadProgressChanged, &progressDialog, &ProgressDialog::updateProgress);
        QtConcurrent::run([&](QString filename){
            exportLayerONE(filename);
        }, filename);
        progressDialog.exec();
    });

    connect(actionExportTraceGrid, &QAction::triggered, [=](bool checked){
        QString filename = QFileDialog::getSaveFileName(0,
                                                        tr("Save the Trace layer"),
                                                        QString(),
                                                        tr("Shapefile file (*.shp)"));

        if(filename.isEmpty())
            return;

        qDebug() << "Exporting shapefile grid of" << filename;
        ProgressDialog progressDialog(_parent, "Writing grid "+QFileInfo(filename).fileName());
        connect(this, &TraceLayer::loadProgressChanged, &progressDialog, &ProgressDialog::updateProgress);
        QtConcurrent::run([&](QString filename){
            exportLayerGrid(filename);
        }, filename);
        progressDialog.exec();
    });

    connect(actionSpatialStats, &QAction::triggered, [=](bool checked){
        qDebug() << "Compute spatial Stats";
        if(!_spatialStats) {
            SpatialStatsDialog spatialStatsDialog(_parent, this);
            int ret = spatialStatsDialog.exec(); // synchronous
            if (ret == QDialog::Rejected) {
                return;
            }
            double sampling  = spatialStatsDialog.getSampling();
            double startTime = spatialStatsDialog.getStartTime();
            double endTime   = spatialStatsDialog.getEndTime();
            GeometryIndex* geometryIndex = spatialStatsDialog.getGeometryIndex();

            _spatialStats = new SpatialStats(_parent, "Spatial Stats layer", this,
                                             (int) sampling, (long long) startTime, (long long) endTime,
                                             geometryIndex);
        }

        QString layerName = "Spatial Stats";
        Loader* loader = new Loader(_spatialStats);
        _parent->createLayer(layerName, _spatialStats, loader);
    });
}
