#include "spatialstats.h"
#include<QGraphicsItemGroup>

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
    action = _statMenu->addAction("Centrality");
    connect(action, &QAction::triggered, this, &SpatialStats::computeCentrality);

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
    long long deadline = diag.getDeadline();
    int nbStorageNodes = diag.getNbStorageNodes();

    // demands are the cells
    // candidates are the cells

    QSet<QPoint> demandsToCover(_cells.keys().toSet());
    QSet<QPoint> candidatesToAllocate(_cells.keys().toSet());
    // candidate index, <weight allocated, set of demands allocated>
    QHash<QPoint, QPair<double, QSet<QPoint>>> allocation;

    // iterate for each storage node to allocate
    for(int i = 0; i < nbStorageNodes; ++i) {
        qDebug() << "allocating demands for storage node" << i;
        double maxCoverage = 0.0;
        QPoint maxCoverageCell = QPoint(-1,-1);
        QSet<QPoint> maxDemandsCovered;

        // find the candidate that covers the most demands

        foreach(QPoint k, candidatesToAllocate) {
//            qDebug() << "\tcandidate" << k;

            QSet<QPoint> demandsCovered;
            double coverage = 0.0;

            // Compute the covering score for the candidate
            foreach(QPoint l, demandsToCover) {
                if(_cellMatrix.contains(l) && _cellMatrix.value(l)->contains(k)) {
                    auto val = _cellMatrix.value(l)->value(k);
                    double avgTravelTime = val->travelTimeDist.getAverage();
                    if(avgTravelTime <= deadline) {
                        double visitCount = val->visits.size();
                        double interVisitTime = val->interVisitDurationDist.getAverage();
                        if(visitCount > 1) {
                            double weight = visitCount / interVisitTime;
                            coverage += weight;
                            demandsCovered.insert(l);
                        }
                    }
                }
            }

            if(coverage > maxCoverage) {
                maxCoverage = coverage;
                maxCoverageCell = k;
                maxDemandsCovered = QSet<QPoint>(demandsCovered);
            }

            // Substitution part of the algorithm
            // tries to replace each facility one at a time with a
            // facility at another "free" site
        }
        qDebug() << "candidate" << maxCoverageCell << "allocated" << maxCoverage << "with" << maxDemandsCovered.size() << "demands" << demandsToCover.size() << "demands to cover";

        // reduce the set of the population to cover
        if(!isCellNull(maxCoverageCell)) {
            qDebug() << "\tcell" << maxCoverageCell;
            demandsToCover.subtract(maxDemandsCovered);
            allocation.insert(maxCoverageCell, qMakePair(maxCoverage, QSet<QPoint>(maxDemandsCovered)));
            candidatesToAllocate.remove(maxCoverageCell);
        }
    }

    // print the result allocation
    for(auto it = allocation.begin(); it != allocation.end(); ++it) {
        qDebug() << "candidate" << it.key() << "allocated" << it.value().first << "with" << it.value().second.size() << "demands";
    }

    // create a new layer
    QString layerName = "Location allocation";
    _locationAllocationLayer = new WeightedAllocationLayer(_parent, layerName, allocation, _cellSize);
    _parent->createLayer(layerName, _locationAllocationLayer);
}

void SpatialStats::computePageRank()
{
    qDebug() << "Compute page rank";
}

void SpatialStats::computeCentrality()
{
    qDebug() << "Compute centrality betweeness";
}
