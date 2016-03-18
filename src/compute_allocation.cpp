#include "compute_allocation.h"

#include "spatial_stats.h"

bool ComputeAllocation::processAllocationMethod(Loader* loader,
                                                AllocationParams* params,
                                                QHash<Geometry*, Allocation*>* allocation) {

    // get the allocation parameters from the AllocationParams srtucture
    long long      deadline     = params->deadline;
    int            nbFacilities = params->nbFacilities;
    double         delFactor    = params->delFactor;   // deletion factor
    TravelTimeStat ttStat       = params->ttStat;
    DistanceStat   dStat        = params->dStat;
    double         travelTime   = params->travelTime;
    double         distance     = params->distance;
    QString        method       = params->method;

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
            /* Location allocation */
            runLocationAllocation(loader, params, allocation);

        } else if(method == PAGE_RANK_MEHTOD_NAME) {
            /* Page Rank */
            //            runPageRank(loader, nbFacilities,ttStat,travelTime,dStat,distance,allocation);
            return false;
        }
    } else if(method == K_MEANS_MEHTOD_NAME) {
        /* k-means */
        //        runKMeans(loader, nbFacilities,allocation);
        return false;
    } else if(method == RANDOM_METHOD_NAME) {
        /* Random */
        runRandomAllocation(loader, nbFacilities, allocation);
    }
    return true;
}

void ComputeAllocation::geomWithin(QSet<Geometry*>* geomWithin,
                                   QSet<Geometry*> geoms,
                                   Geometry* geom,
                                   double distance,
                                   double travelTime,
                                   DistanceStat ds,
                                   TravelTimeStat ts) {

    if(ds == NoneD && ts == NoneTT)
        return;

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
            GeometryMatrixValue* val;
            _spatialStats->getValue(&val, g,geom);
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


void ComputeAllocation::runLocationAllocation(Loader* loader,
                                              AllocationParams* params,
                                              QHash<Geometry*, Allocation*>* allocation) {

    loader->loadProgressChanged((qreal) 0.0, "Initialization");

    // demands are the geometries
    // candidates are the geometries

    // get the allocation parameters from the AllocationParams srtucture
    long long      deadline     = params->deadline;
    int            nbFacilities = params->nbFacilities;
    double         delFactor    = params->delFactor;   // deletion factor
    TravelTimeStat ttStat       = params->ttStat;
    DistanceStat   dStat        = params->dStat;
    double         travelTime   = params->travelTime;
    double         distance     = params->distance;

    qDebug() << "runLocationAllocation" << nbFacilities << deadline << ttStat << travelTime << dStat << distance;

    QSet<Geometry*> demandsToCover;
    QSet<Geometry*> allDemands;
    QSet<Geometry*> candidatesToAllocate;
    QSet<Geometry*> allCandidates;

    QSet<Geometry*> circlesGeometries;
    QSet<Geometry*> cellsGeometries;

    QHash<Geometry*, GeometryValue*> geometries;
    _spatialStats->getGeometries(&geometries);
    for(auto it = geometries.begin(); it != geometries.end(); ++it) {
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
        qreal loaderValue = (qreal) i / (qreal) nbFacilities;
        loader->loadProgressChanged(loaderValue, "Allocate for facility " + QString::number(i));

//        qDebug() << "allocating demands for storage node" << i;

        QList<Allocation> topCandidates;

        // find the candidate that covers the most demands
        foreach(Geometry* k, candidatesToAllocate) {
//            qDebug() << "\tcandidate" << k;

            QHash<Geometry*, double> demandsCovered; // <demand, weight of the demand>

            double backendWeight = 0.0;
            // compute the backend weight for the previously allocated storage nodes
            for(Geometry* c : allocation->keys()) {
                backendWeight += computeBackendWeight(c,k);
            }

            double coverage = 0.0;
            // Compute the covering score for the candidate
            for(Geometry* l : demandsToCover) {
                coverage += computeCoverageWeight(l, k, deadline, &demandsCovered);
            }

            // update the candidate list
            updateTopCandidates(&topCandidates,k,coverage,backendWeight,demandsCovered);
        }

        Allocation bestCandidate;
        // reduce the set of the population to cover
        if(!topCandidates.isEmpty()) {
            // get the best candidate (the one with the greater backend weight)
            double bestWeight = -1.0;
            for(Allocation c : topCandidates) {
                if((c.backendWeight+c.weight) > bestWeight) {
                    bestWeight = c.backendWeight+c.weight;
                    bestCandidate = c;
                }
            }

            candidatesToAllocate.remove(bestCandidate.geom); // remove the selected cell
            demandsToCover.remove(bestCandidate.geom);

            // remove the candidate cells in the vicinity of the selected cell
            QSet<Geometry*> candidatesToRemove;
            geomWithin(&candidatesToRemove, candidatesToAllocate, bestCandidate.geom,
                       distance, travelTime, dStat, ttStat);
            candidatesToAllocate.subtract(candidatesToRemove);

            qDebug() << "\tcell" << bestCandidate.geom->toString() << candidatesToRemove.size();

            demandsToCover.subtract(bestCandidate.demands.keys().toSet());

            // add the allocation
            Allocation* alloc = new Allocation(bestCandidate.geom, bestCandidate.weight, bestCandidate.backendWeight, i,
                                               bestCandidate.demands, candidatesToRemove);
            allocation->insert(bestCandidate.geom, alloc);
        }

        /* Substitution part of the algorithm */
        // -> tries to replace each facility one at a time with a facility at another "free" site
        // TODO Change this (one line below) with a loader for instance

        loader->loadProgressChanged(loaderValue, "Substitution for facility " + QString::number(i));

        for(Geometry* k : allocation->keys()) {
            // get the demands already covered by the current candidate

            Allocation* alloc = allocation->value(k);
            QSet<Geometry*> prevDemandsCovered = alloc->demands.keys().toSet();
            double prevWeight = alloc->weight;
            QSet<Geometry*> prevDeletedCandidates = alloc->deletedCandidates;

            QList<Allocation> newTopCandidates;

            for(Geometry* k1 : allCandidates - allocation->keys().toSet()) {
                if(k1 == bestCandidate.geom) continue;

                QHash<Geometry*, double> demandsCovered; // <demand, weight of the demand>
                double backendWeight = 0.0;
                // compute the backend weight for the previously allocated storage nodes
                for(Geometry* c : allocation->keys()) {
                    backendWeight += computeBackendWeight(c,k);
                }

                double coverage = 0.0;
                // all demands to cover + those covered by the current candidate
                for(Geometry* l : demandsToCover + prevDemandsCovered) {
                    coverage += computeCoverageWeight(l, k, deadline, &demandsCovered);
                }

                // update the candidate list
                updateTopCandidates(&newTopCandidates,k,coverage,backendWeight,demandsCovered);
            }

            // get the best candidate
            Allocation newBestCandidate;
            double newBestBackendWeight = 0.0;
            for(Allocation c : newTopCandidates) {
                if(c.backendWeight > newBestBackendWeight) {
                    newBestBackendWeight = c.backendWeight;
                    newBestCandidate = c;
                }
            }

            if(prevWeight < newBestCandidate.weight) {
                // change the candidate's current allocation
//                qDebug() << "changed candidate / old" << prevWeight << "new" << newBestCandidate.weight;

                alloc->weight = newBestCandidate.weight;
                alloc->backendWeight = newBestCandidate.backendWeight;
                alloc->demands = newBestCandidate.demands;
                QSet<Geometry*> candidatesToRemove;
                geomWithin(&candidatesToRemove,
                           candidatesToAllocate + prevDeletedCandidates,
                           newBestCandidate.geom,
                           distance, travelTime, dStat, ttStat);
                alloc->deletedCandidates = candidatesToRemove;
                alloc->geom = k;
                // TODO Change the new candidate's rank
            }
        }

//        qDebug() << "candidate" << ((bestCandidate.geom) ? bestCandidate.geom->toString() : "None")
//                 << "allocated" << bestCandidate.weight << "with" << bestCandidate.demands.size() << "demands"
//                 << demandsToCover.size() << "demands to cover" << bestCandidate.backendWeight << "backend weight";
    }

    loader->loadProgressChanged(1.0, "Done");
}

double ComputeAllocation::computeBackendWeight(Geometry* c, Geometry* k) {

    GeometryMatrixValue* val;
    _spatialStats->getValue(&val,k,c);
    double weight = 0.0;
    if(val) {
        double visitCount = val->visits.size();
        if(visitCount > 1) {
            weight += val->avgScore;
        }
//                    double interVisitTime = val->interVisitDurationDist.getAverage();
//                    double visitCount = val->visits.size();
//                    if(visitCount > 1) {
//                        coverage += visitCount / interVisitTime;
//                    }
    }

    _spatialStats->getValue(&val,c,k);
    if(val) {
        double visitCount = val->visits.size();
        if(visitCount > 1) {
            weight += val->avgScore;
        }
//                    double interVisitTime = val->interVisitDurationDist.getAverage();
//                    double visitCount = val->visits.size();
//                    if(visitCount > 1) {
//                        coverage += visitCount / interVisitTime;
//                    }
    }

    return weight;
}

double ComputeAllocation::computeCoverageWeight(Geometry* l, Geometry* k, double deadline, QHash<Geometry*, double>* demandsCovered) {
    double weight = 0.0;
    GeometryMatrixValue* val;
    _spatialStats->getValue(&val,l,k);
    if(val) {
        double medTravelTime = val->travelTimeDist.getMedian();
        if(medTravelTime <= deadline) {
            double visitCount = val->visits.size();
            if(visitCount > 1) {
                double score = val->avgScore;
                weight += score; // weight of the demand
                demandsCovered->insert(l, score);
            }
        }
    }

    return weight;
}

void ComputeAllocation::updateTopCandidates(QList<Allocation> *c, Geometry *k,
                                            double coverage, double backendWeight,
                                            QHash<Geometry *, double> const &demandsCovered) {

//    qDebug() << ">>  add candidate" << k->toString() << coverage << backendWeight;
//    qDebug() << "--- print the content of the top candidates (before) ---";
//    int i = 0;
//    for(Allocation a : *c) {
//        qDebug() << i++ << a.geom->toString() << a.demands.size() << a.backendWeight << a.weight;
//    }

    if(c->isEmpty() || coverage > c->last().weight) {
        // go through the top candidates from the bottom of the list
        // and insert the current one at the right index
        Allocation a(k,coverage,backendWeight,-1,QHash<Geometry*, double>(demandsCovered));

        int idx = 0;
        while(idx < c->size()-1 && coverage < c->at(idx).weight)
            idx++;

        if(!c->isEmpty() && coverage < c->at(idx).weight) c->push_back(a);
        else c->insert(idx, a);

        if(c->size() > 5) c->pop_back(); // keep the top 5 candidates
    }

//    qDebug() << "--- print the content of the top candidates (after) ---";
//    i = 0;
//    for(Allocation a : *c) {
//        qDebug() << i++ << a.geom->toString() << a.demands.size() << a.backendWeight << a.weight;
//    }
}



bool ComputeAllocation::runRandomAllocation(Loader* loader, int nbFacilities, QHash<Geometry*, Allocation*>* allocation) {
    QList<Geometry*> toAllocate;
    QHash<Geometry*, GeometryValue*> geometries;
    _spatialStats->getGeometries(&geometries);
    for(auto it = geometries.begin(); it != geometries.end(); ++it) {
        toAllocate.append(it.key());
    }

    for(int i = 0; i < nbFacilities; ++i) {
        // choose a random candidate to allocate
        int idx = qrand() % toAllocate.size();
        Geometry* geom = toAllocate.at(idx);
        toAllocate.removeAt(idx);
        allocation->insert(geom, new Allocation(geom, 1.0, 0.0, i));

        // update the loader
        loader->loadProgressChanged((qreal) i / (qreal) nbFacilities, "Allocating facility "+QString::number(i+1));
    }

    loader->loadProgressChanged(1.0, "Done");

    return true;
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

