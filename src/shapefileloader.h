#ifndef SHAPEFILELOADER_H
#define SHAPEFILELOADER_H

#include <QString>
#include <QGraphicsScene>
#include <QtConcurrent>
#include <QGraphicsItemGroup>
#include <ogrsf_frmts.h>

#include "loader.h"
#include "mainwindow.h"

class ShapefileLoader: public Loader
{
public:
    ShapefileLoader(MainWindow* parent = 0, QString filename = 0, Layer* layer = 0):
        Loader(parent, filename, layer) { }
    ~ShapefileLoader();

private:
    bool concurrentLoad();

    bool loadWKT();
    bool loadShapefile();
};

#endif // SHAPEFILELOADER_H
