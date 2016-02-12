#include "spatial_stats.h"

#include "rest_server.h"
#include "constants.h"
#include "loader.h"

SpatialStats::SpatialStats(MainWindow *parent, QString name, TraceLayer *traceLayer, long long sampling, long long startTime, long long endTime, GeometryIndex *geometryIndex):
    Layer(parent, name), _traceLayer(traceLayer), _sampling(sampling), _startTime(startTime), _endTime(endTime), _geometryIndex(geometryIndex)
{
    _computeAllocation = new ComputeAllocation(this);
    _menu = new QMenu();
    _menu->setTitle("Stat analysis");
    _parent->addMenu(_menu);

    // add action to compute the location allocation
    QAction* action_compute_allocation = _menu->addAction("Compute allocation");
    connect(action_compute_allocation, &QAction::triggered, _computeAllocation, &ComputeAllocation::computeAllocation);

    // add action to the menu to export a contour file
    QAction* action_export_contour = _menu->addAction("Export contour file");
    connect(action_export_contour, &QAction::triggered, this, &SpatialStats::exportContourFile);

    // add action to the menu to launch the REST server
    QAction* action_rest_server = _menu->addAction("Start REST server");
    connect(action_rest_server, &QAction::triggered, this, [=](bool checked) {
        qDebug() << "Lauch REST server";
        if(!_restServer) {
            _restServer = new RESTServer(10, 0, _computeAllocation);
            _restServer->setTimeOut(500);
            _restServer->listen(8080);
            qDebug() << "REST Server listening on port 8080";
        }
    });

    _parent->addMenu(_menu);
    hideMenu();
}

void SpatialStats::populateMobileNodes()
{
    qDebug() << "startTime" << _startTime << "endTime" << _endTime << "sampling" << _sampling;
    // add the successive point positions of the mobile nodes
    auto nodes = _traceLayer->getNodes();

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
    }
}

void SpatialStats::computeStats(Loader* loader)
{
    loader->loadProgressChanged((qreal) 0.0); // initialize the load progress
    loader->changeText("Populate the nodes");
    populateMobileNodes();

    loader->loadProgressChanged((qreal) 0.1);
    loader->changeText("Compute visiting matrix");
    // compute the visiting matrix for the current set of mobile nodes
    int nbNodes = _mobileNodes.size();
    int count = 0;
    qDebug() << "number of mobile nodes" << nbNodes;

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
        loader->loadProgressChanged(0.1 + 0.4 * ((qreal) ++count / (qreal) nbNodes));
    }
    qDebug() << "size of the geometry matrix" << _geometryMatrix.size()
             << "size of the geometry list" << _geometries.size();

    loader->loadProgressChanged((qreal) 0.4);
    loader->changeText("Compute inter-visit durations (cells)");

    qDebug() << "compute the inter-visit durations / cells";
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
        loader->loadProgressChanged(0.5 + 0.16 * ((qreal) ++count / (qreal) size));
    }
    loader->loadProgressChanged((qreal) 0.66);
    loader->changeText("Compute inter-visit durations (matrix)");

    qDebug() << "compute the inter-visit durations / matrix";
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
        loader->loadProgressChanged(0.66 + 0.16 *((qreal) ++count / (qreal) size));
    }
    loader->loadProgressChanged((qreal) 0.82);
    loader->changeText("Compute scores");

    qDebug() << "Compute scores";
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
        loader->loadProgressChanged(0.82 + 0.16 * ((qreal) ++count / (qreal) size));
    }
    loader->loadProgressChanged((qreal) 0.98);
    loader->changeText("Done");

    // load is complete
    qDebug() << "[DONE] Compute spatial stat";
    loader->loadProgressChanged((qreal) 1.0);
}

QColor SpatialStats::selectColorForLocalStat(qreal zScore)
{
    if(zScore >= 3.291) return QColor("#720206");
    else if(zScore >= 2.576) return QColor("#f33f1c");
    else if(zScore >= 1.960) return QColor("#f37b22");
    else if(zScore >= 1.645) return QColor("#fffe38");
    else return QColor("#cccccc");
}

qreal SpatialStats::computeLocalStat(Geometry* geom_i)
{
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

QGraphicsItemGroup* SpatialStats::draw()
{
    _groupItem = new QGraphicsItemGroup();
    _groupItem->setHandlesChildEvents(false);

    for(auto it = _geometryMatrix.begin(); it != _geometryMatrix.end(); ++it) {
        Geometry* geom = it.key();
        GeometryGraphics* item;
        if(geom->getGeometryType() == CircleType)
            item = new CircleGraphics(static_cast<Circle*>(geom));
        else if(geom->getGeometryType() == CellType)
            item = new CellGraphics(static_cast<Cell*>(geom));

        item->setBrush(QBrush(_geometries[it.key()]->color));
        item->setPen(Qt::NoPen);
        item->setOpacity(CELL_OPACITY);
        addGraphicsItem(item);
        _geometryGraphics.insert(it.key(), item);

        // add behavior on mouse press
        connect(item, &GeometryGraphics::mousePressedEvent, [=](Geometry* geom, bool mod){
            if(!_geometries.contains(geom) || !_geometryMatrix.contains(geom)) return;

            qDebug() << "Clicked on geometry" << geom->toString() << mod;
            // select all the reachable geometries
            // restore the paramters for the previously selected geometry
            if(_selectedGeometry && mod) {
                if(!_geometryMatrix[_selectedGeometry]->contains(geom)) return;

                if(!_plots) {
                    _plots = new DockWidgetPlots(_parent, this); // see to dock the widget on the mainwindow
                }
                if(_parent)
                    _parent->addDockWidget(Qt::RightDockWidgetArea, _plots);
                _plots->show();
                _plots->showLinkData(_selectedGeometry, geom);

            } else {
                if(_selectedGeometry) {
                    // restore the "normal" opacity
                    _geometryGraphics[_selectedGeometry]->setOpacity(CELL_OPACITY);
                    // restore the "normal" colors for the neighbor geometries
                    auto geoms = _geometryMatrix[_selectedGeometry];
                    for(auto jt = geoms->begin(); jt != geoms->end(); ++jt) {
                        Geometry* g = jt.key();
                        if(_geometryGraphics.contains(g)) {
                            _geometryGraphics[g]->setBrush(QBrush(_geometries[g]->color));
                            _geometryGraphics[g]->update();
                        }
                    }

                    if(_selectedGeometry == geom) {
                        _geometryGraphics[_selectedGeometry]->setBrush(_geometries[_selectedGeometry]->color);
                        _selectedGeometry = NULL;
                        qDebug() << "clicked on same geometry";
                        return;
                    }
                }

                auto geoms = _geometryMatrix[geom];
                double maxWeight = 0.0;
                for(auto jt = geoms->begin(); jt != geoms->end(); ++jt) {
                    if(jt.key() == geom) continue;
                    double score = jt.value()->avgScore;
                    if(score > maxWeight) maxWeight = score;
                }
                _geometryGraphics[geom]->setOpacity(1.0);
                _geometryGraphics[geom]->setBrush(CELL_COLOR);
                for(auto jt = geoms->begin(); jt != geoms->end(); ++jt) {
                    Geometry* g = jt.key();
                    if(g == geom) continue;
                    auto val = jt.value();
                    double score = val->avgScore;
                    int factor = (int) (50.0 + 150.0 * score / maxWeight);
//                    qDebug() << score << maxWeight << factor;

                    if(_geometryGraphics.contains(g)) {
                        _geometryGraphics[g]->setBrush(CELL_COLOR.darker(factor));
                        _geometryGraphics[g]->update();
                    }
                }

                if(!_plots) {
                    _plots = new DockWidgetPlots(_parent, this); // see to dock the widget on the mainwindow
                }
                if(_parent)
                    _parent->addDockWidget(Qt::RightDockWidgetArea, _plots);
                _plots->show();
                _plots->showNodeData(geom);

                _selectedGeometry = geom;
            }
        });
    }

    return _groupItem;
}

void MobileNode::addPosition(long long time, double x, double y) {
    // assuming the positions are added sequentially
    if(_prevPos.isNull() || time - _prevTime > 300) { // restart the cell recording
        // get the list of geometries that contain the current position
        QSet<Geometry*> geoms = _spatialStats->containsPoint(x,y);
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
            QSet<Geometry*> geoms = _spatialStats->containsPoint(p);

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

void SpatialStats::exportContourFile() {
    if(!_selectedGeometry) {
        QMessageBox q(QMessageBox::Warning, "Error", "No geometry is selected", QMessageBox::Ok);
        q.exec(); // synchronous
        return;
    }

    // get output filename
    QString filename = QFileDialog::getSaveFileName(0,
                                                    tr("Export contour file"),
                                                    QString(),
                                                    tr("CSV file (*.csv)"));
    if(filename.isEmpty())
        return;

    // compute the contour plot
    qDebug() << "Export the contour plot file";
    QFile file(filename);
    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "Unable to write in file "<< filename;
        return;
    }

    Geometry* randomGeom = _geometries.keys().first();
    double cellSize = randomGeom->getBounds().getQRectF().width();
    QPointF topLeft = randomGeom->getBounds().getTopLeft();
    QPointF bottomRight = randomGeom->getBounds().getBottomRight();
    for(Geometry* geom : _geometries.keys()) {
        QPointF tl = geom->getBounds().getTopLeft();
        QPointF br = geom->getBounds().getBottomRight();
        if(tl.x() < topLeft.x())
            topLeft.setX(tl.x());
        if(tl.y() < topLeft.y())
            topLeft.setY(tl.y());
        if(br.x() > bottomRight.x())
            bottomRight.setX(br.x());
        if(br.y() > bottomRight.y())
            bottomRight.setY(br.y());
    }

    QTextStream out(&file);
    // first line (header)
    out << "x;y;z\n";
//    out << QString::number(_selectedGeometry->getCenter().x(), 'f', 4) << ";"
//        << QString::number(_selectedGeometry->getCenter().y(), 'f', 4) << ";"
//        << QString::number(1e10, 'f', 4) << "\n";

    qDebug() << topLeft << bottomRight << cellSize << (bottomRight.x() - topLeft.x()) / cellSize << (bottomRight.y() - topLeft.y()) / cellSize;
    auto cells = _geometryMatrix.value(_selectedGeometry);
    for(double i = topLeft.x(); i < bottomRight.x(); i += 10) {
        for(double j = topLeft.y(); j < bottomRight.y(); j += 10) {
            QSet<Geometry*> geoms = _geometryIndex->getGeometriesAt(i, j);
            bool foundRightGeom = false;
            if(!geoms.isEmpty()) {
                for(Geometry* geom : geoms) {
                    if(geom->getGeometryType() == CellType) {
                        // add the geometry to the output
                        if(cells->contains(geom)) {
                            GeometryMatrixValue* val = cells->value(geom);
//                            double x = geom->getCenter().x();
//                            double y = geom->getCenter().y();
                            double z = 10 * val->avgScore;
                            out << QString::number(i, 'f', 0) << ";"
                                << QString::number(j, 'f', 0) << ";"
                                << QString::number(z, 'f', 2) << "\n";
                            foundRightGeom = true;
                            break;
                        }
                    }
                }
            }
            if(!foundRightGeom) {
                // add the generic output
                out << QString::number(i, 'f', 0) << ";"
                    << QString::number(j, 'f', 0) << ";"
                    << QString::number(0, 'f', 2) << "\n";
            }
        }
    }
//    for(Geometry* geom : cells->keys()) {
//        GeometryMatrixValue* val = cells->value(geom);
//        double x = geom->getCenter().x();
//        double y = geom->getCenter().y();
//        double z = 100.0 * val->avgScore;
//
//        out << QString::number(x, 'f', 0) << ";"
//            << QString::number(y, 'f', 0) << ";"
//            << QString::number(z, 'f', 2) << "\n";
//    }

    file.close();

    qDebug() << "[DONE] export contour file";
}

bool SpatialStats::load(Loader *loader) {
    computeStats(loader);
    return true;
}
