#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "graphicsscene.h"

// forward class declarations
class Layer;
class Loader;
class LayerPanel;
class TraceLayer;
class SpatialStats;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
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
    QGraphicsScene* scene() {
        return _scene;
    }
    void addMenu(QMenu* menu);
    void createLayer(QString name = 0, Layer* layer = 0, Loader* loader = 0);
    void changeProjection(QString filename = 0, QString projOut = 0);
    QString getProjIn(QString filename = 0, QString projOut = 0);

private slots:
    void openShapefile();
    void openTrace();
    void openTraceDirectory();
    void openGTFSDirectory();
    void openFlickrFile();
    void openRoadTrafficData();
    void openWazeData();
    void setProjection();
    void addGrid();
    void showLayerPanel();
    void closedLayerPanel();
    void changeLayerOrder(int oldIndex, int newIndex);
    void exportPDF();

private:
    Ui::MainWindow *ui;
    QString _projIn = QString();
    QString _projOut = QString();
    QList<Layer*> _layers;
    LayerPanel* _layerPanel = 0;
    QGraphicsScene* _scene = 0;
    QMenu* _layerMenu = 0;
    QAction* _showLayersAction = 0;
};

#endif // MAINWINDOW_H
