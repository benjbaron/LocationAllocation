#include "shapefileloader.h"

#include <QString>
#include <QSet>
#include <QGraphicsItemGroup>
#include <QDebug>

#include "constants.h"
#include "shapefilelayer.h"

ShapefileLoader::~ShapefileLoader() { }

bool ShapefileLoader::concurrentLoad()
{
    OGRRegisterAll();
    OGRDataSource *poDS;

    poDS = OGRSFDriverRegistrar::Open(_filename.toStdString().c_str(), FALSE);
    if( poDS == NULL )
    {
       qWarning() << "Open failed for " << _filename;
       return false;
    }

    // Set the accepted featue fields
    QSet<QString> acceptedHWFieldsSet = QSet<QString>();
    acceptedHWFieldsSet << "primary_link" << "tertiary_link" << "trunk_link" << "motorway" << "road" <<  "secondary_link" << "tertiary" << "motorway_link" << "secondary" << "trunk" << "primary";

    for(int i = 0; i < poDS->GetLayerCount(); ++i) {
       OGRLayer  *poLayer = poDS->GetLayer(i);
       qDebug() << "Loading layer" << QString::fromStdString(poLayer->GetName()) << "...";
       OGRFeature *poFeature;
       OGRSpatialReference *poTarget = new OGRSpatialReference();
       poTarget->importFromProj4(_projOut.toLatin1().data());

       poLayer->ResetReading();
       OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
       int hwIdx = poFDefn->GetFieldIndex("type");
       if(hwIdx == -1)
           hwIdx = poFDefn->GetFieldIndex("highway");


       qDebug() << "highway field index" << hwIdx;
       int nrofFeatures = 0;
       while( (poFeature = poLayer->GetNextFeature()) != NULL )
       {
           QString HWFieldStr = QString::fromStdString(poFeature->GetFieldAsString(hwIdx));
           if(acceptedHWFieldsSet.contains(HWFieldStr)) {
               OGRGeometry *poGeometry = poFeature->GetGeometryRef();
               if( poGeometry != NULL)
                {
                   poGeometry->transformTo(poTarget);
                   OGRPoint * pt = (OGRPoint *) poGeometry;
                   ((ShapefileLayer*)_layer)->addGeometry(pt);

               }
               else if (poGeometry != NULL
                        && wkbFlatten(poGeometry->getGeometryType()) == wkbLineString)
               {
                   poGeometry->transformTo(poTarget);
                   OGRLineString * ls = (OGRLineString *) poGeometry;
                   ((ShapefileLayer*)_layer)->addGeometry(ls);
               }
           }
           if(nrofFeatures % 100 == 0)
           {
               qreal loadProgress = nrofFeatures / (qreal) poLayer->GetFeatureCount();
               emit loadProgressChanged(loadProgress);
           }
           nrofFeatures++;
       }
    }

    emit loadProgressChanged((qreal)1.0);
    // Do not delete any structure as they will be used later
    qDebug() << "loaded shapefile" << QFileInfo(_filename).fileName() << "with" << ((ShapefileLayer*)_layer)->countGeomerties() << "features";
    return true;
}

