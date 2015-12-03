#include "spatialstats.h"
#include<QGraphicsItemGroup>
#include <math.h>

#include "constants.h"
#include "tracelayer.h"
#include "allocationdialog.h"

SpatialStats::SpatialStats(MainWindow *parent, QString name, TraceLayer *traceLayer):
    Layer(parent, name), _traceLayer(traceLayer)
{
    _statMenu = new QMenu();
    _statMenu->setTitle("Stat analysis");
    QAction* action = _statMenu->addAction("Location allocation");
    connect(action, &QAction::triggered, this, &SpatialStats::computeLocationAllocation);
    action = _statMenu->addAction("PageRank");
    connect(action, &QAction::triggered, this, &SpatialStats::computePageRank);
    action = _statMenu->addAction("Betweeness centrality");
    connect(action, &QAction::triggered, this, &SpatialStats::computeCentrality);
    action = _statMenu->addAction("Percolation centrality");
    connect(action, &QAction::triggered, this, &SpatialStats::computePercolationCentrality);
    action = _statMenu->addAction("k-means");
    connect(action, &QAction::triggered, this, &SpatialStats::computeKMeans);

    _parent->addMenu(_statMenu);

    populateMobileNodes();
}

void SpatialStats::populateMobileNodes()
{
    // add the successive point positions of the mobile nodes
    auto nodes = _traceLayer->getNodes();

    for(auto it = nodes.begin(); it != nodes.end(); ++it) {
        QString nodeId = it.key();
        if(!_mobileNodes.contains(nodeId) && it.value()->size() > 0) {
            _mobileNodes.insert(nodeId, new MobileNode(nodeId, _cellSize, _sampling));
        }
        MobileNode* node = _mobileNodes.value(nodeId);
        long long startTime = it.value()->firstKey();
        for(auto jt = it.value()->begin(); jt != it.value()->end() && jt.key() - startTime < _duration; ++jt) {
            long long ts = jt.key();
            QPointF pos = jt.value();
            node->addPosition(ts, pos.x(), pos.y());
        }
    }
}

void SpatialStats::computeStats()
{
    // compute the visiting matrix for the current set of mobile nodes
    int nbNodes = _mobileNodes.size();
    qDebug() << "number of mobile nodes" << nbNodes;
    int nodeCounter = 0;
    for(auto it_mobileNode = _mobileNodes.begin(); it_mobileNode != _mobileNodes.end(); ++it_mobileNode) {
        MobileNode* mobileNode = it_mobileNode.value();
        auto cells = mobileNode->getCells();
        for(auto it = cells.begin(); it != cells.end(); ++it) {
            QPoint cell1 = it.value().first;
            long long start1 = it.key();
            long long end1 = it.value().second;
            
            if(!_cells.contains(cell1)) _cells.insert(cell1, new CellValue(cell1));
            CellValue* val = _cells.value(cell1);
            val->visits.insert(start1, end1);
            val->visitFrequency.append(start1);
            val->nodes.insert(it_mobileNode.key());
            
            // examine the subsequent visited cells
            QSet<QPoint> visitedCells;
            for(auto jt = it+1; jt != cells.end(); ++jt) {
                QPoint cell2 = jt.value().first;
                long long start2 = jt.key();
                long long end2 = jt.value().second;

//                qDebug() << "cell1" << cell1 << "cell2" << cell2 << visitedCells;

                if(cell1 == cell2) break; // stop when the node pass on the same stating cell
                if(visitedCells.contains(cell2)){
                    continue; // do not take account previously visited cells
                }
                
                if(!_cellMatrix.contains(cell1)) {
                    _cellMatrix.insert(cell1, new QHash<QPoint, CellMatrixValue*>());
                }
                if(!_cellMatrix.value(cell1)->contains(cell2)) {
                    _cellMatrix.value(cell1)->insert(cell2, new CellMatrixValue(cell1, cell2));
                }

                CellMatrixValue* val = _cellMatrix.value(cell1)->value(cell2);
                val->travelTimeDist.addValue(qMax((long long) 0, start2-start1));
                val->visitFrequency.append(start1);
                val->visits.insert(start1, end1);
                val->nodes.insert(it_mobileNode.key());

                visitedCells.insert(cell2);
            }
        }
//        emit loadProgressChanged((qreal) nodeCounter++ / (qreal) nbNodes);
    }

    qDebug() << "size of matrix" << _cellMatrix.size();

    qDebug() << "compute the inter-visit durations / cells";
    // compute the inter-visit durations for the cells
    for(auto it = _cells.begin(); it != _cells.end(); ++it) {
        QPoint cell = it.key();
        CellValue* val = it.value();
        auto visits = val->visits;
        long long prevStartTime = visits.firstKey();
        for(auto kt = visits.begin(); kt != visits.end(); ++kt) {
            long long start = kt.key();
            foreach(long long end, visits.values(start)) {
                val->interVisitDurationDist.addValue(start-prevStartTime);
                prevStartTime = start;
            }
        }
        val->localStat = computeLocalStat(cell);
        val->color = selectColorForLocalStat(val->localStat);
    }

    qDebug() << "compute the inter-visit durations / matrix";
    // compute the inter-visit durations for the matrix cells
    for(auto it = _cellMatrix.begin(); it != _cellMatrix.end(); ++it) {
        QPoint cell1 = it.key();
        auto cells = it.value();
        for(auto jt = cells->begin(); jt != cells->end(); ++jt) {
            QPoint cell2 = jt.key();
            CellMatrixValue* val = jt.value();
            auto visits = val->visits;
            long long prevStartTime = visits.firstKey();
            for(auto kt = visits.begin(); kt != visits.end(); ++kt) {
                long long start = kt.key();
                foreach(long long end, visits.values(start)) {
                    val->interVisitDurationDist.addValue(start-prevStartTime);
                    prevStartTime = start;
                }
            }

            // update the connections counters
            if(_cellMatrix.contains(cell2)) {
                if(_cellMatrix.value(cell2)->contains(cell1)) {
                    if(_cells.contains(cell2)) {
                        _cells.value(cell2)->connections++;
                        int travelTime = (int) qCeil(_cellMatrix.value(cell2)->value(cell1)->travelTimeDist.getAverage());
                        _cells.value(cell2)->travelTimes.addValue(travelTime);
                    }
                }
            }
        }
    }

    qDebug() << "Compute scores";
    for(auto it = _cellMatrix.begin(); it != _cellMatrix.end(); ++it) {
        QPoint cell1 = it.key();
        CellValue* cellVal = _cells.value(cell1);
        double nodeAvg = cellVal->interVisitDurationDist.getAverage() > 0.0 ? cellVal->interVisitDurationDist.getAverage() : 1.0;
        double nodeMed = cellVal->interVisitDurationDist.getMedian() > 0.0 ? cellVal->interVisitDurationDist.getMedian() : 1.0;
        double nodeCount = (double) cellVal->visits.size();
        double nodeMedScore = nodeCount / nodeMed;
        double nodeAvgScore = nodeCount / nodeAvg;
        if(cellVal) cellVal->medScore = nodeMedScore;
        if(cellVal) cellVal->avgScore = nodeAvgScore;

        auto cells = it.value();
        for(auto jt = cells->begin(); jt != cells->end(); ++jt) {
            QPoint cell2 = jt.key();
            CellMatrixValue* edgeVal = jt.value();
            double edgeAvg = edgeVal->interVisitDurationDist.getAverage() > 0.0 ? edgeVal->interVisitDurationDist.getAverage() : 1.0;
            double edgeMed = edgeVal->interVisitDurationDist.getMedian() > 0.0 ? edgeVal->interVisitDurationDist.getMedian() : 1.0;
            double edgeCount = (double) edgeVal->visits.size();
            double edgeMedScore = edgeCount / edgeMed;
            double edgeAvgScore = edgeCount / edgeAvg;

            if(_cells.contains(cell2)) _cells.value(cell2)->medIncomingScore += edgeMedScore;
            if(_cells.contains(cell2)) _cells.value(cell2)->avgIncomingScore += edgeAvgScore;
            edgeVal->medScore = edgeMedScore;
            edgeVal->avgScore = edgeAvgScore;
        }
    }


    qDebug() << "[DONE] Compute stat";
}

QColor SpatialStats::selectColorForLocalStat(qreal zScore)
{
    if(zScore >= 3.291) return QColor("#720206");
    else if(zScore >= 2.576) return QColor("#f33f1c");
    else if(zScore >= 1.960) return QColor("#f37b22");
    else if(zScore >= 1.645) return QColor("#fffe38");
    else return QColor("#cccccc");
}

qreal SpatialStats::computeLocalStat(QPoint cell_i)
{
    qreal sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0, sum5 = 0.0;
    int n = _cells.size();
    for(auto it = _cells.begin(); it != _cells.end(); ++it) {
        int count_j = it.value()->visits.size();
        QPoint cell_j = it.key();
        int weight = 1.0 / (0.01 + (qreal) qSqrt(qPow(cell_i.x() - cell_j.x(), 2) + qPow(cell_i.y() - cell_j.y(), 2)));
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

QGraphicsItemGroup* SpatialStats::draw()
{
    _groupItem = new QGraphicsItemGroup();
    _groupItem->setHandlesChildEvents(false);

    for(auto it = _cellMatrix.begin(); it != _cellMatrix.end(); ++it) {
        GraphicsCell* item = new GraphicsCell(it.key().x()*_cellSize, it.key().y()*_cellSize, _cellSize, _cellSize, it.key());
        item->setBrush(QBrush(_cells[it.key()]->color));
        item->setPen(Qt::NoPen);
        item->setOpacity(CELL_OPACITY);
        addGraphicsItem(item);
        _cellGraphics.insert(it.key(), item);

        // add behavior on mousepress
        connect(item, &GraphicsCell::mousePressedEvent, [=](QPoint cell, bool mod){
            if(!_cells.contains(cell) || !_cellMatrix.contains(cell)) return;

            qDebug() << "Clicked on cell" << cell << mod;
            // select all the reachable cells
            // restore the paramters for the previously selected cell
            if(!isCellNull(_selectedCell) && mod) {
                if(!_cellMatrix[_selectedCell]->contains(cell)) return;

                if(!_plots) {
                    _plots = new DockWidgetPlots(_parent, this); // see to dock the widget on the mainwindow
                }
                if(_parent)
                    _parent->addDockWidget(Qt::RightDockWidgetArea, _plots);
                _plots->show();
                _plots->showLinkData(_selectedCell, cell);
            } else {
                if(!isCellNull(_selectedCell)) {
                    qDebug() << "hello";
                    // restore the "normal" opacity
                    _cellGraphics[_selectedCell]->setOpacity(CELL_OPACITY);
                    // restore the "normal" colors for the neighbor cells
                    auto cells = _cellMatrix[_selectedCell];
                    for(auto jt = cells->begin(); jt != cells->end(); ++jt) {
                        QPoint c = jt.key();
                        if(_cellGraphics.contains(c)) {
                            _cellGraphics[c]->setBrush(QBrush(_cells[c]->color));
                        }
                    }

                    if(_selectedCell == cell) {
                        _selectedCell = QPoint(-1,-1);
                        qDebug() << "clicked on same cell";
                        return;
                    }
                }

                auto cells = _cellMatrix[cell];
                double maxWeight = 0.0;
                for(auto jt = cells->begin(); jt != cells->end(); ++jt) {
                    if(jt.key() == cell) continue;
//                    double avg = jt.value()->travelTimeDist.getAverage();
                    auto val = jt.value();
                    double interVisitAvg = val->interVisitDurationDist.getAverage();
                    int visitCount = val->visits.size();
                    if(interVisitAvg <= 0 || visitCount <= 1) continue;
                    double avg = visitCount / interVisitAvg;
                    if(avg > maxWeight) maxWeight = avg;
                }
                _cellGraphics[cell]->setOpacity(1.0);
                _cellGraphics[cell]->setBrush(CELL_COLOR);
                for(auto jt = cells->begin(); jt != cells->end(); ++jt) {
                    QPoint c = jt.key();
                    if(c == cell) continue;
                    auto val = jt.value();
                    double interVisitAvg = val->interVisitDurationDist.getAverage();
                    int visitCount = val->visits.size();

                    double factor;
                    if(interVisitAvg <= 0 || visitCount <= 1) {
                        factor = 0.0;
                    } else {
                        double avg = visitCount / interVisitAvg;
//                        qDebug() << avg << maxWeight << val->visits.size() << val->interVisitDurationDist.getAverage() << avg / maxWeight;
                        factor = (int) (50.0 + 150.0 * avg / maxWeight);
                    }

                    if(_cellGraphics.contains(c)) {
                        _cellGraphics[c]->setBrush(CELL_COLOR.darker(factor));
                    }
                }

                if(!_plots) {
                    _plots = new DockWidgetPlots(_parent, this); // see to dock the widget on the mainwindow
                }
                if(_parent)
                    _parent->addDockWidget(Qt::RightDockWidgetArea, _plots);
                _plots->show();
                _plots->showCellData(cell);

                _selectedCell = cell;
            }
        });
    }

    return _groupItem;
}

QList<std::tuple<QPointF, double, double> > SpatialStats::getPoints(int deadline, long long startTime, long long endTime)
{
    
}

void SpatialStats::computeLocationAllocation()
{
    qDebug() << "Compute location allocation";

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

    // demands are the cells
    // candidates are the cells

    QSet<QPoint> demandsToCover(_cells.keys().toSet());
    QSet<QPoint> candidatesToAllocate(_cells.keys().toSet());
    // candidate index, <weight allocated, set of demands allocated>
    QHash<QPoint, Allocation*> allocation;

    // iterate for each storage node to allocate
    for(int i = 0; i < nbStorageNodes && !demandsToCover.isEmpty(); ++i) {
        qDebug() << "allocating demands for storage node" << i;
        double maxCoverage = 0.0;
        double maxBackendWeight = 0.0;
        QPoint maxCoverageCell = QPoint(-1,-1);
        QHash<QPoint, double> maxDemandsCovered; // <demand, weight of the demand>

        // find the candidate that covers the most demands

        foreach(QPoint k, candidatesToAllocate) {
//            qDebug() << "\tcandidate" << k;

            QHash<QPoint, double> demandsCovered; // <demand, weight of the demand>
            double coverage = 0.0;

            // compute the score for the previously allocated storage nodes
            foreach(QPoint c, allocation.keys()) {
                if(_cellMatrix.contains(c) && _cellMatrix.value(c)->contains(k)) {
                    auto val = _cellMatrix.value(c)->value(k);
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

                if(_cellMatrix.contains(k) && _cellMatrix.value(k)->contains(c)) {
                    auto val = _cellMatrix.value(k)->value(c);
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
            foreach(QPoint l, demandsToCover) {
                if(_cellMatrix.contains(l) && _cellMatrix.value(l)->contains(k)) {
                    auto val = _cellMatrix.value(l)->value(k);
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
                maxCoverageCell = k;
                maxDemandsCovered = QHash<QPoint, double>(demandsCovered);
            }

            // TODO Substitution part of the algorithm
            // tries to replace each facility one at a time with a
            // facility at another "free" site
        }
        qDebug() << "candidate" << maxCoverageCell << "allocated" << maxCoverage << "with" << maxDemandsCovered.size() << "demands" << demandsToCover.size() << "demands to cover" << maxBackendWeight << "backend weight";

        // reduce the set of the population to cover
        if(!isCellNull(maxCoverageCell)) {
            candidatesToAllocate.remove(maxCoverageCell); // remove the selected cell
            demandsToCover.remove(maxCoverageCell);

            // remove the candidate cells in the vicinty of the selected cell
            QSet<QPoint> candidatesToRemove;
            cellsWithin(candidatesToRemove, candidatesToAllocate, maxCoverageCell, maxDist, maxTravelTime, op, ts);
            candidatesToAllocate.subtract(candidatesToRemove);

            qDebug() << "\tcell" << maxCoverageCell << candidatesToRemove.size();

            demandsToCover.subtract(maxDemandsCovered.keys().toSet());

            // add the allocation
            Allocation* alloc = new Allocation(maxCoverageCell, maxCoverage, maxDemandsCovered,candidatesToRemove,_cellSize);
            allocation.insert(maxCoverageCell, alloc);
        }
    }

    // print the result allocation
    for(auto it = allocation.begin(); it != allocation.end(); ++it) {
        qDebug() << "candidate" << it.key() << "allocated" << it.value()->weight << "with" << it.value()->demands.size() << "demands" << "deleted" << it.value()->deletedCandidates.size();
    }

    // create a new layer
    QString layerName = "Location allocation";
    WeightedAllocationLayer* layer = new WeightedAllocationLayer(_parent, layerName, allocation, _cellSize);
    _allocationLayers.append(layer);
    _parent->createLayer(layerName, layer);
}

bool SpatialStats::pageRank(QHash<QPoint, double> &x, QSet<QPoint> cells, double alpha, int maxIterations, double tolerance)
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
    for(auto it = _cellMatrix.begin(); it != _cellMatrix.end(); ++it) {
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
            if(!_cellMatrix.contains(n)) continue;
            auto neighbors = _cellMatrix.value(n);
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

void SpatialStats::cellsWithin(QSet<QPoint> &cellsWithin, QSet<QPoint> cells, QPoint cell, double distance, double travelTime,  WithinOperator op, TravelTimeStat ts)
{
    if(op == None) return;

    QRectF cell1(cell.x()*_cellSize, cell.y()*+_cellSize, _cellSize, _cellSize);
    foreach(QPoint k, cells) {
        if(k == cell) continue; // skip the cell itself
        bool distance_flag = false, travelTime_flag = false;
        double dist, tt;
        if(distance > 0.0) {
            QRectF cell2(k.x()*_cellSize, k.y()*+_cellSize, _cellSize, _cellSize);
            dist = qSqrt(qPow(cell1.center().x()-cell2.center().x(), 2) + qPow(cell1.center().y()-cell2.center().y(), 2));
            if(islessequal(dist,distance)) {
                distance_flag = true;
            }
        }
        if(travelTime > 0.0) {
            if(_cellMatrix.contains(k) && _cellMatrix.value(k)->contains(cell)) {
                auto val = _cellMatrix.value(k)->value(cell);
                if(ts == Avg)
                    tt = val->travelTimeDist.getAverage();
                else if(ts == Med)
                    tt = val->travelTimeDist.getMedian();
                if(islessequal(tt,travelTime)) {
                    travelTime_flag = true;
                }
            }
        }
        if(op == Or  && (distance_flag || travelTime_flag)) {
            qDebug() << "or / add" << k << dist << tt;
            cellsWithin.insert(k);
        }
        if(op == And && (distance_flag && travelTime_flag)) {
            qDebug() << "and / add" << k << dist << tt;
            cellsWithin.insert(k);
        }
    }
}


void SpatialStats::computePageRank()
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

    // initialize the candidates
    QSet<QPoint> candidatesToAllocate(_cells.keys().toSet());
    QHash<QPoint, Allocation*> allocation; // resulting allocation of the cells

    for(int i = 0; i < nbStorageNodes; ++i) {
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
        cellsWithin(cellsToDelete,candidatesToAllocate,cell,maxDist,maxTravelTime,op,ts);
        candidatesToAllocate.subtract(cellsToDelete); // remove the cells in the vicinity of the selected cell

        qDebug() << "\tcell" << cell << cellsToDelete.size() << "score" << x[cell];

        // add the cell to the allocated cells
        QHash<QPoint, double> demandsAllocated;
        Allocation* alloc = new Allocation(cell, x[cell], demandsAllocated, cellsToDelete, _cellSize);
        allocation.insert(cell, alloc);
    }

    // print the allocation result
    for(auto it = allocation.begin(); it != allocation.end(); ++it) {
        qDebug() << "candidate" << it.key() << "allocated" << it.value()->weight << "with" << it.value()->demands.size() << "demands" << "deleted" << it.value()->deletedCandidates.size();
    }

    // create a new layer
    QString layerName = "PageRank";
    WeightedAllocationLayer* layer = new WeightedAllocationLayer(_parent, layerName, allocation, _cellSize);
    _allocationLayers.append(layer);
    _parent->createLayer(layerName, layer);
}

void SpatialStats::computeCentrality()
{
    qDebug() << "Compute centrality betweeness";
}

void SpatialStats::computePercolationCentrality()
{
    qDebug() << "Compute percolation centrality";
}


void SpatialStats::computeKMeans()

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
