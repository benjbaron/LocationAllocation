#include "trace_layer.h"

#include "proj_factory.h"
#include "intermediate_pos_layer.h"
#include "spatial_stats_dialog.h"
#include "spatial_stats.h"
#include "spatial_stats_layer.h"

QGraphicsItemGroup* TraceLayer::draw() {
    int radius = 10;
    _groupItem = new QGraphicsItemGroup();

    QHash<QString, QMap<long long, QPointF>*> nodes;
    _trace->getNodes(&nodes);

    for(auto it = nodes.begin(); it != nodes.end(); ++it) {
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

bool TraceLayer::load(Loader* loader) {
    _trace->openTrace(loader);
    showMenu();
    return true;
}

bool TraceLayer::exportLayer(Loader* loader, QString output) {
    loader->loadProgressChanged((qreal) 0.0, "");
    const char *pszDriverName = "ESRI Shapefile";
    OGRSFDriver *poDriver;

    OGRRegisterAll();
    OGRDataSource *poDS;

    poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(
                    pszDriverName );
    if( poDriver == NULL ) {
        printf( "%s driver not available.\n", pszDriverName );
        loader->loadProgressChanged((qreal) 1.0, "");
        exit( 1 );
    }

    poDS = poDriver->CreateDataSource( output.toLatin1().data(), NULL );
    if( poDS == NULL ) {
        qWarning() << "Creation of file " + output + "failed.";
    }

    OGRSpatialReference *srs = new OGRSpatialReference();
    srs->importFromProj4(_parent->getProjOut().toLatin1().data());

    OGRLayer *poLayerTrace;
    poLayerTrace = poDS->CreateLayer( "trace", srs, wkbPoint, NULL );
    if(poLayerTrace == NULL) {
        qWarning() << "trace Layer creation failed.";
    }

    OGRFieldDefn oFieldWeight( "Weight", OFTReal );
    oFieldWeight.SetWidth(32);
    if( poLayerTrace->CreateField( &oFieldWeight ) != OGRERR_NONE ) {
        qWarning() <<  "Creating Weight field failed.";
    }

    OGRFieldDefn oFieldDistance( "Distance", OFTReal );
    oFieldDistance.SetWidth(32);
    if( poLayerTrace->CreateField( &oFieldDistance ) != OGRERR_NONE ) {
        qWarning() <<  "Creating Distance field failed.";
    }

    OGRFieldDefn oFieldTimestamp( "Timestamp", OFTInteger );
    oFieldTimestamp.SetWidth(32);
    if( poLayerTrace->CreateField( &oFieldTimestamp ) != OGRERR_NONE ) {
        qWarning() <<  "Creating Timestamp field failed.";
    }

    OGRFieldDefn oFieldNode( "Node", OFTString );
    oFieldNode.SetWidth(32);
    if( poLayerTrace->CreateField( &oFieldNode ) != OGRERR_NONE ) {
        qWarning() <<  "Creating Node field failed.";
    }

    int count = 0;
    int size = _trace->getNbNodes();
    QHash<QString, QMap<long long, QPointF>*> nodes;
    _trace->getNodes(&nodes);

    for(auto it = nodes.begin(); it != nodes.end(); ++it) {
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

            if( poLayerTrace->CreateFeature( poFeature ) != OGRERR_NONE ) {
               qWarning() << "Failed to create feature in shapefile.";
            }
            OGRFeature::DestroyFeature( poFeature );
        }
        loader->loadProgressChanged((qreal) ++count / (qreal) size, "");
    }
    loader->loadProgressChanged((qreal) 1.0, "Done");
    qDebug() << "[Writing] DONE";
    OGRDataSource::DestroyDataSource( poDS );

    return true;
}

bool TraceLayer::exportLayerONE(Loader* loader, QString output) {
    loader->loadProgressChanged((qreal) 0.0, "");
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
    int size = _trace->getNbNodes();
    QHash<QString, QMap<long long, QPointF>*> nodes;
    _trace->getNodes(&nodes);

    for(auto it = nodes.begin(); it != nodes.end(); ++it) {
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
        loader->loadProgressChanged(0.5 * ((qreal) nodeCounter / (qreal) size), "");
    }
    loader->loadProgressChanged((qreal) 0.5, "");

    QFile file(output);
    if(!file.open(QFile::WriteOnly)) {
        qDebug() << "Unable to write in file "<< output;
        loader->loadProgressChanged((qreal) 1.0, "");
        return false;
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
        loader->loadProgressChanged(0.5 + ((qreal) ++count / (qreal) size), "");
    }
    loader->loadProgressChanged((qreal) 1.0, "Done");
    file.close();
    qDebug() << "[DONE] finished writing in file" << output;

    return true;
}

bool TraceLayer::exportLayerGrid(Loader* loader, QString output, int cellSize, long long duration) {
    loader->loadProgressChanged((qreal) 0.0, "");
    // populate the cells
    QHash<QPoint, int> cells;
    int count = 0;
    int size = cells.size();

    QHash<QString, QMap<long long, QPointF>*> nodes;
    _trace->getNodes(&nodes);

    for(auto it = nodes.begin(); it != nodes.end(); ++it) {
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
        loader->loadProgressChanged(0.5 * ((qreal) ++count / (qreal) size), "");
    }
    loader->loadProgressChanged((qreal) 0.5, "");

    // export the points with spatial bining
    qDebug() << "[Writing] output grid cells";

    const char *pszDriverName = "ESRI Shapefile";
    OGRSFDriver *poDriver;

    OGRRegisterAll();
    OGRDataSource *poDS;

    poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(
                    pszDriverName );
    if( poDriver == NULL ) {
        qWarning() << pszDriverName << "driver not available.";
        loader->loadProgressChanged((qreal) 1.0, "Failed");
        exit( 1 );
    }

    poDS = poDriver->CreateDataSource( output.toLatin1().data(), NULL );
    if( poDS == NULL ) {
        qWarning() << "Creation of output file failed.";
    }

    OGRSpatialReference *srs = new OGRSpatialReference();
    srs->importFromProj4(ProjFactory::getInstance().getOutputProj());

    OGRLayer *poLayerGrid;
    poLayerGrid = poDS->CreateLayer( "grid", srs, wkbPolygon, NULL );
    if( poLayerGrid == NULL ) {
        qWarning() << "Grid Layer creation failed.";
    }

    OGRFieldDefn oFieldCount( "Count", OFTInteger );
    oFieldCount.SetWidth(32);
    if( poLayerGrid->CreateField( &oFieldCount ) != OGRERR_NONE ) {
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

        if( poLayerGrid->CreateFeature( poFeature ) != OGRERR_NONE ) {
           qWarning() << "Failed to create feature in shapefile.";
        }
        OGRFeature::DestroyFeature( poFeature );
        loader->loadProgressChanged((qreal) ++cellCount / (qreal) cellSize, "");
    }

    OGRDataSource::DestroyDataSource( poDS );
    loader->loadProgressChanged((qreal) 1.0, "Done");

    qDebug() << "[Writing] DONE writing" << cells.size() << "cells.";

    return true;
}


bool TraceLayer::exportLayerText(Loader* loader, QString output, long long duration) {
    double minTime = 1e10;
    double maxTime = 0;
    double minX = 1e10;
    double maxX = 0;
    double minY = 1e10;
    double maxY = 0;

    loader->loadProgressChanged((qreal) 0.0, "");
    QFile file(output);
    if(!file.open(QFile::WriteOnly)) {
        loader->loadProgressChanged((qreal) 1.0, "");
        qWarning() << "Unable to write in file" << output;
        return false;
    }

    QTextStream out(&file);
    int count = 0;
    int size = _trace->getNbNodes();
    QHash<QString, QMap<long long, QPointF>*> nodes;
    _trace->getNodes(&nodes);

    // loop for all the nodes to get the points within t0 (the beginning of the trace) and t0 + duration
    for(auto it = nodes.begin(); it != nodes.end(); ++it) {
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
        loader->loadProgressChanged((qreal) ++count / (qreal) size, "");
    }
    file.close();
    loader->loadProgressChanged((qreal) 1.0, "Done");

    qDebug() << "[DONE] finished writing in file" << output;
    qDebug() << "C("<<minX<<","<<maxX<<"), c("<<minY<<","<<maxY<<")";

    return true;
}

void TraceLayer::addMenuBar() {
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
        IntermediatePosLayer* layer = new IntermediatePosLayer(_parent, name, _trace);
        Loader loader;
        _parent->createLayer(name, layer, &loader);
    });

    connect(actionExportTraceTxt, &QAction::triggered, [=](bool checked){
        QString filename = QFileDialog::getSaveFileName(0,
                                                        tr("Save the trace layer"),
                                                        QString(),
                                                        tr("CSV file (*.csv)"));

        if(filename.isEmpty())
            return;

        qDebug() << "Exporting text of" << filename;
        long long duration = 86400;
        loadWithProgressDialog(this, &TraceLayer::exportLayerText, filename, duration);
    });

    connect(actionExportTraceShp, &QAction::triggered, [=](bool checked){
        QString filename = QFileDialog::getSaveFileName(0,
                                                        tr("Save the trace layer"),
                                                        QString(),
                                                        tr("Shapefile file (*.shp)"));

        if(filename.isEmpty())
            return;

        qDebug() << "Exporting trace shapefile" << filename;
        loadWithProgressDialog(this, &TraceLayer::exportLayer, filename);
//        ProgressDialog progressDialog(_parent, "Writing Shp "+QFileInfo(filename).fileName());
//        connect(this, &TraceLayer::loadProgressChanged, &progressDialog, &ProgressDialog::updateProgress);
//        QtConcurrent::run([&](QString filename){
//            exportLayer(filename);
//        }, filename);
//        progressDialog.exec();
    });

    connect(actionExportONETrace, &QAction::triggered, [=](bool checked){
        QString filename = QFileDialog::getSaveFileName(0,
                                                        tr("Save the trace layer"),
                                                        QString(),
                                                        tr("Text file (*.txt)"));

        if(filename.isEmpty())
            return;

        qDebug() << "Exporting ONE trace" << filename;
        loadWithProgressDialog(this, &TraceLayer::exportLayerONE, filename);
//        ProgressDialog progressDialog(_parent, "Writing ONE "+QFileInfo(filename).fileName());
//        connect(this, &TraceLayer::loadProgressChanged, &progressDialog, &ProgressDialog::updateProgress);
//        QtConcurrent::run([&](QString filename){
//            exportLayerONE(filename);
//        }, filename);
//        progressDialog.exec();
    });

    connect(actionExportTraceGrid, &QAction::triggered, [=](bool checked){
        QString filename = QFileDialog::getSaveFileName(0,
                                                        tr("Save the trace layer"),
                                                        QString(),
                                                        tr("Shapefile file (*.shp)"));

        if(filename.isEmpty())
            return;

        qDebug() << "Exporting shapefile grid of" << filename;
        int cellSize = 250;
        long long duration = 86400;
        loadWithProgressDialog(this, &TraceLayer::exportLayerGrid, filename, cellSize, duration);
    });

    connect(actionSpatialStats, &QAction::triggered, [=](bool checked){
        qDebug() << "Compute spatial Stats";
        if(!_spatialStatsLayer) {
            qDebug() << "trace end time" << _trace->getEndTime() << "start time" << _trace->getStartTime();
            qDebug() << "trace name: " << _trace->getName();
            SpatialStatsDialog spatialStatsDialog(_parent, _trace);
            int ret = spatialStatsDialog.exec(); // synchronous
            if (ret == QDialog::Rejected) {
                return;
            }
            double sampling  = spatialStatsDialog.getSampling();
            double startTime = spatialStatsDialog.getStartTime();
            double endTime   = spatialStatsDialog.getEndTime();
            GeometryIndex* geometryIndex = GeometryIndex::make_geometryIndex(_trace, sampling, startTime, endTime,
                                                                             spatialStatsDialog.getCellSize(),
                                                                             spatialStatsDialog.getGeometryType(),
                                                                             spatialStatsDialog.getCircleFile());

            SpatialStats* spatialStats = new SpatialStats(_trace,
                                                          (long long) sampling,
                                                          (long long) startTime,
                                                          (long long) endTime,
                                                          geometryIndex);

            _spatialStatsLayer = new SpatialStatsLayer(_parent, "Spatial Stats layer", spatialStats);
        }

        QString layerName = "Spatial Stats";
        Loader loader;
        _parent->createLayer(layerName, _spatialStatsLayer, &loader);
    });
}
