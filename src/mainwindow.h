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
    GraphicsScene* scene() {
        return _scene;
    }
    void addMenu(QMenu* menu);
    void createLayer(QString name = 0, Layer* layer = 0, Loader* loader = 0);

signals:
    void mousePressedEvent(QGraphicsSceneMouseEvent*);

private slots:
    void openShapefile();
    void openTrace();
    void openTraceDirectory();
    void setProjection();
    void addGrid();
    void showLayerPanel();
    void closedLayerPanel();
    void onMousePressEvent();
    void changeLayerOrder(int oldIndex, int newIndex);

private:
    void changeProjection(QString filename = 0, QString projOut = 0);

    Ui::MainWindow *ui;
    QString _projIn = 0, _projOut = 0;
    QList<Layer*> _layers;
    LayerPanel* _layerPanel = 0;
    GraphicsScene* _scene = 0;
    QMenu* _layerMenu = 0;
    QAction* _showLayersAction = 0;
};

#endif // MAINWINDOW_H
