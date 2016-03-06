//
// Created by Benjamin Baron on 21/02/16.
//

#ifndef LOCALL_FLICKR_LAYER_H
#define LOCALL_FLICKR_LAYER_H

#include "utils.h"
#include "progress_dialog.h"
#include "trace_layer.h"
#include <QAction>

class SpatialStats;

class FlickrTrace: public Trace {
public:
    FlickrTrace(QString filename): Trace(filename) { }

    virtual bool openTrace(Loader* loader);

private:
    QHash<QString, QMap<long long, QPointF>*> _nodes;

};

class FlickrLayer : public TraceLayer {
    Q_OBJECT
public:
    FlickrLayer(MainWindow* parent = 0, QString name = 0, FlickrTrace* trace = nullptr):
            TraceLayer(parent, name, trace) {
        if(parent) {
            addBarMenuItems();
        }
    }

    QGraphicsItemGroup* draw();
    QString getInformation() { return "Flickr: " + _name; }
    bool load(Loader* loader) {
        _trace->openTrace(loader);
        return true;
    }

private:
    virtual void addBarMenuItems();
};


#endif //LOCALL_FLICKR_LAYER_H
