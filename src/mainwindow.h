#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "graphicsscene.h"

// forward class declarations
class Layer;
class Loader;
class LayerPanel;
class TraceLayer;
class AllocationLayer;
class SpatialStats;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString getProjIn() { return _projIn; }
    QString getProjOut() { return _projOut; }
    void addGraphicsItemToScene(QGraphicsItem* item) {
        _scene->addItem(item);
    }
    QRectF getSceneRect() {
        return _scene->sceneRect();
    }
    void addMenu(QMenu* menu);
    void createLayer(QString name = 0, Layer* layer = 0, Loader* loader = 0);

signals:
    void mousePressedEvent(QGraphicsSceneMouseEvent*);

private slots:
    void openShapefile();
    void openTrace();
    void setProjection();
    void addGrid();
    void addRoute();
    void showLayerPanel();
    void closedLayerPanel();
    void onMousePressEvent();
    void computeAllocation();
    void changeLayerOrder(int oldIndex, int newIndex);

private:
    void changeProjection(QString filename = 0, QString projOut = 0);

    Ui::MainWindow *ui;
    QString _projIn = 0, _projOut = 0;
    QList<Layer*> _layers;
    QList<Loader*> _loaders;
    LayerPanel* _layerPanel = 0;
    GraphicsScene* _scene = 0;
    QMenu* _layerMenu = 0;
    QAction* _showLayersAction = 0;
    TraceLayer* _traceLayer = 0;
    AllocationLayer* _allocLayer = 0;
    SpatialStats* _spatialStats = 0;

    int _routeCounter = -1;
};

#endif // MAINWINDOW_H
