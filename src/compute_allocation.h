#ifndef COMPUTEALLOCATION_H
#define COMPUTEALLOCATION_H

#include "utils.h"

// forward declarations
class SpatialStats;
class AllocationDialog;
class Loader;

// structure for the allocation parameters
struct AllocationParams {
    AllocationParams(long long deadline, int nbFacilities, double delFactor, TravelTimeStat ttStat,
                     DistanceStat dStat, double travelTime, double distance, const QString& method):
            deadline(deadline), nbFacilities(nbFacilities), delFactor(delFactor), ttStat(ttStat),
            dStat(dStat), travelTime(travelTime), distance(distance), method(method) { }
    AllocationParams() { }

    long long      deadline;
    int            nbFacilities;
    double         delFactor;
    TravelTimeStat ttStat;
    DistanceStat   dStat;
    double         travelTime;
    double         distance;
    QString        method;
    QString        computeAllStorageNodes;
};

class ComputeAllocation {
public:
    ComputeAllocation(SpatialStats* spatialStats):
            _spatialStats(spatialStats) { }

    bool processAllocationMethod(Loader* loader, AllocationParams* params, QHash<Geometry*, Allocation *>* allocation);
    void runLocationAllocation(Loader* loader, AllocationParams* params, QHash<Geometry*, Allocation*>* allocation);
    bool runRandomAllocation(Loader* loader, int nbFacilities, QHash<Geometry*, Allocation *>* allocation);
//    void runPageRank(Loader* loader, int nbFacility, TravelTimeStat ttStat, double travelTime, DistanceStat dStat, double distance, QHash<Geometry*, Allocation*>& allocation);
//    void runKMeans(Loader* loader, int nbFacilities, QHash<Geometry*, Allocation*>& allocation);

private:
    SpatialStats* _spatialStats;

    /* Returns the cells from "cells" that are within distance "distance" and/or travel time "travelTime" of "cell" in "cellsWithinDistance", depending on "op" */
    void geomWithin(QSet<Geometry*>* cellsWithin, const QSet<Geometry*>& cells, Geometry* cell, double distance = -1.0, double travelTime = -1.0, DistanceStat ds = Auto, TravelTimeStat ts = Med);

    // private methods for the location allocation computation
    double computeBackendWeight(Geometry* c, Geometry* k);
    double computeCoverageWeight(Geometry* l, Geometry* k, double deadline, QHash<Geometry*, double>* demandsCovered);
    void updateTopCandidates(QList<Allocation>* c, Geometry* k,double coverage, double backendWeight, double incomingWeight,
                             QHash<Geometry*, double> const &demandsCovered, QHash<Geometry *, double> const &backendCovered);

    /* Compute the page rank for the given set of cells */
//    bool pageRank(QHash<Geometry*, double>& x, QSet<Geometry*> cells, double alpha = 0.85, int maxIterations = 100, double tolerance = 1.0e-6);
};

#endif // COMPUTEALLOCATION_H
