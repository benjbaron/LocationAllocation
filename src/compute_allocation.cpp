#include "compute_allocation.h"

#include "spatial_stats.h"

bool ComputeAllocation::processAllocationMethod(Loader* loader,
                                                AllocationParams* params,
                                                QHash<Geometry*, Allocation*>* allocation) {

    // get the allocation parameters from the AllocationParams srtucture
    long long      deadline               = params->deadline;
    int            nbFacilities           = params->nbFacilities;
    double         delFactor              = params->delFactor;   // deletion factor
    TravelTimeStat ttStat                 = params->ttStat;
    DistanceStat   dStat                  = params->dStat;
    double         travelTime             = params->travelTime;
    double         distance               = params->distance;
    QString        method                 = params->method;
    QString        computeAllStorageNodes = params->computeAllStorageNodes;


    qDebug() << "Method" << method << "nbFacilities" << nbFacilities << "deadline" << deadline <<
    "delFactor" << delFactor << "travelStat" << ttStat << "travelTime" << travelTime << "dstat" <<
    dStat << "distance" << distance << "allStorageNodes" << computeAllStorageNodes;


    if(method == LOCATION_ALLOCATION_MEHTOD_NAME || method == PAGE_RANK_MEHTOD_NAME) {
        double maxTravelTime, maxDist;
        if(ttStat != NoneTTStat)
            maxTravelTime = travelTime;
        else
            maxTravelTime = delFactor * deadline;

        if(dStat == FixedDStat)
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
                                   const QSet<Geometry*>& geoms,
                                   Geometry* geom,
                                   double distance,
                                   double travelTime,
                                   DistanceStat ds,
                                   TravelTimeStat ts) {

    if(ds == NoneDStat && ts == NoneTTStat)
        return;

//    QRectF cell1(cell.x()*_geometriesize, cell.y()*+_geometriesize, _geometriesize, _geometriesize);
    foreach(Geometry* g, geoms) {
        if(g == geom)
            continue; // skip the ogrGeometry itself

        bool distance_flag = false;
        bool travelTime_flag = false;
        double dist;
        double tt;

        if(ds != NoneDStat && distance > 0.0) {
            dist = g->distance(geom);
            if(std::islessequal(dist,distance)) {
                distance_flag = true;
            }
        }
        if(ts != NoneTTStat || travelTime > 0.0) {
            GeometryMatrixValue* val;
            _spatialStats->getValue(&val, g, geom);
            if(val) {
                if(ts == AvgTTStat) {
                    tt = val->travelTimeDist.getAverage();
                }

                else if(ts == MedTTStat) {
                    tt = val->travelTimeDist.getMedian();
                }

                if(std::islessequal(tt,travelTime)) {
                    travelTime_flag = true;
                }
            }
        }

        if((ds != NoneDStat && distance_flag) || (ts != NoneTTStat && travelTime_flag)) {
//            qDebug() << "or / add" << geom << g->toString()
//                     << "distance" << ds << dist << distance
//                     << "travelTime" << ts << tt << travelTime;
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

    // get the allocation parameters from the AllocationParams structure
    long long      deadline        = params->deadline;
    int            nbFacilities    = params->nbFacilities;
    double         delFactor       = params->delFactor;   // deletion factor
    TravelTimeStat ttStat          = params->ttStat;
    DistanceStat   dStat           = params->dStat;
    double         travelTime      = params->travelTime;
    double         distance        = params->distance;
    QString        allStorageNodes = params->computeAllStorageNodes;

    bool exportAllocation = !allStorageNodes.isEmpty();
    /* if allStorageNodes is not an empty string, we export the allocation at each iteration of the allocation
     * id X Y rank weight number_deleted number_allocated */

    qDebug() << "runLocationAllocation" << nbFacilities << deadline << ttStat << travelTime << dStat << distance << allStorageNodes << exportAllocation;

    QSet<Geometry*> demandsToCover;
    QSet<Geometry*> allDemands;
    QSet<Geometry*> candidatesToAllocate;
    QSet<Geometry*> allCandidates;

    QSet<Geometry*> circlesGeometries;
    QSet<Geometry*> cellsGeometries;

    // get circle ogrGeometry types if there are some
    QHash<Geometry*, GeometryValue*> geometries;
    _spatialStats->getGeometries(&geometries);
    for(auto it = geometries.begin(); it != geometries.end(); ++it) {
        Geometry* geom = it.key();
        if(geom->getGeometryType() == CellGeometryType) cellsGeometries.insert(geom);
        else if(geom->getGeometryType() == CircleGeometryType) circlesGeometries.insert(geom);
    }

    if(circlesGeometries.size() > 0)
        allCandidates = circlesGeometries;
    else
        allCandidates = cellsGeometries;
    allDemands = cellsGeometries;

    candidatesToAllocate = allCandidates;
    demandsToCover = allDemands;
    int prevAllocated = 0;

    // iterate for each storage node to allocate
    for(int i = 0; (!exportAllocation && i < nbFacilities && !demandsToCover.isEmpty()) || (exportAllocation && !demandsToCover.isEmpty()); ++i) {
        qreal loaderValue = (qreal) i / (qreal) nbFacilities;
        loader->loadProgressChanged(loaderValue, "Allocate for facility " + QString::number(i));

//        qDebug() << "allocating demands for storage node" << i;

        QList<Allocation> topCandidates;

        // find the candidate that covers the most demands
        double maxCoverageWeights = 0.0; // to normalize the coverage weights (demand weights)
        double maxBackendWeights  = 0.0; // to normalize the backend weights
        double maxIncomingWeights = 0.0; // to normalize the incoming weights (connectivity weight)
        foreach(Geometry* k, candidatesToAllocate) {
            double incomingWeight = 0.0;
            GeometryValue* geomVal;
            _spatialStats->getValue(&geomVal, k);
            incomingWeight = geomVal->avgIncomingScore;
            if(incomingWeight > maxIncomingWeights)
                maxIncomingWeights = incomingWeight;

            double backendWeight = 0.0;
            for(Geometry* c : allocation->keys()) {
                backendWeight += computeBackendWeight(c,k);
            }
            if(backendWeight > maxBackendWeights) {
                maxBackendWeights = backendWeight;
            }

            double coverage = 0.0;
            QHash<Geometry*, double> demandsCovered; // <demand, weight of the demand>
            for(Geometry* l : demandsToCover) {
                coverage += computeCoverageWeight(l, k, deadline, &demandsCovered);
            }
            if(coverage > maxCoverageWeights) {
                maxCoverageWeights = coverage;
            }
        }

        foreach(Geometry* k, candidatesToAllocate) {
//            qDebug() << "\tcandidate" << k;
            double incomingWeight = 0.0;
            GeometryValue* geomVal;
            _spatialStats->getValue(&geomVal, k);
            incomingWeight = geomVal->avgIncomingScore;

            // compute the backend weight for the previously allocated storage nodes
            double backendWeight = 0.0;
            int nbBackendLinks = 0;
            QHash<Geometry*, double> backendCovered;
            for(Geometry* c : allocation->keys()) {
                double w = computeBackendWeight(c,k);
                backendWeight += w;
                backendCovered.insert(c, w);
                if(w > 0.0)
                    nbBackendLinks++;
//                qDebug() << "\t\t" << "candidate" << k->toString() << backendWeight << w;
            }

            // Compute the covering score for the candidate
            double coverage = 0.0;
            QHash<Geometry*, double> demandsCovered; // <demand, weight of the demand>
            for(Geometry* l : demandsToCover) {
                coverage += computeCoverageWeight(l, k, deadline, &demandsCovered);
            }

            // update the candidate list
            double normalizedBackendWeight  = maxBackendWeights > 0.0  ? backendWeight/maxBackendWeights   : 0.0;
            double normalizedCoverageWeight = maxCoverageWeights > 0.0 ? coverage/maxCoverageWeights       : 0.0;
            double normalizedIncomingWeight = maxIncomingWeights > 0.0 ? incomingWeight/maxIncomingWeights : 0.0;
//            qDebug() << "\tcandidate" << k->toString()
//                     << "| Backend:"    << normalizedBackendWeight  << backendWeight  << maxBackendWeights
//                     << "| Coverage:"   << normalizedCoverageWeight << coverage       << maxCoverageWeights
//                     << "| Incoming"    << normalizedIncomingWeight << incomingWeight << maxIncomingWeights;


            if(normalizedCoverageWeight > 0 && (i==0 || normalizedBackendWeight > 0.0)) {
                // add the candidate to the top candidates
                Allocation a(k,normalizedCoverageWeight,normalizedBackendWeight,normalizedIncomingWeight,-1,demandsCovered,backendCovered);
                topCandidates.append(a);
            }
//            updateTopCandidates(&topCandidates,k,normalizedCoverageWeight,normalizedBackendWeight,normalizedIncomingWeight,demandsCovered, backendCovered);
        }

        Allocation bestCandidate;
        // reduce the set of the population to cover
        if(!topCandidates.isEmpty()) {

            // get the best candidate (the one with the greater total weight)
            double bestWeight = -1.0;
            for(Allocation c : topCandidates) {
                double w = c.weight; // c.getWeight();
//                qDebug() << "\t" << QString("(Allocation %1)").arg(QString::number(i)) << "candidate" << w << c.weight<< c.backendWeight << c.incomingWeight << bestWeight;
                if(w > bestWeight) {
                    bestWeight = w;
                    bestCandidate = c;
                }
            }

            candidatesToAllocate.remove(bestCandidate.geom); // remove the selected cell
            demandsToCover.remove(bestCandidate.geom); // remove the ogrGeometry from the demands to cover

            // remove the candidate cells in the vicinity of the selected cell
            QSet<Geometry*> candidatesToRemove;
            geomWithin(&candidatesToRemove, candidatesToAllocate, bestCandidate.geom,
                       distance, travelTime, dStat, ttStat);
            candidatesToAllocate.subtract(candidatesToRemove);

//            qDebug() << "\tAllocation" << i << bestCandidate.geom->toString() << candidatesToRemove.size() << bestCandidate.backendWeight << bestCandidate.weight;

            demandsToCover.subtract(bestCandidate.demands.keys().toSet());

            // add the allocation
            Allocation* alloc = new Allocation(bestCandidate.geom, bestCandidate.weight, bestCandidate.backendWeight, bestCandidate.incomingWeight,
                                               i, bestCandidate.demands, bestCandidate.backends, candidatesToRemove);
            allocation->insert(bestCandidate.geom, alloc);

//            qDebug() << "best candidate" << bestCandidate.geom->toString() << bestCandidate.weight << bestCandidate.backendWeight;
        }

        /* Substitution part of the algorithm */
        // -> tries to replace each facility one at a time with a facility at another "free" site
        // TODO Change this (one line below) with a loader for instance

        loader->loadProgressChanged(loaderValue, "Substitution for facility " + QString::number(i));

        for(Geometry* k : allocation->keys()) {
            if(k == bestCandidate.geom)
                continue; // ignore the best candidate that was picked

            Allocation* alloc = allocation->value(k);

            // get the demands already covered by the current candidate
            QSet<Geometry*> prevDemandsCovered = alloc->demands.keys().toSet();

            double prevWeight = alloc->weight; // previous total weight
            QSet<Geometry*> prevDeletedCandidates = alloc->deletedCandidates;

            // find the top new best candidates that could replace the current allocated candidate
            QList<Allocation> newTopCandidates;

            // find the candidate that covers the most demands
            double newMaxCoverageWeights = 0.0; // to normalize the coverage weights (demand weights)
            double newMaxBackendWeights  = 0.0; // to normalize the backend weights
            double newMaxIncomingWeights = 0.0; // to normalize the incoming weights
            for(Geometry* k1 : allCandidates - allocation->keys().toSet()) {
                GeometryValue* geomVal;
                _spatialStats->getValue(&geomVal, k);
                double incomingWeight = geomVal->avgIncomingScore;
                if(incomingWeight > newMaxIncomingWeights)
                    newMaxIncomingWeights = incomingWeight;

                double backendWeight = 0.0;
                for(Geometry* c : allocation->keys()) {
                    if(c == k) continue;
                    double w = computeBackendWeight(c,k1);
                    backendWeight += w;
                }
                if(backendWeight > newMaxBackendWeights) {
                    newMaxBackendWeights = backendWeight;
                }

                double coverage = 0.0;
                QHash<Geometry*, double> demandsCovered; // <demand, weight of the demand>
                for(Geometry* l : demandsToCover + prevDemandsCovered) {
                    coverage += computeCoverageWeight(l, k1, deadline, &demandsCovered);
                }
                if(coverage > newMaxCoverageWeights) {
                    newMaxCoverageWeights = coverage;
                }
            }

            for(Geometry* k1 : allCandidates - allocation->keys().toSet()) {
                if(k1 == bestCandidate.geom)
                    continue;

                GeometryValue* geomVal;
                _spatialStats->getValue(&geomVal, k1);
                double incomingWeight = geomVal->avgIncomingScore;

                QHash<Geometry*, double> demandsCovered; // <demand, weight of the demand>
                QHash<Geometry*, double> backendCovered; // <facility, weight of the facility>
                double backendWeight = 0.0;
                int nbBackendLinks = 0;
                // compute the backend weight for the previously allocated storage nodes
                for(Geometry* c : allocation->keys()) {
                    if(c == k) continue;
                    double w = computeBackendWeight(c,k1);
                    backendWeight += w;
                    backendCovered.insert(c,w);
                    if(w > 0.0)
                        nbBackendLinks++;
                }

                double coverage = 0.0;
                // all demands to cover + those covered by the current candidate
                for(Geometry* l : demandsToCover + prevDemandsCovered) {
                    coverage += computeCoverageWeight(l, k1, deadline, &demandsCovered);
                }

                // update the candidate list
                double newNormalizedBackendWeight  = newMaxBackendWeights > 0.0  ? backendWeight/newMaxBackendWeights   : 0.0;
                double newNormalizedCoverageWeight = newMaxCoverageWeights > 0.0 ? coverage/newMaxCoverageWeights       : 0.0;
                double newNormalizedIncomingWeight = newMaxIncomingWeights > 0.0 ? incomingWeight/newMaxIncomingWeights : 0.0;

//                qDebug() << "\t\tcandidate" << k->toString()
//                << "| Backend:"    << newNormalizedBackendWeight  << backendWeight  << newMaxBackendWeights
//                << "| Coverage:"   << newNormalizedCoverageWeight << coverage       << newMaxCoverageWeights
//                << "| Incoming"    << newNormalizedIncomingWeight << incomingWeight << newMaxIncomingWeights;

                if(newNormalizedCoverageWeight > 0 && newNormalizedBackendWeight > 0.0) {
                    // add the candidate to the top candidates
                    Allocation a(k1,newNormalizedCoverageWeight,newNormalizedBackendWeight,newNormalizedIncomingWeight,-1,demandsCovered,backendCovered);
                    newTopCandidates.append(a);
                }

//                updateTopCandidates(&newTopCandidates,k1,
//                                    newNormalizedCoverageWeight,
//                                    newNormalizedBackendWeight,
//                                    normalizedIncomingWeight,
//                                    demandsCovered, backendCovered);
            }

            // get the best candidate
            Allocation newBestCandidate;
            double newBestWeight = 0.0;
            for(Allocation c : newTopCandidates) {
                double w = c.weight; //c.getWeight();
                if(w > newBestWeight) {
                    newBestWeight = w;
                    newBestCandidate = c;
                }
            }

            if(prevWeight < newBestWeight) {
                // change the candidate's current allocation
//                qDebug() << "changed candidate / old" << prevWeight << "new" << newBestCandidate.weight;

                // delete the current allocation
                allocation->remove(k);

//                qDebug() << "delete " << k->toString() << newBestCandidate.geom->toString() << prevWeight << newBestCandidate.weight;
                QSet<Geometry*> candidatesToRemove;
                geomWithin(&candidatesToRemove,
                           candidatesToAllocate + prevDeletedCandidates,
                           newBestCandidate.geom,
                           distance, travelTime, dStat, ttStat);

                Allocation* a = new Allocation(newBestCandidate.geom, newBestCandidate.weight, newBestCandidate.backendWeight, newBestCandidate.incomingWeight,
                                               -1, newBestCandidate.demands, newBestCandidate.backends, candidatesToRemove);
                allocation->insert(newBestCandidate.geom, a);

                // update the candidates and the demands that are deleted and allocated
                candidatesToAllocate.subtract(candidatesToRemove);
                candidatesToAllocate.unite(prevDeletedCandidates);
                candidatesToAllocate.remove(newBestCandidate.geom);
                candidatesToAllocate.insert(k);

                demandsToCover.unite(prevDemandsCovered);
                demandsToCover.subtract(newBestCandidate.demands.keys().toSet());

                // TODO Change the new candidate's rank
            }
        }

//        qDebug() << "candidate" << ((bestCandidate.geom) ? bestCandidate.geom->toString() : "None")
//                 << "allocated" << bestCandidate.weight << "with" << bestCandidate.demands.size() << "demands"
//                 << demandsToCover.size() << "demands to cover" << bestCandidate.backendWeight << "backend weight";

        if(prevAllocated == allocation->size())
            break;
        prevAllocated = allocation->size();

        if(exportAllocation) {
            // output the current allocation
            QString filename = allStorageNodes + "/alloc"+ QString::number(i+1)+"-"+QString::number(deadline)+".txt";
            qDebug() << "outputing file" << filename;
            QFile file(filename);
            if(!file.open(QFile::WriteOnly)) {
                qDebug() << "Unable to write in file "<< filename;
                return;
            }
            QTextStream out(&file);

            int count = 0;
            for(auto it = allocation->begin(); it != allocation->end(); ++it) {
                out << QString::number(count)
                    << " " << QString::number(it.key()->getCenter().x(), 'f', 2)
                    << " " << QString::number(it.key()->getCenter().y(), 'f', 2)
                    << " " << QString::number(it.value()->weight, 'f', 2)
                    << " " << QString::number(it.value()->deletedCandidates.size())
                    << " " << QString::number(it.value()->demands.size()) << "\n";

                count++;
            }
            file.close();
        }
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
    }

    _spatialStats->getValue(&val,c,k);
    if(val) {
        double visitCount = val->visits.size();
        if(visitCount > 1) {
            weight += val->avgScore;
        }
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
                                            double coverage, double backendWeight, double incomingWeight,
                                            QHash<Geometry *, double> const &demandsCovered,
                                            QHash<Geometry *, double> const &backendCovered) {

//    qDebug() << ">>  add candidate" << k->toString() << coverage << backendWeight;
//    qDebug() << "--- print the content of the top candidates (before) ---";
//    int i = 0;
//    for(Allocation a : *c) {
//        qDebug() << i++ << a.geom->toString() << a.demands.size() << a.backendWeight << a.weight;
//    }

    double weight = backendWeight;
    double lastWeight = c->isEmpty() ? -1.0 : c->last().getWeightRank();
    if(c->isEmpty() || weight > lastWeight) {
        // go through the top candidates from the top of the list
        // and insert the current one at the right index
        Allocation a(k,coverage,backendWeight,incomingWeight,-1,demandsCovered,backendCovered);

        int idx = 0;
        double currentWeight = c->isEmpty() ? -1.0 : c->at(idx).getWeightRank();
        while(idx < c->size()-1 && weight < currentWeight) {
            idx++;
            currentWeight = c->at(idx).getWeightRank();
        }

//        if(!c->isEmpty() && weight < currentWeight)
//            c->push_back(a);
//        else
            c->insert(idx, a);

        if(c->size() > 10)
            c->pop_back(); // keep the top 10 candidates
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

