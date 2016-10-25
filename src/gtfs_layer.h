//
// Created by Benjamin Baron on 08/02/16.
//

#ifndef LOCALL_GTFS_LOADER_H
#define LOCALL_GTFS_LOADER_H

#include <QObject>
#include <QString>
#include <QPointF>
#include <QMap>

#include "trace_layer.h"
#include "gtfs_trace.h"


class GTFSLayer : public TraceLayer {
    Q_OBJECT
public:
    GTFSLayer(MainWindow* parent = 0, QString name = 0, GTFSTrace* trace = nullptr):
            TraceLayer(parent, name, trace, false) {
        if(parent) {
            addMenuBar();
        }
    }

    virtual QGraphicsItemGroup* draw();
    virtual void addMenuBar();

    QString getInformation() {
        return "GTFS Layer: " + _name;
    }
    bool load(Loader* loader) {
        _trace->openTrace(loader);
        return true;
    }

private slots:
    void exportStops();
    void exportONESettings();
    void exportTrajectoryShapefile();

private:
    void toWKTFile(geos::geom::LineString* ls, const QString& output);
};

#endif //LOCALL_GTFS_LOADER_H
