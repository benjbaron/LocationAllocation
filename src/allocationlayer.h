#ifndef ALLOCATIONLAYER_H
#define ALLOCATIONLAYER_H

#include "layer.h"

class AllocationLayer: public Layer
{
public:
    AllocationLayer(MainWindow* parent = 0, QString name = 0, Layer* candidate = 0, Layer* demand = 0, int deadline = 0, long long startTime = 0, long long endTime = 0, int nbFacilities = 0, int cellSize = 0);

    Layer* computeAllocation();
    Layer *computeAllocationGridTrace();

    QString toString() {
        return "Allocation with candidate "
                + (_candidateLayer ? _candidateLayer->getName() : "auto grid")
                + " demand " + _demandLayer->getName()
                + " deadline " + QString::number(_deadline)
                + " start time " + QString::number(_startTime)
                + " end time " + QString::number(_endTime);
    }

    QGraphicsItemGroup *draw();
    QList<std::tuple<QPointF,double,double>> getPoints(int deadline = 0, long long startTime = 0, long long endTime = 0);
    void exportFacilities(QString output);

private:
    double getDistance(QPointF p1, QPointF p2);
    void createGrid();
    void createGridTrace();
    void computeDistanceMatrix();
    void computeDistanceMatrix2();

    Layer* _candidateLayer;
    Layer* _demandLayer;
    QList<std::tuple<QPointF,double,double>> _demands;
    QList<std::tuple<QPointF,double,double>> _candidates;
    int _deadline, _nbFacilities, _cellSize;
    long long _startTime, _endTime;

    // candidate index, <weight allocated, set of demands allocated>
    QHash<int,QPair<double,QSet<int>>> _allocation;

    // cell id, <weight, distance, set of demands indexes>
    QHash<QPoint, std::tuple<double,double,QSet<int>>> _grid;

    // cell id, <node id, <timestamp>>
    QHash<QPoint, QHash<QString, QMap<long long,QPointF>>> _gridNodes;
    QList<QPoint> _gridNodeCandidates;

    // <node id, timestamp>
    QList<QPair<QString, long long>> _nodesDemands;

    // distance matrix between candidates and demands (-1 if none)
    // <cell id (candidate), <demand id, distance> >
    QHash<QPoint, QHash<int, long long>> _distances;
};

#endif // ALLOCATIONLAYER_H
