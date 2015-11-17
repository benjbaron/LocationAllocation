#include "allocationlayer.h"

#include <QDebug>
#include <tuple>
#include <qmath.h>
#include <QFile>
#include <QLinkedList>

#include "tracelayer.h"

AllocationLayer::AllocationLayer(MainWindow *parent, QString name, Layer *candidate, Layer *demand, int deadline, long long startTime, long long endTime, int nbFacilities, int cellSize):
    Layer(parent,name), _candidateLayer(candidate), _demandLayer(demand),
    _deadline(deadline), _startTime(startTime), _endTime(endTime), _nbFacilities(nbFacilities), _cellSize(cellSize)
{
    //        if(_candidateLayer) {
    //            // get the candidate facilities
    //            _candidates = _candidateLayer->getPoints(_deadline, _startTime, _endTime);
    //        } else {
    //            // create grid
    //            createGrid();
    //        }

    //        // get the demand points
    //        _demands = _demandLayer->getPoints(_deadline, _startTime, _endTime);

}

Layer* AllocationLayer::computeAllocation()
{
    qDebug() << "compute allocation" << toString();
    // using Euclidean distance

    //    qDebug() << "cleaning the candidate data";
    //    qDebug() << "candidate facililties" << _candidates.size();

    //    // eliminate the candidate locations that do not intersect with any demand
    //    OGRGeometryCollection* demandsGeom = _demandLayer->getGeometry(_startTime, _endTime);
    //    QList<OGRPoint> demandsGeoms;
    //    for(int l = 0; l < _demands.size(); ++l) {
    //        auto demand = _demands.at(l);
    //        QPointF demandPoint = std::get<0>(demand);
    //        OGRPoint pt(demandPoint.x(), demandPoint.y());
    //        demandsGeoms.append(pt);
    //    }
    //    // examine the candidate locations
    //    //!\\ TODO Optimize !!
    //    auto it = _candidates.begin();
//    while(it != _candidates.end()) {
//        QPointF candidatePoint = std::get<0>(*it);
//        OGRPoint pt(candidatePoint.x(), candidatePoint.y());

//        OGRGeometry* range = pt.Buffer(1);
//        int i = 0;
//        for(; i < demandsGeoms.size(); ++i) {
//            OGRPoint pt1 = demandsGeoms.at(i);
////            qDebug() << pt.getX() << "," << pt.getY() << " ; " << pt1.getX() << "," << pt1.getY() << range->Contains(&pt1);
//            if(range->Contains(&pt1)) {
//                qDebug() << "hello";
//                break;
//            }
//        }
//        qDebug() << i << demandsGeoms.size();
//        if(i == demandsGeoms.size()) {
//            it = _candidates.erase(it);
//        } else {
//            ++it;
//        }
//    }
//    qDebug() << "candidate facililties" << _candidates.size();

    // initialize the sets of demands to cover and candidates to allocate
    QSet<int> demandsToCover;
    for(int l = 0; l < _demands.size(); ++l) {
        demandsToCover.insert(l);
    }
    QSet<int> candidatesToAllocate;
    for(int k = 0; k < _candidates.size(); ++k) {
        candidatesToAllocate.insert(k);
    }

    // iterate for each facility
    for(int i = 0; i < _nbFacilities; ++i) {
        qDebug() << "allocating demands for facility" << i;
        double maxCoverage = 0.0;
        int maxCoverageIdx = -1;
        QSet<int> maxDemandsCovered;
        // find the candidate that covers the most demands
        foreach(int k, candidatesToAllocate) {
            qDebug() << "\tcandidate" << k;
            auto candidate = _candidates.at(k);
            QPointF candidatePoint    = std::get<0>(candidate);
            double  candidateWeight   = std::get<1>(candidate);
            double  candidateDistance = std::get<2>(candidate);
            QSet<int> demandsCovered;
            double coverage = 0.0;
            // Compute the covering score for the candidate
            foreach(int l, demandsToCover) {
                auto demand = _demands.at(l);
                QPointF demandPoint    = std::get<0>(demand);
                double  demandWeight   = std::get<1>(demand);
                double  demandDistance = std::get<2>(demand);
                // The candidate must be located within a given distance of the candidate
                if(getDistance(candidatePoint,demandPoint) <= demandDistance) {
                    coverage += demandWeight;
                    demandsCovered.insert(l);
                }
            }
            if(coverage > maxCoverage) {
                maxCoverage = coverage;
                maxCoverageIdx = k;
                maxDemandsCovered = QSet<int>(demandsCovered);
            }
        }
        qDebug() << "candidate" << maxCoverageIdx << "allocated" << maxCoverage << "with" << maxDemandsCovered.size() << "demands";

        // reduce the set of the population to cover
        demandsToCover.subtract(maxDemandsCovered);
        _allocation.insert(maxCoverageIdx,qMakePair(maxCoverage, QSet<int>(maxDemandsCovered)));
        candidatesToAllocate.remove(maxCoverageIdx);
    }

    // print the result allocation
    for(auto it = _allocation.begin(); it != _allocation.end(); ++it) {
        qDebug() << "candidate" << it.key() << "allocated" << it.value().first << "with" << it.value().second.size() << "demands";
    }
}

Layer *AllocationLayer::computeAllocationGridTrace()
{
    qDebug() << "compute allocation grid trace" << toString();

    _allocation.clear();

    if(_distances.isEmpty() || _nodesDemands.isEmpty() || _gridNodeCandidates.isEmpty()) {
        createGridTrace();
        computeDistanceMatrix2();
    }

    QSet<int> demandsToCover;
    for(int l = 0; l < _nodesDemands.size(); ++l) {
        demandsToCover.insert(l);
    }
    QSet<int> candidatesToAllocate;
    for(int k = 0; k < _gridNodeCandidates.size(); ++k) {
        candidatesToAllocate.insert(k);
    }

    // iterate fro each facility
    for(int i = 0; i < _nbFacilities; ++i) {
        qDebug() << "allocating demands for facility" << i;
        double maxCoverage = 0.0;
        int maxCoverageIdx = -1;
        QSet<int> maxDemandsCovered;

        // get the time it takes to reach the candidate facility
        foreach(int k, candidatesToAllocate) {
            qDebug() << "\tcandidate" << k;
            QPoint candidate = _gridNodeCandidates.at(k);
            if(!_distances.contains(candidate))
                continue;
//            auto nodes = _gridNodes[candidate];

            QSet<int> demandsCovered;
            double coverage = 0.0;
            // Compute the covering score for the candidate
            foreach(int l, demandsToCover) {
                // using the distance matrix
                if(_distances[candidate].contains(l)) {
                    long long distance = _distances[candidate][l];
                    if(distance <= _deadline) {
                        coverage += 1.0;
                        demandsCovered.insert(l);
                    }
                }

                /*
                auto demand = _nodesDemands.at(l);
                QString nodeId = demand.first;
                long long timestamp = demand.second;



                // compute the next timestamp of the node in the candidate location
                if(nodes.contains(nodeId)) {
                    auto up = nodes[nodeId].lowerBound(timestamp);
                    if(up != nodes[nodeId].end()) {
                        long long timeDiff = up.key() - timestamp;
                        if(timeDiff <= _deadline) {
                            coverage += 1.0;
                            demandsCovered.insert(l);
                        }
                    }
                }
                */
            }
            if(coverage > maxCoverage) {
                maxCoverage = coverage;
                maxCoverageIdx = k;
                maxDemandsCovered = QSet<int>(demandsCovered);
            }
        }
        qDebug() << "candidate" << maxCoverageIdx << "allocated" << maxCoverage << "with" << maxDemandsCovered.size() << "demands" << demandsToCover.size() << "demands to cover";

        // reduce the set of the population to cover
        if(maxCoverageIdx != -1) {
            demandsToCover.subtract(maxDemandsCovered);
            _allocation.insert(maxCoverageIdx,qMakePair(maxCoverage, QSet<int>(maxDemandsCovered)));
            candidatesToAllocate.remove(maxCoverageIdx);
        }

    }
    // print the result allocation
    for(auto it = _allocation.begin(); it != _allocation.end(); ++it) {
        qDebug() << "candidate" << it.key() << "allocated" << it.value().first << "with" << it.value().second.size() << "demands";
    }
}

QGraphicsItemGroup* AllocationLayer::draw()
{
    _groupItem = new QGraphicsItemGroup();
    // TODO draw the lines between the demands and the candidate locations

    // display the auto generated grid
    for(QPoint cell : _gridNodeCandidates) {
        QRectF gridCell(cell.x()*_cellSize, cell.y()*_cellSize, _cellSize, _cellSize);
        QGraphicsRectItem* item = new QGraphicsRectItem(gridCell);
        item->setBrush(QBrush(QColor(255,0,0)));
        item->setOpacity(0.6);
        item->setPen(QPen(Qt::black));

        addGraphicsItem(item);
    }

    // display the chosen allocated candidate points
    for(auto it = _allocation.begin(); it != _allocation.end(); ++it) {
        auto facility = _candidates.at(it.key());
        QPointF point = std::get<0>(facility);
        QGraphicsEllipseItem* item = new QGraphicsEllipseItem(point.x() - 500, point.y() - 500, 1000, 1000);
        item->setBrush(QBrush(QColor(0,0,255)));

        addGraphicsItem(item);
    }

    return _groupItem;
}

QList<std::tuple<QPointF, double, double> > AllocationLayer::getPoints(int deadline, long long startTime, long long endTime)
{
    Q_UNUSED(deadline)
    Q_UNUSED(startTime)
    Q_UNUSED(endTime)

    return QList<std::tuple<QPointF, double, double>>();
}

void AllocationLayer::exportFacilities(QString output)
{
    qDebug() << "[START] Writting in file" << output;
    QFile file(output);
    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "Unable to write in file "<< output;
        return;
    }

    QTextStream out(&file);
    for(auto it = _allocation.begin(); it != _allocation.end(); ++it) {
        auto facility = _candidates.at(it.key());
        QPointF pos = std::get<0>(facility);
        int facilityId = it.key();
        out << facilityId << " "
            << QString::number(pos.x(), 'f', 0) << " "
            << QString::number(pos.y(), 'f', 0) << "\n";
    }

    file.close();
}

double AllocationLayer::getDistance(QPointF p1, QPointF p2)
{
    return QLineF(p1, p2).length();
}

void AllocationLayer::createGrid()
{
    _candidates.clear();

    // creates a grid of size "_cellSize" over the demand set
    if(!_demands.isEmpty())
        _demands = _demandLayer->getPoints(_deadline, _startTime, _endTime);
    for(int l = 0; l < _demands.size(); ++l) {
        auto demand = _demands.at(l);
        QPointF demandPoint    = std::get<0>(demand);
        double  demandWeight   = std::get<1>(demand);
        double  demandDistance = std::get<2>(demand);
        // compute the cell where the demand point is
        QPoint cell((int)qFloor(demandPoint.x() / _cellSize), (int)qFloor(demandPoint.y() / _cellSize));
        if(!_grid.contains(cell)) {
            _grid.insert(cell, std::make_tuple(0.0, 0.0, QSet<int>()));
        }
        std::get<0>(_grid[cell]) += demandWeight;
        std::get<1>(_grid[cell]) += demandDistance; // Alternatively: max distance
        std::get<2>(_grid[cell]).insert(l);
    }

    // get the averages for all created cells
    for(auto it = _grid.begin(); it != _grid.end(); ++it) {
        QPoint cell = it.key();
        int nbDemands = std::get<2>(it.value()).size();
        std::get<0>(_grid[cell]) /= nbDemands;
        std::get<1>(_grid[cell]) /= nbDemands;

        QPointF cellCenter(QRectF(cell.x()*_cellSize, cell.y()*_cellSize, _cellSize, _cellSize).center());
        _candidates.append(std::make_tuple(cellCenter, std::get<0>(_grid[cell]), std::get<1>(_grid[cell])));
    }

    qDebug() << "created grid with" << _grid.size() << "cells of size" << _cellSize;
}

void AllocationLayer::createGridTrace()
{
    _gridNodes.clear();
    _nodesDemands.clear();

    // check endTime value
    if(_endTime == 0) {
        // set to max
        _endTime = std::numeric_limits<qlonglong>::max();
    }

    QHash<QString, QMap<long long, QPointF>*> nodes = ((TraceLayer*) _demandLayer)->getNodes();
    for(auto it = nodes.begin(); it != nodes.end(); ++it) {
        QString nodeId = it.key();
        auto jt_start = it.value()->lowerBound(_startTime);
        for(auto jt = jt_start; jt != it.value()->end() && jt.key() <= _endTime; ++jt) {
            QPointF point = jt.value();
            long long timestamp = jt.key();
            // compute the cell where the demand point is
            QPoint cell((int)qFloor(point.x() / _cellSize), (int)qFloor(point.y() / _cellSize));
            if(!_gridNodes.contains(cell)) {
                _gridNodes.insert(cell,QHash<QString, QMap<long long, QPointF>>());
                _gridNodeCandidates.append(cell);
            }
            if(!_gridNodes[cell].contains(nodeId)) {
                _gridNodes[cell].insert(nodeId, QMap<long long, QPointF>());
            }
            _gridNodes[cell][nodeId].insert(timestamp, point);

            QPointF cellCenter(QRectF(cell.x()*_cellSize, cell.y()*_cellSize, _cellSize, _cellSize).center());
            _candidates.append(std::make_tuple(cellCenter, 1.0, -1.0));
            _nodesDemands.append(qMakePair(nodeId, timestamp));
        }
    }

    qDebug() << "created grid nodes with" << _gridNodes.size() << "cells of size" << _cellSize;
}

void AllocationLayer::computeDistanceMatrix()
{
    _distances.clear();

    qDebug() << "[START] compute distance matrix";
    // check endTime value
    if(_endTime == 0) {
        // set to max
        _endTime = std::numeric_limits<qlonglong>::max();
    }

    QHash<QString, QMap<long long, QPointF>*> nodes = ((TraceLayer*) _demandLayer)->getNodes();
    for(auto it = nodes.begin(); it != nodes.end(); ++it) {
        QString nodeId = it.key();
        auto jt_start = it.value()->lowerBound(_startTime);
        QLinkedList<QPair<long long, int>> times;
        qDebug() << "examining node" << nodeId;
        for(auto jt = jt_start; jt != it.value()->end() && jt.key() <= _endTime; ++jt) {
            QPointF point = jt.value();
            QPoint candidateCell((int)qFloor(point.x() / _cellSize), (int)qFloor(point.y() / _cellSize));
            QPointF cellCenter(QRectF(candidateCell.x()*_cellSize, candidateCell.y()*_cellSize, _cellSize, _cellSize).center());
            _candidates.append(std::make_tuple(cellCenter, 1.0, -1.0));

            long long timestamp = jt.key();

            int demandIdx = _nodesDemands.size();
            _nodesDemands.append(qMakePair(nodeId, timestamp));

            // delete all recorded timestamps that are over the deadline
            while(!times.isEmpty() && (timestamp - times.front().first) < _deadline){
                times.pop_front();
            }
            // compute the distances between the old recorded timestamps and the current one
            for(auto oldDemand: times) {
                long long oldTimestamp = oldDemand.first;
                long long timeDiff = timestamp - oldTimestamp;
                // insert the distance into the matrix
                int oldDemandIdx = oldDemand.second;
                if(!_distances.contains(candidateCell)) {
                    _distances.insert(candidateCell,QHash<int, long long>());
                    _gridNodeCandidates.append(candidateCell);
                }
                _distances[candidateCell][oldDemandIdx] = timeDiff;
            }
            // append the recorded timestamp to the back
            times.push_back(qMakePair(timestamp, demandIdx));
        }
    }
    qDebug() << "[DONE] distance matrix computed";
}

void AllocationLayer::computeDistanceMatrix2()
{
    _distances.clear();

    qDebug() << "[START] compute distance matrix - 2";
    // check endTime value
    if(_endTime == 0) {
        // set to max
        _endTime = std::numeric_limits<qlonglong>::max();
    }

    for(QPoint candidate: _gridNodeCandidates) {
        auto nodes = _gridNodes[candidate];

        for(int i = 0; i < _nodesDemands.size(); ++i) {
            auto demand = _nodesDemands.at(i);
            QString nodeId = demand.first;
            long long timestamp = demand.second;
            // compute the next timestamp of the node in the candidate location
            if(nodes.contains(nodeId)) {
                auto up = nodes[nodeId].lowerBound(timestamp);
                if(up != nodes[nodeId].end()) {
                    long long timeDiff = up.key() - timestamp;
                    if(!_distances.contains(candidate)) {
                        _distances.insert(candidate,QHash<int, long long>());
                    }
                    _distances[candidate][i] = timeDiff;
                }
            }
        }

    }

    qDebug() << "[DONE] distance matrix - 2 - computed";
}



inline uint qHash(const QPoint &key)
{
    return qHash(key.x()) ^ qHash(key.y());
}
