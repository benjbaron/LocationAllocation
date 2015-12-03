#ifndef SPATIALSTATS_H
#define SPATIALSTATS_H

#include "qcustomplot.h"
#include "dockwidgetplots.h"
#include "utils.h"

#include "layer.h"

#include "weightedallocationlayer.h"

class TraceLayer;

struct CellInfo {
    QPoint cellId;
    long long startTime;         // start time of the contact in the cell
    long long endTime;           // end time of the contact (regardless of the cell)
};

class MobileNode {
public:
    MobileNode(QString id = "", int cellSize = -1, int sampling = -1):
        _id(id), _cellSize(cellSize), _sampling(sampling) { }


    void addPosition(long long time, double x, double y) {
        // assuming the positions are added sequencially
        if(_prevPos.isNull() || time - _prevTime > 300) {
            // restart the cell recording
            QPoint gc((int)qFloor(x / _cellSize), (int)qFloor(y / _cellSize));
            _prevCell = gc;
            _startTimeCell = time;
            _visitedCells.insert(time, qMakePair(gc, time));
        } else {
            QPointF pos(x,y);
            int nbPos = qMax(1,(int) qCeil((time - _prevTime) / _sampling));
            for(int i = 1; i <= nbPos; ++i) {
                long long t = _prevTime + i*_sampling;
                QPointF p = (time - t)*_prevPos + (t - _prevTime)*pos;
                p /= (time - _prevTime);

                // get the corresponding visited cell
                QPoint gc((int)qFloor(p.x() / _cellSize), (int)qFloor(p.y() / _cellSize));
                // check if cell is different than the previous one
                if(gc != _prevCell) {
                    // add the new cell to the visited cells
                    _visitedCells.insert(time, qMakePair(gc, time));
                    _prevCell = gc;
                    _startTimeCell = time;
                } else {
                    // update the end time of the current cell
                    _visitedCells[_startTimeCell].second = time;
                }
            }
        }
        _prevPos = QPoint(x,y);
        _prevTime = time;
    }

    QMap<long long,QPair<QPoint,long long>> getCells() { return _visitedCells; }

    QMap<long long,QPair<QPoint,long long>> getCells(long long start, long long end) {
        auto up = _visitedCells.lowerBound(start);
        auto it = up;
        if(up.key() != start)
            it = up-1;
        if(up == _visitedCells.end())
            return QMap<long long,QPair<QPoint,long long>>(); // reached the end

        QMap<long long,QPair<QPoint,long long>> res;
        for(; it != _visitedCells.end() && it.key() <= end; ++it) {
            res.insert(it.key(), it.value());
        }

        return res;
    }

private:
    QString _id;
    int _cellSize;
    int _sampling;
    QPoint _prevCell = QPoint();
    long long _startTimeCell;
    long long _prevTime = 0;
    QPointF _prevPos = QPointF();

    // start time, < cell id, duration >
    QMap<long long,QPair<QPoint,long long>> _visitedCells;
};

struct CellValue {
    QPoint cell;
    CellValue(QPoint c) { cell = c; }
    Distribution interVisitDurationDist;
    QList<long long> visitFrequency; // timestamp of the begining of the visit
    QMultiMap<long long, long long> visits; // <start, end>
    QSet<QString> nodes; // nodes that visited the cell
    Distribution travelTimes;
    int connections = 0;
    qreal localStat;
    QColor color;
    qreal medIncomingScore = 0.0; // sum of the score of the incoming edges (with median)
    qreal avgIncomingScore = 0.0; // sum of the score of the incoming edges (with average)
    qreal medScore = 0.0; // score with median of the inter-visit distribution
    qreal avgScore = 0.0; // score with average of the inter-visit distribution
};

struct CellMatrixValue {
    CellMatrixValue(QPoint c1, QPoint c2) { cell1 = c1; cell2 = c2; }
    QPoint cell1, cell2;
    Distribution travelTimeDist;
    Distribution interVisitDurationDist;
    QList<long long> visitFrequency; // timestamp of the begining of the visit
    QMultiMap<long long, long long> visits; // <start, end>
    QSet<QString> nodes; // nodes that visited the link
    qreal medScore = 0.0; // score with median of the inter-visit distribution
    qreal avgScore = 0.0; // score with average of the inter-visit distribution
};


class GraphicsCell: public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    GraphicsCell():
        QGraphicsRectItem(), _id(QPoint()) {}
    GraphicsCell(qreal x, qreal y, qreal w, qreal h, QPoint id):
        QGraphicsRectItem(x, y, w, h), _id(id) {}

signals:
    void mousePressedEvent(QPoint, bool);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        if(event->button() == Qt::LeftButton) {
            event->accept();
            emit mousePressedEvent(_id, (event->modifiers() == Qt::ShiftModifier));
        } else {
            event->ignore();
        }
    }
private:
    QPoint _id;
};




class SpatialStats: public Layer
{
    Q_OBJECT
public:
    SpatialStats(MainWindow* parent = 0, QString name = 0, TraceLayer* traceLayer = 0);

    void populateMobileNodes();
    void computeStats();
    CellValue* getValue(QPoint cell) {
        if(_cells.contains(cell))
            return _cells.value(cell);
        return NULL;
    }
    CellMatrixValue* getValue(QPoint cell1, QPoint cell2) {
        if(_cellMatrix.contains(cell1) && _cellMatrix.value(cell1)->contains(cell2))
            return _cellMatrix.value(cell1)->value(cell2);
        return NULL;
    }
    long long getDuration() { return _duration; }
    QColor selectColorForLocalStat(qreal zScore);
    qreal computeLocalStat(QPoint cell_i);

    QGraphicsItemGroup* draw();
    QList<std::tuple<QPointF,double,double>> getPoints(int deadline = 0, long long startTime = 0, long long endTime = 0);
    bool isCellNull(QPoint cell) {
        return cell.x() == -1 && cell.y() == -1;
    }
    double dist(QPointF a, QPointF b) {
        return qSqrt(qPow(a.x() - b.x(),2) + qPow(a.y() - b.y(),2));
    }
    QPointF cellToCoords(QPoint cell) {
        return QRect(cell.x()*_cellSize, cell.y()*_cellSize, _cellSize, _cellSize).center();
    }

private slots:
    void computeLocationAllocation();
    void computePageRank();
    void computeCentrality();
    void computePercolationCentrality();
    void computeKMeans();

private:
    TraceLayer* _traceLayer;
    QHash<QString, MobileNode*> _mobileNodes;
    QHash<QPoint, QHash<QPoint, CellMatrixValue*>* > _cellMatrix;
    QHash<QPoint, GraphicsCell*> _cellGraphics;
    QHash<QPoint, CellValue*> _cells;
    int _cellSize = 100; // 100 x 100 square meters
    int _sampling = 1; // each 10 seconds
    long long _duration = 86400; // one day
    QPoint _selectedCell = QPoint(-1,-1);
    DockWidgetPlots* _plots = 0;
    QMenu* _statMenu = 0;
    QList<WeightedAllocationLayer*> _allocationLayers;

    /* Compute the page rank for the given set of cells */
    bool pageRank(QHash<QPoint, double>& x, QSet<QPoint> cells, double alpha = 0.85, int maxIterations = 100, double tolerance = 1.0e-6);
    /* Returns the cells from "cells" that are within distance "distance" and/or travel time "travelTime" of "cell" in "cellsWithinDistance", depending on "op" */
    void cellsWithin(QSet<QPoint> &cellsWithin, QSet<QPoint> cells, QPoint cell, double distance = -1.0, double travelTime = -1.0, WithinOperator op = And, TravelTimeStat ts = Med);
};


inline uint qHash(const QPoint &key)
{
    return qHash(key.x()) ^ qHash(key.y());
}


#endif // SPATIALSTATS_H
