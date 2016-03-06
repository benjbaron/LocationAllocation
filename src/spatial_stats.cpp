#include "spatial_stats.h"

#include "loader.h"

SpatialStats::SpatialStats(Trace* trace,
                           long long sampling,
                           long long startTime,
                           long long endTime,
                           GeometryIndex *geometryIndex):
    _trace(trace),
    _sampling(sampling),
    _startTime(startTime),
    _endTime(endTime),
    _geometryIndex(geometryIndex) { }

void SpatialStats::populateMobileNodes(Loader* loader) {
    QString currentMsg = "Populate the nodes ("
                         +QString::number(_startTime)+" -> "+QString::number(_endTime)
                         +", "+QString::number(_sampling)+")";

    // add the successive point positions of the mobile nodes
    QHash<QString, QMap<long long, QPointF>*> nodes;
    _trace->getNodes(&nodes);

    int count = 0;
    for(auto it = nodes.begin(); it != nodes.end(); ++it) {
        QString nodeId = it.key();
        if(!_mobileNodes.contains(nodeId) && it.value()->size() > 0) {
            _mobileNodes.insert(nodeId, new MobileNode(nodeId, (int) _sampling, this));
        }
        MobileNode* node = _mobileNodes.value(nodeId);
        if(it.value()->lastKey() < _startTime)
            continue;

        auto jt = (_startTime == -1) ? it.value()->begin() : it.value()->lowerBound(_startTime);
        for(; jt != it.value()->end(); ++jt) {
            if(_endTime != -1 && jt.key() > _endTime) break;
            long long ts = jt.key();
            QPointF pos = jt.value();
            node->addPosition(ts, pos.x(), pos.y());
        }

        count++;
        loader->loadProgressChanged(0.10 * ((qreal) count / (qreal) nodes.size()), currentMsg);
//        if(loader) {
//            loader->loadProgressChanged();
//            loader->changeText(currentMsg);
//        } else {
//            printConsoleProgressBar(0.10 * ((qreal) count / (qreal) nodes.size()), currentMsg);
//        }
    }
}

bool SpatialStats::computeStats(Loader* loader) {
    QString currentMsg = "Populate the nodes";
    loader->loadProgressChanged(0.0, currentMsg);

    populateMobileNodes(loader);

    // compute the visiting matrix for the current set of mobile nodes
    int nbNodes = _mobileNodes.size();

    currentMsg = "Compute visit matrix ("+QString::number(nbNodes)+" nodes)";
    loader->loadProgressChanged(0.1, currentMsg);

    qDebug() << currentMsg;

    int count = 0;
    for(auto it_mobileNode = _mobileNodes.begin(); it_mobileNode != _mobileNodes.end(); ++it_mobileNode) {
        MobileNode* mobileNode = it_mobileNode.value();
        auto geoms = mobileNode->getGeometries(); // get the set of geometries the node visits
        for(auto it = geoms.begin(); it != geoms.end(); ++it) {
            long long start1 = it.key();

            // loop through the geometries visited at the same start time start1
            for(auto kt = it.value()->begin(); kt != it.value()->end(); ++kt) {
                Geometry* geom1 = kt.key();
                long long end1  = kt.value();

                // add the geometry to the set of visited geometries
                if(!_geometries.contains(geom1))
                    _geometries.insert(geom1, new GeometryValue(geom1));

                // update the corresponding geometry value
                GeometryValue* val = _geometries.value(geom1);
                val->visits.insert(start1, end1);
                val->visitFrequency.append(start1);
                val->nodes.insert(it_mobileNode.key());

                // examine the subsequent visited geometries
                QSet<Geometry*> visitedGeometries; // remember the cells the node visited
                                                   // to avoid accounting for them multiple times
                for(auto jt = it+1; jt != geoms.end(); ++jt) {
                    long long start2 = jt.key();

                    // loop through the geometries visited at the same start time start2
                    for(auto lt = jt.value()->begin(); lt != jt.value()->end(); ++lt) {
                        Geometry* geom2 = lt.key();
                        long long end2  = lt.value();

                        // stop when the node visits the same stating geometry again
                        if(geom1 == geom2) break;

                        // do not take account already visited geometries
                        if(visitedGeometries.contains(geom2))
                            continue;

                        // add the geometry to the matrix of visited geometries
                        if(!_geometryMatrix.contains(geom1)) {
                            _geometryMatrix.insert(geom1, new QHash<Geometry*, GeometryMatrixValue*>());
                        }
                        if(!_geometryMatrix.value(geom1)->contains(geom2)) {
                            _geometryMatrix.value(geom1)->insert(geom2, new GeometryMatrixValue(geom1, geom2));
                        }

                        GeometryMatrixValue* matVal = _geometryMatrix.value(geom1)->value(geom2);
                        matVal->travelTimeDist.addValue((int) qMax((long long) 0, start2-start1));
                        matVal->visitFrequency.append(start1);
                        matVal->visits.insert(start1, end1);
                        matVal->nodes.insert(it_mobileNode.key());

                        visitedGeometries.insert(geom2);
                    }
                }
            }
        }

        // update the UI or console
        count++;
        loader->loadProgressChanged(0.1 + 0.4 * ((qreal) count / (qreal) nbNodes), currentMsg);
    }

    currentMsg = "Inter-visit ("
                 +QString::number(_geometryMatrix.size())+")"
                 +" matrix size, ("
                 +QString::number(_geometries.size())
                 +" nodes size)";
    loader->loadProgressChanged(0.4, currentMsg);

    currentMsg = "Compute inter-visit durations (cells)";
    // compute the inter-visit durations for the cells
    count = 0;
    int size = _geometries.size();
    for(auto it = _geometries.begin(); it != _geometries.end(); ++it) {
        Geometry* geom = it.key();
        GeometryValue* val = it.value();
        auto visits = val->visits;
        long long prevStartTime = visits.firstKey();
        for(auto kt = visits.begin(); kt != visits.end(); ++kt) {
            long long start = kt.key();
            foreach(long long end, visits.values(start)) {
                val->interVisitDurationDist.addValue((int) (start-prevStartTime));
                prevStartTime = start;
            }
        }
        val->localStat = computeLocalStat(geom);
        val->color = selectColorForLocalStat(val->localStat);

        // update the UI
        count++;
        loader->loadProgressChanged(0.5 + 0.16 * ((qreal) count / (qreal) size), currentMsg);
    }

    currentMsg = "Compute inter-visit durations (matrix)";
    loader->loadProgressChanged(0.66, currentMsg);

    // compute the inter-visit durations for the matrix cells
    count = 0;
    size = _geometryMatrix.size();
    for(auto it = _geometryMatrix.begin(); it != _geometryMatrix.end(); ++it) {
        Geometry* geom1 = it.key();
        auto geoms = it.value();
        for(auto jt = geoms->begin(); jt != geoms->end(); ++jt) {
            Geometry* geom2 = jt.key();
            GeometryMatrixValue* val = jt.value();
            auto visits = val->visits;
            long long prevStartTime = visits.firstKey();
            for(auto kt = visits.begin(); kt != visits.end(); ++kt) {
                long long start = kt.key();
                foreach(long long end, visits.values(start)) {
                    val->interVisitDurationDist.addValue((int) (start-prevStartTime));
                    prevStartTime = start;
                }
            }

            // update the connections counters
            if(_geometryMatrix.contains(geom2)) {
                if(_geometryMatrix.value(geom2)->contains(geom1)) {
                    if(_geometries.contains(geom2)) {
                        _geometries.value(geom2)->connections++;
                        int travelTime = (int) qCeil(_geometryMatrix.value(geom2)->value(geom1)->travelTimeDist.getAverage());
                        _geometries.value(geom2)->travelTimes.addValue(travelTime);
                    }
                }
            }
        }
        count++;
        loader->loadProgressChanged(0.66 + 0.16 * ((qreal) count / (qreal) size), currentMsg);
    }

    currentMsg = "Compute scores";
    loader->loadProgressChanged(0.82, currentMsg);

    count = 0;
    size = _geometryMatrix.size();
    for(auto it = _geometryMatrix.begin(); it != _geometryMatrix.end(); ++it) {
        Geometry* geom1 = it.key();
        GeometryValue* nodeVal = _geometries.value(geom1);
        double nodeAvg = nodeVal->interVisitDurationDist.getAverage() > 0.0 ? nodeVal->interVisitDurationDist.getAverage() : 1.0;
        double nodeMed = nodeVal->interVisitDurationDist.getMedian() > 0.0 ? nodeVal->interVisitDurationDist.getMedian() : 1.0;
        double nodeCount = (double) nodeVal->visits.size();
        double nodeMedScore = nodeCount / nodeMed;
        double nodeAvgScore = nodeCount / nodeAvg;
        if(nodeVal) nodeVal->medScore = nodeMedScore;
        if(nodeVal) nodeVal->avgScore = nodeAvgScore;

        auto geoms = it.value();
        for(auto jt = geoms->begin(); jt != geoms->end(); ++jt) {
            Geometry* geom2 = jt.key();
            GeometryMatrixValue* edgeVal = jt.value();
            double edgeAvg = edgeVal->interVisitDurationDist.getAverage() > 0.0 ? edgeVal->interVisitDurationDist.getAverage() : 1.0;
            double edgeMed = edgeVal->interVisitDurationDist.getMedian() > 0.0 ? edgeVal->interVisitDurationDist.getMedian() : 1.0;
            double edgeCount = (double) edgeVal->visits.size();
            double edgeMedScore = edgeCount / edgeMed;
            double edgeAvgScore = edgeCount / edgeAvg;

            if(_geometries.contains(geom2)) _geometries.value(geom2)->medIncomingScore += edgeMedScore;
            if(_geometries.contains(geom2)) _geometries.value(geom2)->avgIncomingScore += edgeAvgScore;
            edgeVal->medScore = edgeMedScore;
            edgeVal->avgScore = edgeAvgScore;
        }

        // update the UI and console
        count++;
        loader->loadProgressChanged(0.82 + 0.16 * ((qreal) count / (qreal) size), currentMsg);
    }

    // TODO  Only one loader for both console and GUI
    loader->loadProgressChanged((qreal) 1.0, "Done");
    std::cout << std::endl;

    return true;
}

QColor SpatialStats::selectColorForLocalStat(qreal zScore) {
    if(zScore >= 3.291) return QColor("#720206");
    else if(zScore >= 2.576) return QColor("#f33f1c");
    else if(zScore >= 1.960) return QColor("#f37b22");
    else if(zScore >= 1.645) return QColor("#fffe38");
    else return QColor("#cccccc");
}

qreal SpatialStats::computeLocalStat(Geometry* geom_i) {
    qreal sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0, sum5 = 0.0;
    int n = _geometries.size();
    for(auto it = _geometries.begin(); it != _geometries.end(); ++it) {
        int count_j = it.value()->visits.size();
        Geometry* geom_j = it.key();
        int weight = (int) (1.0 / (0.01 + (qreal) qSqrt(qPow(geom_i->getCenter().x() - geom_j->getCenter().x(), 2) +
                                                        qPow(geom_i->getCenter().y() - geom_j->getCenter().y(), 2))));
        sum1 += weight * count_j;
        sum2 += weight;
        sum3 += qPow(weight,2);
        sum4 += count_j;
        sum5 += qPow(count_j,2);
    }

    //!\\ TODO get p-scores

    qreal mean = sum4 / n;
    qreal S = qSqrt(sum5 / n - qPow(mean, 2));
    return (sum1 - mean * sum2) / (S * qSqrt((n * sum3 - qPow(sum2,2)) / (n-1)));
}


void MobileNode::addPosition(long long time, double x, double y) {
    // assuming the positions are added sequentially
    if(_prevPos.isNull() || time - _prevTime > 300) { // restart the cell recording
        // get the list of geometries that contain the current position
        QSet<Geometry*> geoms;
        _spatialStats->containsPoint(&geoms, x,y);
        _startTimeGeometries.clear();
        // record all geometries
        for(Geometry* geom : geoms) {
            _startTimeGeometries.insert(geom, time);
            if(!_visitedGeometries.contains(time))
                _visitedGeometries.insert(time, new QHash<Geometry*,long long>());
            _visitedGeometries.value(time)->insert(geom,time);
        }
    } else { // increase the end time of the current recorded geometries
        QPointF pos(x,y); // position of the node
        // number of intermediate positions (with the sampling)
        int nbPos = qMax(1, qCeil((time - _prevTime) / _sampling));
        for(int i = 1; i <= nbPos; ++i) {
            long long t = _prevTime + i*_sampling; // get the sampling time
            QPointF p = (time - t)*_prevPos + (t - _prevTime)*pos;
            p /= (time - _prevTime);

            // get the corresponding visited Geometries
            QSet<Geometry*> geoms;
            _spatialStats->containsPoint(&geoms, p);

            // start recording all the new geometries
            QSet<Geometry*> newGeometries = geoms - _prevGeometries;
            for(Geometry* geom : newGeometries) {
                _startTimeGeometries.insert(geom, time);
                if(!_visitedGeometries.contains(time))
                    _visitedGeometries.insert(time, new QHash<Geometry*,long long>());
                _visitedGeometries.value(time)->insert(geom,time);
            }
            // update the geometries that are currently being recorded
            QSet<Geometry*> currentGeometries = geoms & _prevGeometries;
            for(Geometry* geom : currentGeometries) {
                long long startTimeGeom = _startTimeGeometries.value(geom);
                _visitedGeometries.value(startTimeGeom)->insert(geom, time);
            }
        }
    }
    _prevPos = QPointF(x,y);
    _prevTime = time;
}
