#include "compute_allocation.h"

#include "allocation_dialog.h"
#include "spatial_stats.h"
#include "mainwindow.h"
#include "loader.h"

ComputeAllocation::ComputeAllocation(SpatialStats *spatialStats):
    _spatialStats(spatialStats) { }

/*
 * Compute the location allocation
*/
void ComputeAllocation::computeAllocation() {
    qDebug() << "Compute location allocation";

    AllocationDialog diag(_spatialStats->getParent());
    if (diag.exec() == QDialog::Rejected) {
        return;
    }

    // get the parameters
    long long deadline = diag.getDeadline();
    int nbStorageNodes = diag.getNbStorageNodes();
    double deletionFactor = diag.getDeletionFactor();
    TravelTimeStat ts = diag.getTravelTimeStat();
    DistanceStat   ds = diag.getDistanceStat();
    double travelTime = diag.getTravelTime();
    double distance = diag.getDistance();
    QString method = diag.getMethod();

    QHash<Geometry*, Allocation*> allocation;
    processAllocationMethod(method,nbStorageNodes,deadline,deletionFactor,ts,travelTime,ds,distance,&allocation);

    // print the resulting allocation
    for(auto it = allocation.begin(); it != allocation.end(); ++it) {
        qDebug() << "candidate" << it.key()->getCenter().x()
                 << "allocated" << it.value()->weight << "with" << it.value()->demands.size()
                 << "demands" << "deleted" << it.value()->deletedCandidates.size();
    }

    // create a new layer
    QString layerName = "Location allocation";
    WeightedAllocationLayer* layer = new WeightedAllocationLayer(_spatialStats->getParent(), layerName, allocation);
    Loader loader(layer);
    _allocationLayers.append(layer);
    _spatialStats->getParent()->createLayer(layerName, layer, &loader);
}

void ComputeAllocation::processAllocationMethod(QString method,
                                                int nbFacilities,
                                                double deadline,
                                                double delFactor,
                                                TravelTimeStat ttStat,
                                                double travelTime,
                                                DistanceStat dStat,
                                                double distance,
                                                QHash<Geometry*, Allocation*>* allocation,
                                                bool isMultiThreaded) {
    qDebug() << "processAllocationMethod" << method << nbFacilities << deadline << delFactor
             << ttStat << travelTime << dStat << distance;

    if(method == LOCATION_ALLOCATION_MEHTOD_NAME || method == PAGE_RANK_MEHTOD_NAME) {
        double maxTravelTime, maxDist;
        if(ttStat != NoneTT)
            maxTravelTime = travelTime;
        else
            maxTravelTime = delFactor * deadline;

        if(dStat == FixedD)
            maxDist = distance;
        else // use the average speed of the mobile users
            maxDist = delFactor * _spatialStats->getAverageSpeed() * deadline;

        qDebug() << deadline << nbFacilities << _spatialStats->getAverageSpeed() << maxDist << maxTravelTime;

        if(method == LOCATION_ALLOCATION_MEHTOD_NAME) {
            if(isMultiThreaded) {
                ProgressDialog progressDialog(_spatialStats->getParent(), "Computing location allocation ");
                connect(this, &ComputeAllocation::loadProgressChanged, &progressDialog, &ProgressDialog::updateProgress);
                connect(this, &ComputeAllocation::changeText, &progressDialog, &ProgressDialog::changeText);
                QtConcurrent::run([&](){
                    runLocationAllocation(nbFacilities,deadline,ttStat,travelTime,dStat,distance,allocation);
                });
                progressDialog.exec();
            } else {
                runLocationAllocation(nbFacilities,deadline,ttStat,travelTime,dStat,distance,allocation);
            }
        } else if(method == PAGE_RANK_MEHTOD_NAME) {
            return;
//            runPageRank(nbFacilities,ttStat,travelTime,dStat,distance,allocation);
        }
    } else if(method == K_MEANS_MEHTOD_NAME) {
        return;
//        runKMeans(nbFacilities,allocation);
    } else if(method == RANDOM_METHOD_NAME) {
        runRandomAllocation(nbFacilities, allocation);
    }
}

void ComputeAllocation::geomWithin(QSet<Geometry*>* geomWithin,
                                   QSet<Geometry*> geoms,
                                   Geometry* geom,
                                   double distance,
                                   double travelTime,
                                   DistanceStat ds,
                                   TravelTimeStat ts) {
    if(ds == NoneD && ts == NoneTT) return;

//    QRectF cell1(cell.x()*_geometriesize, cell.y()*+_geometriesize, _geometriesize, _geometriesize);
    foreach(Geometry* g, geoms) {
        if(g == geom) continue; // skip the geometry itself
        bool distance_flag = false, travelTime_flag = false;
        double dist, tt;
        if(ds != NoneD && distance > 0.0) {
            dist = g->distance(geom);
            if(std::islessequal(dist,distance)) {
                distance_flag = true;
            }
        }
        if(ts != NoneTT && travelTime > 0.0) {
            GeometryMatrixValue* val = _spatialStats->getValue(g,geom);
            if(val) {
                if(ts == Avg)
                    tt = val->travelTimeDist.getAverage();
                else if(ts == Med)
                    tt = val->travelTimeDist.getMedian();
                if(std::islessequal(tt,travelTime)) {
                    travelTime_flag = true;
                }
            }
        }

        if((ds != NoneD && distance_flag) || (ts != NoneTT && travelTime_flag)) {
            qDebug() << "or / add" << geom << g->toString()
                     << "distance" << ds << dist << distance
                     << "travelTime" << ts << tt << travelTime;
            geomWithin->insert(g);
        }
    }
}


void ComputeAllocation::runLocationAllocation(int nbFacilities,
                                              double deadline,
                                              TravelTimeStat ttStat,
                                              double travelTime,
                                              DistanceStat dStat,
                                              double distance,
                                              QHash<Geometry*, Allocation*>* allocation)
{
    emit loadProgressChanged((qreal) 0.0);
    emit changeText("Initialization");

    // demands are the geometries
    // candidates are the geometries

    qDebug() << "runLocationAllocation" << nbFacilities << deadline << ttStat << travelTime << dStat << distance;

    QSet<Geometry*> demandsToCover;
    QSet<Geometry*> allDemands;
    QSet<Geometry*> candidatesToAllocate;
    QSet<Geometry*> allCandidates;

    QSet<Geometry*> circlesGeometries;
    QSet<Geometry*> cellsGeometries;
    for(auto it = _spatialStats->getGeometries()->begin(); it != _spatialStats->getGeometries()->end(); ++it) {
        Geometry* geom = it.key();
        if(geom->getGeometryType() == CellType) cellsGeometries.insert(geom);
        else if(geom->getGeometryType() == CircleType) circlesGeometries.insert(geom);
    }

    if(circlesGeometries.size() > 0) allCandidates = circlesGeometries;
    else allCandidates = cellsGeometries;
    allDemands = cellsGeometries;

    candidatesToAllocate = allCandidates;
    demandsToCover = allDemands;

    // iterate for each storage node to allocate
    for(int i = 0; i < nbFacilities && !demandsToCover.isEmpty(); ++i) {
        emit changeText("Allocate for facility " + QString::number(i));
        qDebug() << "allocating demands for storage node" << i;
        double maxCoverage = 0.0;
        double maxBackendWeight = 0.0;
        Geometry* maxCoverageGeometry = NULL;
        QHash<Geometry*, double> maxDemandsCovered; // <demand, weight of the demand>

        // find the candidate that covers the most demands
        foreach(Geometry* k, candidatesToAllocate) {
//            qDebug() << "\tcandidate" << k;

            QHash<Geometry*, double> demandsCovered; // <demand, weight of the demand>
            double coverage = 0.0;

            // compute the score for the previously allocated storage nodes
            foreach(Geometry* c, allocation->keys()) {
                GeometryMatrixValue* val = _spatialStats->getValue(c,k);
                if(val) {
                    double visitCount = val->visits.size();
                    if(visitCount > 1) {
                        coverage += val->avgScore;
                    }
//                    double interVisitTime = val->interVisitDurationDist.getAverage();
//                    double visitCount = val->visits.size();
//                    if(visitCount > 1) {
//                        coverage += visitCount / interVisitTime;
//                    }
                }

                val = _spatialStats->getValue(k,c);
                if(val) {
                    double visitCount = val->visits.size();
                    if(visitCount > 1) {
                        coverage += val->avgScore;
                    }
//                    double interVisitTime = val->interVisitDurationDist.getAverage();
//                    double visitCount = val->visits.size();
//                    if(visitCount > 1) {
//                        coverage += visitCount / interVisitTime;
//                    }
                }
            }

            double backendWeight = coverage;

            // Compute the covering score for the candidate
            foreach(Geometry* l, demandsToCover) {
                GeometryMatrixValue* val = _spatialStats->getValue(l,k);
                if(val) {
                    double medTravelTime = val->travelTimeDist.getMedian();
                    if(medTravelTime <= deadline) {
                        double visitCount = val->visits.size();
                        if(visitCount > 1) {
                            double score = val->avgScore;
                            coverage += score; // weight of the demand
                            demandsCovered.insert(l, score);
                        }
                    }
                }
            }

            if(coverage > maxCoverage) {
                maxCoverage = coverage;
                maxBackendWeight = backendWeight;
                maxCoverageGeometry = k;
                maxDemandsCovered = QHash<Geometry*, double>(demandsCovered);
            }
        }

        // reduce the set of the population to cover
        if(maxCoverageGeometry) {
            candidatesToAllocate.remove(maxCoverageGeometry); // remove the selected cell
            demandsToCover.remove(maxCoverageGeometry);

            // remove the candidate cells in the vicinity of the selected cell
            QSet<Geometry*> candidatesToRemove;
            geomWithin(&candidatesToRemove, candidatesToAllocate, maxCoverageGeometry,
                       distance, travelTime, dStat, ttStat);
            candidatesToAllocate.subtract(candidatesToRemove);

            qDebug() << "\tcell" << maxCoverageGeometry->toString() << candidatesToRemove.size();

            demandsToCover.subtract(maxDemandsCovered.keys().toSet());

            // add the allocation
            Allocation* alloc = new Allocation(maxCoverageGeometry, maxCoverage,
                                               maxDemandsCovered, candidatesToRemove, i);
            allocation->insert(maxCoverageGeometry, alloc);
        }

        /* Substitution part of the algorithm */
        // -> tries to replace each facility one at a time with a facility at another "free" site
        emit changeText("Substitution for facility " + QString::number(i));
        for(Geometry* k : allocation->keys()) {
            // get the demands already covered by the current candidate

            Allocation* alloc = allocation->value(k);
            QSet<Geometry*> prevDemandsCovered = alloc->demands.keys().toSet();
            double prevWeight = alloc->weight;
            QSet<Geometry*> prevDeletedCandidates = alloc->deletedCandidates;

            double maxNewCoverage = 0.0;
            Geometry* maxNewCoverageGeometry;
            QHash<Geometry*, double> maxNewDemandsCovered;

            for(Geometry* k1 : allCandidates - allocation->keys().toSet()) {
                if(k1 == maxCoverageGeometry) continue;

                QHash<Geometry*, double> demandsCovered; // <demand, weight of the demand>
                double coverage = 0.0;

                // all allocated candidates backend cost
                for(Geometry* c : allocation->keys()) {
                    GeometryMatrixValue* val = _spatialStats->getValue(c,k1);
                    if(val) {
                        double visitCount = val->visits.size();
                        if(visitCount > 1) {
                            coverage += val->avgScore;
                        }
                    }

                    val = _spatialStats->getValue(k1,c);
                    if(val) {
                        double visitCount = val->visits.size();
                        if(visitCount > 1) {
                            coverage += val->avgScore;
                        }
                    }
                }

                // all demands to cover + those covered by the current candidate
                for(Geometry* l : demandsToCover + prevDemandsCovered) {
                    GeometryMatrixValue* val = _spatialStats->getValue(l,k);
                    if(val) {
                        double medTravelTime = val->travelTimeDist.getMedian();
                        if(medTravelTime <= deadline) {
                            double visitCount = val->visits.size();
                            if(visitCount > 1) {
                                double score = val->avgScore;
                                coverage += score; // weight of the demand
                                demandsCovered.insert(l, score);
                            }
                        }
                    }
                }
                if(coverage > maxNewCoverage) {
                    maxNewCoverage = coverage;
                    maxNewCoverageGeometry = k;
                    maxNewDemandsCovered = QHash<Geometry*, double>(demandsCovered);
                }
            }

            if(prevWeight < maxNewCoverage) {
                // change the candidate's current allocation
                qDebug() << "changed candidate / old" << prevWeight << "new" << maxNewCoverage;

                Allocation* alloc = allocation->value(k);
                alloc->weight = maxNewCoverage;
                alloc->demands = maxNewDemandsCovered;
                QSet<Geometry*> candidatesToRemove;
                geomWithin(&candidatesToRemove,
                           candidatesToAllocate + prevDeletedCandidates,
                           maxNewCoverageGeometry,
                           distance, travelTime, dStat, ttStat);
                alloc->deletedCandidates = candidatesToRemove;
                alloc->geom = k;
                // TODO Change the new candidate's rank
            }
        }

        qDebug() << "candidate" << ((maxCoverageGeometry) ? maxCoverageGeometry->toString() : "None")
                 << "allocated" << maxCoverage << "with" << maxDemandsCovered.size() << "demands"
                 << demandsToCover.size() << "demands to cover" << maxBackendWeight << "backend weight";

        // update the progress
        emit loadProgressChanged((qreal) i / (qreal) nbFacilities);
    }

    emit changeText("Done");
    emit loadProgressChanged((qreal) 1.0);
}


void ComputeAllocation::runRandomAllocation(int nbFacilities, QHash<Geometry*, Allocation*>* allocation) {
    QList<Geometry*> toAllocate;
    for(auto it = _spatialStats->getGeometries()->begin(); it != _spatialStats->getGeometries()->end(); ++it) {
        toAllocate.append(it.key());
    }

    for(int i = 0; i < nbFacilities; ++i) {
        // choose a random candidate to allocate
        int idx = qrand() % toAllocate.size();
        Geometry* geom = toAllocate.at(idx);
        toAllocate.removeAt(idx);
        allocation->insert(geom, new Allocation(geom, 1.0));

    }
}


/*
 * PageRank function
 */
/*
bool ComputeAllocation::pageRank(QHash<QPoint, double> &x, QSet<QPoint> cells, double alpha, int maxIterations, double tolerance)
{
    int N = cells.size();
    QHash<QPoint, double> xlast, danglingWeights, p;
    for(auto n : cells) {
        double v = 1.0/N;
        x.insert(n, v);
        danglingWeights.insert(n, v);
        p.insert(n, v);
    }

    QSet<QPoint> danglingNodes = QSet<QPoint>(cells);
    for(auto it = _geometryMatrix.begin(); it != _geometryMatrix.end(); ++it) {
        // remove the nodes that have out-edges
        if(it.value()->size() > 0)
            danglingNodes.remove(it.key());
    }

    for(int i = 0; i < maxIterations; ++i) {
        xlast = x;
        xlast.detach(); // deep copy

        for(auto it = x.begin(); it != x.end(); ++it) {
            x[it.key()] = 0.0;
        }

        double dangleSum = 0.0;
        for(QPoint node : danglingNodes) {
            dangleSum += xlast[node];
        }
        dangleSum *= alpha;

        for(auto it = x.begin(); it != x.end(); ++it) {
            QPoint n = it.key(); // node
            if(!_geometryMatrix.contains(n)) continue;
            auto neighbors = _geometryMatrix.value(n);
            double nbNeighbors = (double) neighbors->size(); // number of outbound links of n
            for(auto jt = neighbors->begin(); jt != neighbors->end(); ++jt) {
                QPoint nbr = jt.key(); // neighbor
                auto val = jt.value(); // value for edge n -> nbr
                double weight = val->medScore;
//                double median = val->interVisitDurationDist.getMedian();
//                double weight = median > 0 ? ((double) val->visits.size()) / median : ((double) val->visits.size());
                double edgeWeight = nbNeighbors > 0 ? weight / nbNeighbors : 0.0;
//                double before = x[nbr];
                x[nbr] += alpha * xlast[n] * edgeWeight;
//                qDebug() << "\t\t edge" << n << "->" << nbr << "/" << edgeWeight << xlast[n] << before << x[nbr] << nbNeighbors;
            }
            x[n] += dangleSum * danglingWeights[n] + (1.0 - alpha) * p[n];
//            qDebug() << "\t node" << n << "/" << x[n] << dangleSum << danglingWeights[n] << p[n];
        }

        double err = 0.0;
        for(auto n : x.keys()) {
            err += qAbs(x[n] - xlast[n]);
        }
        qDebug() << "iteration" << (i+1) << x.size() << err;
        if(err < N*tolerance)
            return true;
    }

    // did not converge
    return false;
}


void ComputeAllocation::runPageRank(int nbFacilities, double deadline, double delFactor, TravelTimeStat ttStat, double travelTime, DistanceStat dStat, double distance, QHash<QPoint, Allocation*>& allocation) {
    // initialize the candidates
    QSet<QPoint> candidatesToAllocate(_geometries.keys().toSet());

    for(int i = 0; i < nbFacilities; ++i) {
        // run pagerank with the current set of candidate cells
        QHash<QPoint, double> x; // result array
        bool pageRankRet = pageRank(x, candidatesToAllocate);

        if(!pageRankRet) {
            qDebug() << "PageRank did not converge";
            break;
        }

        // Select the cell with the highest page rank
        // sort the "x" QHash by value
        QList<double> vals = x.values();
        QList<QPoint> sortedCells; //to fill with the cells
        qSort(vals);
        for(double val : vals) {
            QList<QPoint> keys = x.keys(val);
            for(QPoint cell : keys) {
                sortedCells.append(cell);
            }
        }
        QPoint cell = sortedCells.last(); // cell with highest score
        candidatesToAllocate.remove(cell); // remove the cell from the set of cells to allocate

        // remove cells around the selected cell
        QSet<QPoint> cellsToDelete;
        geomWithin(cellsToDelete,candidatesToAllocate,cell,maxDist,maxTravelTime,op,ts);
        candidatesToAllocate.subtract(cellsToDelete); // remove the cells in the vicinity of the selected cell

        qDebug() << "\tcell" << cell << cellsToDelete.size() << "score" << x[cell];

        // add the cell to the allocated cells
        QHash<QPoint, double> demandsAllocated;
        Allocation* alloc = new Allocation(cell, x[cell], demandsAllocated, cellsToDelete, _geometriesize);
        allocation.insert(cell, alloc);
    }
}
*/

/*
 * Compute the PageRank allocation
 */
/*
void ComputeAllocation::computePageRank()
{
    qDebug() << "Compute page rank";

    AllocationDialog diag(_parent);
    int ret = diag.exec();
    if (ret == QDialog::Rejected) {
        return;
    }

    // get the parameters
    double maxTravelTime = -1.0, maxDist = -1.0;

    long long deadline = diag.getDeadline();
    int nbStorageNodes = diag.getNbStorageNodes();
    double deletionFactor = diag.getDeletionFactor();
    WithinOperator op = diag.getWithinOperator();
    TravelTimeStat ts = diag.getTravelTimeStat();

    if(diag.isFixedTravelTime())
        maxTravelTime = diag.getTravelTime();
    else
        maxTravelTime = deletionFactor * deadline;

    if(diag.isFixedDistance())
        maxDist = diag.getDistance();
    else
        maxDist = deletionFactor * _traceLayer->getAverageSpeed() * deadline; // use the average speed of the mobile users

    qDebug() << deadline << nbStorageNodes << _traceLayer->getAverageSpeed() << maxDist << maxTravelTime;

    QHash<QPoint, Allocation*> allocation; // resulting allocation of the cells


    // print the allocation result
    for(auto it = allocation.begin(); it != allocation.end(); ++it) {
        qDebug() << "candidate" << it.key() << "allocated" << it.value()->weight << "with" << it.value()->demands.size() << "demands" << "deleted" << it.value()->deletedCandidates.size();
    }

    // create a new layer
    QString layerName = "PageRank";
    WeightedAllocationLayer* layer = new WeightedAllocationLayer(_parent, layerName, allocation, _geometriesize);
    _allocationLayers.append(layer);
    _parent->createLayer(layerName, layer);
}


void ComputeAllocation::computeCentrality()
{
    qDebug() << "Compute centrality betweeness";
}

void ComputeAllocation::computePercolationCentrality()
{
    qDebug() << "Compute percolation centrality";
}
*/


/*
 * Compute the k-means allocation
 */
/*
void ComputeAllocation::computeKMeans()

{
    qDebug() << "Compute K means";

    AllocationDialog diag(_parent, true, false, false);
    int ret = diag.exec();
    if (ret == QDialog::Rejected) {
        return;
    }

    // get the parameters
    double maxTravelTime = -1.0, maxDist = -1.0;

    int nbCentroids = diag.getNbStorageNodes();

    double xmax = 0, ymax = 0, xmin = 1e20, ymin = 1e20;
    int maxIterations = 100;
    double tolerance = 1e-6;

    QHash<QPointF, Allocation*> allocation; // resulting allocation of the centroids

    // populate the points
    QList<QPointF> points, x, xlast;
    auto nodes = _traceLayer->getNodes();
    for(auto it = nodes.begin(); it != nodes.end(); ++it) {
        for(auto jt = it.value()->begin(); jt != it.value()->end(); ++jt) {
            QPointF p = jt.value();
            points.append(p);
            if(p.x() > xmax) xmax = p.x();
            if(p.x() < xmin) xmin = p.x();
            if(p.y() > ymax) ymax = p.y();
            if(p.y() < ymin) ymin = p.y();
        }
    }

    int nbPoints = points.size();

    // initialize the features randomly
    for(int i = 0; i < nbCentroids; ++i) {
        double p_x = xmin + static_cast <double> (rand()) /( static_cast <double> (RAND_MAX/(xmax-xmin)));
        double p_y = ymin + static_cast <double> (rand()) /( static_cast <double> (RAND_MAX/(ymax-ymin)));
        x.append(QPointF(p_x, p_y));
    }

    bool finished = false;
    QList<QList<int>> c; // list of the points assigned per centroid
    for(int i = 0; i < maxIterations; ++i) {
        xlast = x;
        xlast.detach(); // deep copy

        // cluster assignment step
        for(int j = 0; j < nbPoints; ++j) {
            double minDist =  1e10;
            int idx = -1;
            for(int k = 0; k < nbCentroids; ++k) {
                double d = dist(points[j],x[k]);
                if(d < minDist) {
                    minDist = d;
                    idx = k;
                }
            }
            // add point j to centroid k
            c[idx].append(j);
        }

        // move centroids step
        for(int k = 0; k < nbCentroids; ++k) {
            QPointF avg = QPointF(0.0,0.0);
            int count = c[k].size();
            for(int j = 0; j < count; ++j) {
                int idx = c[k][j];
                avg +=  points[idx];
            }
            x[k] = avg / count;
        }

        // check error
        double xerr = 0.0, yerr = 0.0;
        for(int k = 0; k < nbCentroids; ++k) {
            xerr += qAbs(x[k].x() - xlast[k].x());
            yerr += qAbs(x[k].y() - xlast[k].y());
        }

        if(xerr < nbCentroids*tolerance && yerr < nbCentroids*tolerance) {
            finished = true;
            break;
        }
    }

    if(!finished) {
        qDebug() << "Did not converge";
        return;
    }

    for(int k = 0; k < nbCentroids; ++k) {
        QHash<QPointF, double> pointsAttached;
        for(int j = 0; j < c[k].size(); ++j) {
            int idx = c[k][j];
            pointsAttached.insert(points[idx], 1.0);
        }
        QSet<QPointF> deletedCandidates;
        Allocation* alloc = new Allocation(x[k], c[k].size(), pointsAttached, deletedCandidates);
        allocation.insert(x[k], alloc);
    }

    // create a new layer
    QString layerName = "kMeans";
    WeightedAllocationLayer* layer = new WeightedAllocationLayer(_parent, layerName, allocation);
    _allocationLayers.append(layer);
    _parent->createLayer(layerName, layer);

}
*/
