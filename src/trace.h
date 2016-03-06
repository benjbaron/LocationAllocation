//
// Created by Benjamin Baron on 02/03/16.
//

#ifndef LOCALL_TRACE_H
#define LOCALL_TRACE_H

#include <QString>
#include <QPointF>
#include <QMap>

#include "utils.h"

// forward class declarations
class Loader;

class Trace {

public:
    Trace(const QString& filename):
            _filename(filename) { }

    virtual bool openTrace(Loader* loader);

    /* open different traces */
    void openNodeTrace(QString filename);
    void openDieselNetNodeFolder(QString dirname);
    void openDieselNetNodeTrace(QString filename, QString node);

    /* Adds successive points to the "_nodes" hash */
    void addPoint(QString node, long long ts, double lat, double lon) {
        if(!_nodes.contains(node)) {
            // create the corresponding Map of successive positions
            _nodes.insert(node, new QMap<long long, QPointF>());
        }
        // update the node position
        _nodes.value(node)->insert(ts, QPointF(lat, lon));

        // update the startTime and endTime
        if(ts >= 0 && ts < _startTime) _startTime = ts;
        if(ts >= 0 && ts > _endTime) _endTime = ts;
    }

    double averageSampling() {
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

    void getNodes(QHash<QString, QMap<long long, QPointF>*>* nodes) {
        *nodes = _nodes;
    }
    void getNodeTrace(QMap<long long, QPointF>** nodeTrace, QString node) {
        *nodeTrace = _nodes.value(node);
    }
    long long getStartTime() {
        return _startTime;
    }
    long long getEndTime() {
        return _endTime;
    }
    int getNbNodes() const {
        return _nodes.size();
    }
    double averageSpeed();
    QString getName() const {
        qDebug() << "name of the trace: " << _filename;
        return QFileInfo(_filename).fileName();
    }

protected:
    const QString _filename;
    QHash<QString, QMap<long long, QPointF>*> _nodes;
    Distribution _averageSpeeds;
    long long _startTime = (long long) 1e20;
    long long _endTime = -1;
    double _sampling = -1;
};


#endif //LOCALL_TRACE_H
