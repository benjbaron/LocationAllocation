#ifndef TRACELAYER_H
#define TRACELAYER_H

#include "layer.h"
#include "utils.h"
#include "progressdialog.h"
#include <QAction>

class SpatialStats;

class TraceLayer: public Layer
{
    Q_OBJECT
public:
    TraceLayer(MainWindow* parent = 0, QString name = 0, QString filename = 0):
        Layer(parent, name), _filename(filename)
    {
        addBarMenuItems();
        hideMenu();
    }

    /* Adds successive points to the "_nodes" hash */
    void addPoint(QString node, long long ts, double lat, double lon) {
        if(!_nodes.contains(node)) {
            // create the corresponding Map of successive positions
            _nodes.insert(node, new QMap<long long, QPointF>());
        }
        // update the node position
        _nodes.value(node)->insert(ts, QPoint(lat, lon));

        // update the startTime and endTime
        if(ts >= 0 && ts < _startTime) _startTime = ts;
        if(ts >= 0 && ts > _endTime) _endTime = ts;
    }

    QGraphicsItemGroup* draw();

    OGRGeometryCollection *getGeometry(long long startTime = 0, long long endTime = 0);

    QHash<QString, QMap<long long, QPointF>*> getNodes() const { return _nodes; }
    QMap<long long, QPointF>* const getNodeTrace(QString node) { return _nodes.value(node); }
    double getAverageSpeed();
    long long getStartTime() { return _startTime; }
    long long getEndTime() { return _endTime; }

    double getAverageSampling() {
        if(_sampling < 0.0) {
            // compute the sampling
            long long sum = 0;
            long long count = 0;
            for(auto it = _nodes.begin(); it != _nodes.end(); ++it) {
                auto jt = it.value()->begin();
                long long prevTimeStamp = jt.key();
                for(jt++; jt != it.value()->end(); ++jt) {
                    long long curTimeStamp = jt.key();
                    sum += (curTimeStamp - prevTimeStamp);
                    count++;
                    prevTimeStamp = curTimeStamp;
                }
            }
            _sampling = ((double)sum) / ((double) count);
        }
        return _sampling;
    }

    /* Load functions */
    bool load(Loader* loader);
    void openNodeTrace(QString filename);
    void openDieselNetNodeFolder(QString dirname);
    void openDieselNetNodeTrace(QString filename, QString node);

    /* Export functions */
    void exportLayer(QString output);
    void exportLayerONE(QString output);
    void exportLayerText(QString output, long long duration = 86400);
    void exportLayerGrid(QString output, int cellSize = 200, long long duration = 86400);

signals:
    void loadProgressChanged(qreal);

private:
    QString _filename;
    QHash<QString, QMap<long long, QPointF>*> _nodes;
    Distribution _averageSpeeds;
    long long _startTime = (long long) 1e20;
    long long _endTime = -1;
    double _sampling = -1;
    SpatialStats* _spatialStats = 0;

    void addBarMenuItems();
};

#endif // TRACELAYER_H
