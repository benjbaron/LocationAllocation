#ifndef TRACELAYER_H
#define TRACELAYER_H

#include "layer.h"
#include "utils.h"
#include "progress_dialog.h"
#include "trace.h"
#include <QAction>

class SpatialStatsLayer;

class TraceLayer: public Layer
{
    Q_OBJECT
public:
    TraceLayer(MainWindow* parent = 0, QString name = 0, Trace* trace = nullptr, bool showMenu = true):
        Layer(parent, name), _trace(trace) {
        if(parent && showMenu) {
            addMenuBar();
            hideMenu();
        }
    }

    virtual QGraphicsItemGroup* draw();
    virtual bool load(Loader* loader);
    virtual void addMenuBar();

    /* Export functions */
    bool exportLayer(Loader* loader, QString output);
    bool exportLayerONE(Loader* loader, QString output);
    bool exportLayerText(Loader* loader, QString output, long long duration = 86400);
    bool exportLayerGrid(Loader* loader, QString output, int cellSize = 200, long long duration = 86400);

protected:
    SpatialStatsLayer* _spatialStatsLayer = 0;
    Trace* _trace;

};

#endif // TRACELAYER_H
