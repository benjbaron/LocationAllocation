#ifndef COMPUTEALLOCATION_H
#define COMPUTEALLOCATION_H

#include "weightedallocationlayer.h"

class SpatialStats;
class Geometry;

class ComputeAllocation: public QObject
{
    Q_OBJECT
public:
    ComputeAllocation(SpatialStats* spatialStats);
    void processAllocationMethod(QString method, int nbFacilities, double deadline, double delFactor, TravelTimeStat ttStat, double travelTime, DistanceStat dStat, double distance, QHash<Geometry*, Allocation *> &allocation);
    void runLocationAllocation(int nbFacilities, double deadline, TravelTimeStat ttStat, double travelTime, DistanceStat dStat, double distance, QHash<Geometry*, Allocation*>& allocation);
//    void runPageRank(int nbFacility, TravelTimeStat ttStat, double travelTime, DistanceStat dStat, double distance, QHash<Geometry*, Allocation*>& allocation);
//    void runKMeans(int nbFacilities, QHash<Geometry*, Allocation*>& allocation);

public slots:
    void computeAllocation();

private:
    /* Compute the page rank for the given set of cells */
//    bool pageRank(QHash<Geometry*, double>& x, QSet<Geometry*> cells, double alpha = 0.85, int maxIterations = 100, double tolerance = 1.0e-6);
    /* Returns the cells from "cells" that are within distance "distance" and/or travel time "travelTime" of "cell" in "cellsWithinDistance", depending on "op" */
    void geomWithin(QSet<Geometry*> &cellsWithin, QSet<Geometry*> cells, Geometry* cell, double distance = -1.0, double travelTime = -1.0, DistanceStat ds = Auto, TravelTimeStat ts = Med);

    QList<WeightedAllocationLayer*> _allocationLayers;
    SpatialStats* _spatialStats;
};

#endif // COMPUTEALLOCATION_H
