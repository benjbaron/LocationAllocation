//
// Created by Benjamin Baron on 08/02/16.
//

#include <QFileInfo>
#include <QDebug>

#include "loader.h"
#include "gtfs_layer.h"
#include "proj_factory.h"
#include "csv_parser.h"
#include "point_layer.h"
#include "linestring_layer.h"
#include "number_dialog.h"
#include "shapefile_layer.h"



QGraphicsItemGroup *GTFSLayer::draw() {
    // show the bus lines
    qDebug() << _trace->getNbNodes() << "nodes";
    return TraceLayer::draw();
}

void GTFSLayer::addMenuBar() {
    TraceLayer::addMenuBar();
    _menu->addSeparator();
    QAction* actionShowStops          = _menu->addAction("Show Stops");
    QAction* actionShowLines          = _menu->addAction("Show Lines");
    QAction* actionExportStops        = _menu->addAction("Export Stops");
    QAction* actionExportTrajectories = _menu->addAction("Export Trajectories");
    QAction* actionExportONEfile      = _menu->addAction("Export ONE setting file");

    /* Show GTFS bus stops */
    connect(actionShowStops, &QAction::triggered, [=](bool checked){
        qDebug() << "Show stops of " << getName();
        QString name = "GTFS Bus stops";
        QList<QPointF> stops;
        GTFSTrace* ptrace = static_cast<GTFSTrace*>(_trace);
        QMap<QString, Stop*> stopMap;
        ptrace->getAllStops(&stopMap);
        for(auto it = stopMap.begin(); it != stopMap.end(); ++it) {
            stops.append(it.value()->getCoords());
        }
        qDebug() << stops.size() << "stops";
        PointLayer* layer = new PointLayer(_parent, name, stops);
        Loader loader;
        _parent->createLayer(name, layer, &loader);
    });

    /* Show GTFS bus lines */
    connect(actionShowLines, &QAction::triggered, [=](bool checked){
        qDebug() << "Show lines of " << getName();
        QString name = "GTFS Bus lines";
        QList<LineStringDisplay*> lines;
        GTFSTrace* ptrace = static_cast<GTFSTrace*>(_trace);
        QMap<QString, QMap<int,QPointF>*> shapesMap;
        ptrace->getShapes(&shapesMap);

        /* Get the number of max trips */
        int maxTripCount = 0;
        QMap<QString, QSet<QString>*> shapeToTrips;
        ptrace->getShapesToTrips(&shapeToTrips);
        for(auto it = shapeToTrips.begin(); it != shapeToTrips.end(); it++) {
            int count = it.value()->size();
            if(count > maxTripCount) maxTripCount = count;
        }

        for(auto it = shapesMap.begin(); it != shapesMap.end(); ++it) {
            QList<QPointF> line;
            for(auto jt = it.value()->begin(); jt != it.value()->end(); ++jt) {
                line.append(jt.value());
            }
            // compute the width
            int count = shapeToTrips.value(it.key())->size();
            int width = (int) (1.0 + ((double) count / (double) maxTripCount) * 5.0);
            lines.append(new LineStringDisplay(line, width));
        }
        LineStringLayer* layer = new LineStringLayer(_parent, name, lines);
        Loader loader;
        _parent->createLayer(name, layer, &loader);
    });

    connect(actionExportONEfile, &QAction::triggered, this, &GTFSLayer::exportONESettings);

    connect(actionExportStops, &QAction::triggered, this, &GTFSLayer::exportStops);
    connect(actionExportTrajectories, &QAction::triggered, this, &GTFSLayer::exportTrajectoryShapefile);
}

void GTFSLayer::exportStops() {

    QString filename = QFileDialog::getSaveFileName(0,
                                                    tr("Save the stops"),
                                                    QString(),
                                                    tr("CSV file (*.csv)"));

    if(filename.isEmpty())
        return;

    // choose radius
    NumberDialog numDiag(_parent, "Set radius");
    numDiag.addField("Radius", 0);
    int ret = numDiag.exec(); // synchronous
    if (ret == QDialog::Rejected) {
        return;
    }
    int radius = (int)numDiag.getNumber(0);
    GTFSTrace* ptrace = static_cast<GTFSTrace*>(_trace);
    QMap<QString, Stop*> stopMap;
    ptrace->getAllStops(&stopMap);

    qDebug() << "Exporting" << stopMap.size() << "stops in" << filename;
    QFile file(filename);
    if(!file.open(QFile::WriteOnly)) {
        qDebug() << "Unable to write in file "<< filename;
        return;
    }

    QTextStream out(&file);
    for(const Stop* s : stopMap.values()) {
        out << QString::number(s->getCoords().x(), 'f', 4) << ";"
        << QString::number(s->getCoords().y(), 'f', 4) << ";"
        << QString::number(radius) << "\n";
    }
    file.close();

    qDebug() << "[DONE] export stops points";
}

void GTFSLayer::exportONESettings() {
    qDebug() << "Export ONE setting file of" << getName();

    // choose directory to record the WKT bus lines
    QSettings settings;
    QFileDialog d(_parent, "Open the output directory");
    d.setFileMode(QFileDialog::Directory);
    QString path = d.getExistingDirectory(_parent,
                                          "Open the output directory",
                                          settings.value("defaultGTFSOutputONESettingPath", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString());

    if(path.isEmpty()) {
        return;
    }

    // Save the filename path in the app settings
    settings.setValue("defaultGTFSOutputONESettingPath", QFileInfo(path).absolutePath());
    QString name = QFileInfo(path).fileName();
    qDebug() << "opened directory" << name;

    GTFSTrace* trace = static_cast<GTFSTrace*>(_trace);


    // record all the trajectories (shapes) into a world WKT (world.wkt)
    QString shapefilepath = QFileDialog::getOpenFileName(_parent,
                                                         "Open a shapefile",
                                                         settings.value("defaultShapefileFilePath",
                                                                        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                         "Shapefile (*.shp);;WKT files (*.wkt)");

    if(shapefilepath.isEmpty()) {
        return;
    }

    settings.setValue("defaultShapefileFilePath", QFileInfo(shapefilepath).absolutePath());
    QString shapefilename = QFileInfo(shapefilepath).fileName();
    qDebug() << "opened shapefile" << shapefilepath;

    Loader l;
    QFuture<bool> future;
    ProgressDialog p(_parent);
    QObject::connect(&l, &Loader::loadProgressChanged, &p, &ProgressDialog::updateProgress);

    // open shapefile and load it
    Shapefile* shapefile = new Shapefile(shapefilepath);
    future = l.load(shapefile, &Shapefile::open, &l);
    p.exec();

    future.result(); // wait for results

    qDebug() << "remove not connected components";
    future = l.load(shapefile, &Shapefile::removeNotConnectedComponents, &l);
    p.exec();

    future.result(); // wait for results

    // get all the stops to project
    QHash<QPointF,ProjectedPoint*> projPoints;
    QMap<QString, Stop*> stopMap;
    trace->getAllStops(&stopMap);

    for(auto it = stopMap.begin(); it != stopMap.end(); ++it) {
        Stop* stop = it.value();
        QPointF pt(stop->getCoords());
        ProjectedPoint* projPt = new ProjectedPoint(pt, QPointF());
        projPt->stop = stop;
        projPoints.insert(pt, projPt);
    }

    future = l.load(shapefile, &Shapefile::projectPoints, &l, &projPoints);
    p.exec();

    future.result(); // wait for results

    // export the shapefile that includes the projected points
    QString worldWKToutput = path+"/world.wkt";
    future = l.load(shapefile, &Shapefile::exportWKT, &l, worldWKToutput);
    p.exec();

    future.result(); // wait for results

    // update all the projection of the stops
    QSet<Stop*> notProjectedStops;
    for(auto it = projPoints.begin(); it != projPoints.end(); ++it) {
        ProjectedPoint* projPt = it.value();
        if(projPt->projectedPoint.isNull()) {
            notProjectedStops.insert(projPt->stop);
        } else {
            // modify the projection of the stop
            projPt->stop->setProjCoords(projPt->projectedPoint);
        }
    }

    // record each bus trajectory (successive bus stops) into WKT (line-<route_id>.wkt)
    // project the stops on the corresponding shapes
    QMap<QString, QSet<QString>*> shapesToTrips;
    trace->getShapesToTrips(&shapesToTrips);

    QString output = path+"/settings.txt";
    QFile file(output);
    if(!file.open(QFile::WriteOnly)) {
        qWarning() << "Unable to write in file" << output;
        return;
    }


    int count = 1;
    for(QString shapeId : shapesToTrips.keys()) {
        double averageNrofVehicles = trace->getAverageNumberOfVehicles(shapeId);
        if(averageNrofVehicles >= 1)
            count += 1;
    }

    // export the settings file
    QTextStream out(&file);
    out << QString("%1\n").arg(QString::number(count))
        << "MovementModel.worldSize = 37500, 59500\n"
        << "MapBasedMovement.nrofMapFiles = 1\n"
        << "MapBasedMovement.mapFile1 = data/FileSystem/sfmuni/world.wkt";

    for(QString shapeId : shapesToTrips.keys()) {
        qDebug() << "processing shapeId" << shapeId;

        // get the average number of vehicles for each group (route)
        double averageSpeed;
        double stddevSpeed;
        trace->getAverageSpeed(shapeId, &averageSpeed, &stddevSpeed);
        // get the average speed for each group (route)
        double averageNrofVehicles = trace->getAverageNumberOfVehicles(shapeId);

        if(averageNrofVehicles < 1)
            continue;

        // get the corresponding linestring and stops
        geos::geom::LineString* stops = nullptr;
        trace->getTrajectoryLineString(shapeId, stops, notProjectedStops);

        // output the linestring to a wkt file
        QString wktFile = QString(path+"/bus%1.wkt").arg(QString::number(count));
        QString wktFilename = QFileInfo(wktFile).fileName();
        toWKTFile(stops, wktFile);

        // print the setting attributes
        QString str = QString("Group%1.groupID = t\n").arg(QString::number(count));
        str += QString("Group%1.movementModel = MapRouteMovement\n").arg(QString::number(count));
        str += QString("Group%1.routeFile = data/FileSystem/sfmuni/%2\n").arg(QString::number(count), wktFilename);
        str += QString("Group%1.routeType = 2\n").arg(QString::number(count));
        str += QString("Group%1.waitTime = 10, 30\n").arg(QString::number(count));
        str += QString("Group%1.speed = %2, %3\n").arg(QString::number(count),
                                                   QString::number(averageSpeed),
                                                   QString::number(averageSpeed+stddevSpeed));
        str += QString("Group%1.nrofHosts = %2\n").arg(QString::number(count), QString::number(averageNrofVehicles));
        out << str + "\n";
    }
}

void GTFSLayer::toWKTFile(geos::geom::LineString *ls, const QString &output) {
    QFile file(output);
    if(!file.open(QFile::WriteOnly)) {
        qWarning() << "Unable to write in file" << output;
        return;
    }

    geos::io::WKTWriter writer;
    QTextStream out(&file);

    writer.setRoundingPrecision(3); // remove unnecessary zeros
    out << QString::fromStdString(writer.write(ls)) << "\n";

    file.close();
}

void GTFSLayer::exportTrajectoryShapefile() {
    GTFSTrace* trace = static_cast<GTFSTrace*>(_trace);

    QMap<QString, QSet<QString>*> shapesToTrips;
    trace->getShapesToTrips(&shapesToTrips);
    QMap<QString, QMap<int,QPointF>*> shapesMap;
    trace->getShapes(&shapesMap);

    // calculate the maximum number of trips to normalize the width of the shapes
    int maxTripCount = 0;
    for(auto it = shapesToTrips.begin(); it!=shapesToTrips.end(); ++it) {
        int count = it.value()->size();
        if(count > maxTripCount) maxTripCount = count;
    }

    // create the map of shape linestring
    QMap<QString, OGRLineString*> shapes;

    const char* pszDriverName = "ESRI Shapefile";
    OGRSFDriver* poDriver;

    OGRRegisterAll();

    poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName( pszDriverName );
    if( poDriver == NULL ) {
        printf( "%s driver not available.\n", pszDriverName );
        exit( 1 );
    }

    OGRDataSource* poDS;
    poDS = poDriver->CreateDataSource( "/Users/ben/Desktop/ls_out1.shp", NULL );
    if( poDS == NULL ) {
        qWarning() << "Creation of output file failed.";
    }

    OGRLayer *poLayer;

    poLayer = poDS->CreateLayer( "ls_out", NULL, wkbLineString, NULL );
    if( poLayer == NULL ) {
        qWarning() << "Layer creation failed.";
    }

    OGRFieldDefn oField1( "Name", OFTString );
    oField1.SetWidth(32);

    if( poLayer->CreateField( &oField1 ) != OGRERR_NONE ) {
        qWarning() <<  "Creating Name field failed.";
    }

    OGRFieldDefn oField2( "Trip", OFTInteger );
    oField1.SetWidth(32);

    if( poLayer->CreateField( &oField2 ) != OGRERR_NONE ) {
        qWarning() << "Creating Trips field failed.";
    }

    OGRFieldDefn oField3( "Normalized", OFTReal );
    oField1.SetWidth(32);

    if( poLayer->CreateField( &oField3 ) != OGRERR_NONE ) {
        qWarning() << "Creating Trips field failed.";
    }

    for(auto it = shapesMap.begin(); it != shapesMap.end(); ++it) {
        QString shapeId = it.key();
        if(! shapesToTrips.contains(shapeId)) continue;
        OGRLineString* ls = new OGRLineString();

        for(auto sIt = it.value()->begin(); sIt != it.value()->end(); ++sIt) {
            ls->addPoint(sIt.value().x(), sIt.value().y());
        }
        shapes.insert(it.key(), ls);

        // compute the width
        int count = shapesToTrips.value(shapeId)->size();
        double width = (int) (1.0 + ((double)count / (double)maxTripCount)*5.0);

        // Add the linestring to the list of geometries to draw
        OGRFeature* poFeature;
        poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
        poFeature->SetField( "Name", shapeId.toLatin1().data() );
        poFeature->SetField( "Trip", count );
        poFeature->SetField( "Normalized", width );
        poFeature->SetGeometry( ls );
        qDebug() << "we have:" << poFeature->GetFieldCount() << poFeature->GetFieldAsString("Name");

        if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE ) {
            qWarning() << "Failed to create feature in shapefile.";
        }
        OGRFeature::DestroyFeature( poFeature );
    }
    OGRDataSource::DestroyDataSource( poDS );

    // Create the shapefile of intersection points
    poDS = poDriver->CreateDataSource( "/Users/ben/Desktop/pt_out1.shp", NULL );
    if( poDS == NULL ) {
        qWarning() << "Creation of output file failed.";
    }

    poLayer = poDS->CreateLayer( "pt_out", NULL, wkbPoint, NULL );
    if( poLayer == NULL ) {
        qWarning() << "Layer creation failed.";
    }

    if( poLayer->CreateField( &oField1 ) != OGRERR_NONE ) {
        qWarning() <<  "Creating Name field failed.";
    }

    if( poLayer->CreateField( &oField2 ) != OGRERR_NONE ) {
        qWarning() << "Creating Trips field failed.";
    }

    qDebug() << "shapes" << shapes.count();
    // Get all of the intersections between the linestrings
    for(auto it = shapes.begin(); it != shapes.end(); ++it) {
        OGRLineString* ls1 = it.value();
        int count1 = shapesToTrips.value(it.key())->size();
        for(auto jt = it+1; jt != shapes.end(); jt++) {
            OGRLineString* ls2 = jt.value();
            int count2 = shapesToTrips.value(jt.key())->size();
            OGRGeometry* geom = ls1->Intersection(ls2);
            if(!geom->IsEmpty()) {

                if(wkbFlatten(geom->getGeometryType()) == wkbPoint) {
                    OGRPoint* pt = (OGRPoint*) geom;

                } else if(wkbFlatten(geom->getGeometryType()) == wkbMultiPoint) {
                    OGRMultiPoint* mPt = (OGRMultiPoint*) geom;
                    for(int i = 0; i < mPt->getNumGeometries(); ++i) {
                        OGRGeometry* mGeom = mPt->getGeometryRef(i);

                        if(wkbFlatten(mGeom->getGeometryType()) == wkbPoint) {
                            OGRPoint* pt  = (OGRPoint*) mGeom->clone();
                            OGRFeature* poFeature;
                            poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
                            QString name = it.key() + "-" + jt.key() + "-" + QString::number(i);
                            poFeature->SetField( "Name", name.toLatin1().data() );
                            poFeature->SetField( "Trip", count1 * count2 );
                            poFeature->SetGeometry( pt );

                            qDebug() << "we have:" << poFeature->GetFieldCount() << poFeature->GetFieldAsString("Name");

                            if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE ) {
                                qWarning() << "Failed to create feature in shapefile.";
                            }

                            OGRFeature::DestroyFeature( poFeature );
                        }
                    }
                }
            }
        }
    }

    OGRDataSource::DestroyDataSource( poDS );

    qDebug() << "[DONE] written shapefile";
}
