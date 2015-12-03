#include "tracelayer.h"

#include <QDebug>
#include <QFile>
#include <qmath.h>

#include "osrmwrapper.h"

QGraphicsItemGroup *TraceLayer::draw() {
    int radius = 10;
    _groupItem = new QGraphicsItemGroup();

    for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
        for(auto jt = it.value()->begin(); jt != it.value()->end(); ++jt) {
            int x = jt.value().x();
            int y = jt.value().y();

//            qDebug() << "node" << it.key() << "(" << jt.key() << "," << x << "," << y << ")";
            QGraphicsEllipseItem* item = new QGraphicsEllipseItem(x-radius, y-radius, radius*2, radius*2);
//            item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

            item->setBrush(QBrush(QColor(0,0,0)));
            item->setPen(Qt::NoPen);

            addGraphicsItem(item);
        }
    }

    return _groupItem;
}

QList<std::tuple<QPointF, double, double> > TraceLayer::getPoints(int deadline, long long startTime, long long endTime)
{
    // check endTime value
    if(endTime == 0) {
        // set to max
        endTime = std::numeric_limits<qlonglong>::max();
    }

    QList<std::tuple<QPointF,double,double>> res;
    for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
        // compute the average speed for each node
        double spd = 0.0;
        auto jt_start = it.value()->lowerBound(startTime);
        long long prevTS = jt_start.key();
        QPointF prevPoint = jt_start.value();
        for(auto jt = jt_start; jt != it.value()->end() && jt.key() <= endTime; ++jt) {
            long long curTS = jt.key();
            QPointF curPoint = jt.value();
            int timeDiff = curTS - prevTS;
            if(timeDiff > 0) {
                spd += QLineF(prevPoint, curPoint).length() / timeDiff;
            }
            prevTS = curTS;
            prevPoint = curPoint;
        }
        spd = spd / it.value()->size();
        qDebug() << "node" << it.key() << "speed" << spd << deadline * spd;
        // add the demand points
        for(auto jt = jt_start; jt != it.value()->end() && jt.key() <= endTime; ++jt) {
            auto t = std::make_tuple(jt.value(), 1.0, deadline * spd);
            res.append(t);
        }
    }

    return res;
}

OGRGeometryCollection *TraceLayer::getGeometry(long long startTime, long long endTime)
{
    // check endTime value
    if(endTime == 0) {
        // set to max
        endTime = std::numeric_limits<qlonglong>::max();
    }

    OGRGeometryCollection* collection = new OGRGeometryCollection();
    for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
        auto jt_start = it.value()->lowerBound(startTime);
        for(auto jt = jt_start; jt != it.value()->end() && jt.key() <= endTime; ++jt) {
            OGRPoint pt(jt.value().x(), jt.value().y());
            collection->addGeometry(&pt);
        }
    }
    return collection;
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

void TraceLayer::exportLayer(QString output)
{
    const char *pszDriverName = "ESRI Shapefile";
    OGRSFDriver *poDriver;

    OGRRegisterAll();
    OGRDataSource *poDS;

    poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(
                    pszDriverName );
    if( poDriver == NULL )
    {
        printf( "%s driver not available.\n", pszDriverName );
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

//    QList<std::tuple<QPointF, double, double> > points = getPoints();
//    QMap<QString, QMap<long long, QPointF>*> _nodes;
    for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
        const char* node = it.key().toLatin1().data();
        for(auto jt = it.value()->begin(); jt != it.value()->end(); jt++) {
            int timestamp = jt.key();
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
    }
    qDebug() << "[Writing] DONE";
    OGRDataSource::DestroyDataSource( poDS );
}

void TraceLayer::exportLayerONE(QString output)
{
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

    qDebug() << "[START] Writting in file" << output;

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

        qDebug() << nodeCounter << currentTimestamp << currentPoint;

        jt++;
        for(; jt != it.value()->end(); ++jt) {
            long long nextTimestamp = jt.key();
            QPointF nextPoint = jt.value();

            if(nextTimestamp <= 1000) {
                qDebug() << "we have" << nodeCounter << nextTimestamp << currentTimestamp;
            }

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
    }

    QFile file(output);
    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "Unable to write in file "<< output;
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
        qDebug() << nodeID << firstTimestamp << timestamp;
    }
    it++;

    // iterate throught the timestamps
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
                << QString::number(pos.x(), 'f', 0) << " "
                << QString::number(pos.y(), 'f', 0) << " "
                << (state ? "UP" : "DOWN") << "\n";
        }
    }
    file.close();

    qDebug() << "[DONE] finished writting in file" << output;
}

void TraceLayer::exportLayerGrid(QString output, int cellSize, long long duration)
{
    // populate the cells
    QHash<QPoint, int> cells;
    for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
        auto trace = it.value();
        long long startTime = trace->firstKey();
        for(auto jt = trace->begin(); jt != trace->end() && jt.key() - startTime <= duration; ++jt) {
            QPointF p(jt.value());
            QPoint cell((int)qFloor(p.x() / cellSize), (int)qFloor(p.y() / cellSize));
            if(!cells.contains(cell)) {
                cells.insert(cell, 0);
            }
            cells[cell]++;
        }
    }
    
    // export the points with spatial bining
    qDebug() << "[Writting] output grid cells";

    const char *pszDriverName = "ESRI Shapefile";
    OGRSFDriver *poDriver;

    OGRRegisterAll();
    OGRDataSource *poDS;

    poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(
                    pszDriverName );
    if( poDriver == NULL )
    {
        printf( "%s driver not available.\n", pszDriverName );
        exit( 1 );
    }

    poDS = poDriver->CreateDataSource( output.toLatin1().data(), NULL );
    if( poDS == NULL )
    {
        qWarning() << "Creation of output file failed.";
    }

    OGRSpatialReference *srs = new OGRSpatialReference();
    srs->importFromProj4(OSRMWrapper::getInstance().getOutputProj());

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
    }

    OGRDataSource::DestroyDataSource( poDS );
    qDebug() << "[Writing] DONE writting" << cells.size() << "cells.";
}


void TraceLayer::exportLayerText(QString output, long long duration)
{
    double minTime = 1e10;
    double maxTime = 0;
    double minX = 1e10;
    double maxX = 0;
    double minY = 1e10;
    double maxY = 0;

    QFile file(output);
    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "Unable to write in file "<< output;
        return;
    }
    QTextStream out(&file);
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
    }
    file.close();
    qDebug() << "[DONE] finished writting in file" << output;
    qDebug() << "C("<<minX<<","<<maxX<<"), c("<<minY<<","<<maxY<<")";
}

inline uint qHash(const QPoint &key)
{
    return qHash(key.x()) ^ qHash(key.y());
}
