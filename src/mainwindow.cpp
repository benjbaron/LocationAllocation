#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QTcpServer>

#include "graphicsview.h"
#include "projection_dialog.h"
#include "progress_dialog.h"
#include "shapefile_layer.h"
#include "trace_layer.h"
#include "grid_layer.h"
#include "layer_panel.h"
#include "proj_factory.h"
#include "loader.h"
#include "gtfs_layer.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QPixmapCache::setCacheLimit(102400);

    // initialize the graphics scene
    _scene = new GraphicsScene();
    GraphicsView* v = new GraphicsView(_scene, this);
    setCentralWidget(v);

    v->setCacheMode(QGraphicsView::CacheBackground);


    // set up the layers menu
    _layerMenu = new QMenu();
    _layerMenu->setTitle("Layers");
    _showLayersAction = _layerMenu->addAction("Show Layers");
    ui->menuBar->addMenu(_layerMenu);

    // set up the layer panel (do not show)
    _layerPanel = new LayerPanel(this, &_layers);
    _layerPanel->setVisible(false);
    addDockWidget(Qt::LeftDockWidgetArea, _layerPanel);

    // connections
    connect(ui->actionOpen_Shapefile, &QAction::triggered, this, &MainWindow::openShapefile);
    connect(ui->actionOpen_Trace, &QAction::triggered, this, &MainWindow::openTrace);
    connect(ui->actionOpen_TraceDir, &QAction::triggered, this, &MainWindow::openTraceDirectory);
    connect(ui->actionOpen_GTFS, &QAction::triggered, this, &MainWindow::openGTFSDirectory);
    connect(ui->actionSet_projection, &QAction::triggered, this, &MainWindow::setProjection);
    connect(_showLayersAction, &QAction::triggered, this, &MainWindow::showLayerPanel);
    connect(_scene, &GraphicsScene::mousePressedEvent, this, &MainWindow::onMousePressEvent);
    connect(_layerPanel, &LayerPanel::closedEvent, this, &MainWindow::closedLayerPanel);
    connect(_layerPanel, &LayerPanel::reorderEvent, this, &MainWindow::changeLayerOrder);
    connect(ui->actionAdd_Grid, &QAction::triggered, this, &MainWindow::addGrid);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addMenu(QMenu *menu) {
    ui->menuBar->addMenu(menu);
}

void MainWindow::openShapefile()
{
    QSettings settings;
    QString filename = QFileDialog::getOpenFileName(this,
                       "Open a Shapefile",
                       settings.value("defaultShapefilePath",
                                      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                       "Shapefile (*.shp);;WKT files (*.wkt)");

    if(filename.isEmpty()) {
        return;
    }
    // Save the filename path in the app settings
    settings.setValue("defaultShapefilePath", QFileInfo(filename).absolutePath());
    QString name = QFileInfo(filename).fileName();

    changeProjection(name, _projOut);

    // instantiate a new layer and a new loader
    ShapefileLayer* layer = new ShapefileLayer(this, name, filename);
    Loader* loader = new Loader(layer);

    createLayer(name, layer, loader);
}

void MainWindow::openTrace()
{
    QSettings settings;
    QFileDialog d(this, "Open a trace directory");
    d.setFileMode(QFileDialog::Directory);
    QString filename = d.getOpenFileName(this,
                                         "Open a trace directory",
                                         settings.value("defaultTracePath", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString());


    if(filename.isEmpty()) {
        return;
    }
    // Save the filename path in the app settings
    settings.setValue("defaultTracePath", QFileInfo(filename).absolutePath());
    QString name = QFileInfo(filename).fileName();
    qDebug() << "opened" << name;

    changeProjection(name, _projOut);

    // instantiate a new layer and a new loader
    TraceLayer* layer  = new TraceLayer(this, name, filename);
    Loader loader(layer);
    createLayer(name, layer, &loader);
}


void MainWindow::openTraceDirectory() {
    QSettings settings;
    QFileDialog d(this, "Open a trace directory");
    d.setFileMode(QFileDialog::Directory);
    QString path = d.getExistingDirectory(this,
                                          "Open a trace directory",
                                          settings.value("defaultTraceDirPath", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString());

    if(path.isEmpty()) {
        return;
    }
    // Save the filename path in the app settings
    settings.setValue("defaultTraceDirPath", QFileInfo(path).absolutePath());
    QString name = QFileInfo(path).fileName();
    qDebug() << "opened" << name;

    changeProjection(name, _projOut);

    // instantiate a new layer and a new loader
    TraceLayer* layer  = new TraceLayer(this, name, path);
    Loader loader(layer);
    createLayer(name, layer, &loader);
    qDebug() << "[DONE] opening directory" << path;
}

void MainWindow::openGTFSDirectory() {
    QSettings settings;
    QFileDialog d(this, "Open a GTFS directory");
    d.setFileMode(QFileDialog::Directory);
    QString path = d.getExistingDirectory(this,
                                          "Open a GTFS directory",
                                          settings.value("defaultGTFSPath", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString());

    if(path.isEmpty()) {
        return;
    }
    // Save the filename path in the app settings
    settings.setValue("defaultGTFSPath", QFileInfo(path).absolutePath());
    QString name = QFileInfo(path).fileName();
    qDebug() << "opened" << name;

    changeProjection(name, _projOut);

    // instantiate a new layer and a new loader
    GTFSLayer* layer  = new GTFSLayer(this, name, path);
    Loader loader(layer);
    createLayer(name, layer, &loader);
    qDebug() << "[DONE] opening GTFS directory" << path;
}

void MainWindow::setProjection()
{
    changeProjection("global", _projOut);
}

void MainWindow::addGrid()
{
    QString name = "grid";
    GridLayer* layer = new GridLayer(this, name, GRID_SIZE);
    Loader loader(layer);
    createLayer(name, layer, &loader);
}

void MainWindow::changeProjection(QString filename, QString projOut)
{
    // execute the projection dialog
    ProjectionDialog projectionDialog(this, filename, projOut);
    int ret = projectionDialog.exec(); // synchronous
    if (ret == QDialog::Rejected) {
        return;
    }
    // Get the projections
    projectionDialog.getProjections(&_projIn, &_projOut);
    ProjFactory::getInstance().setProj(_projIn, _projOut);
}

void MainWindow::closedLayerPanel()
{
    // received a close event from the layer panel
    _showLayersAction->setText("Show Layers");
}

void MainWindow::showLayerPanel() {
    // toggle the layer panel
    if(_layerPanel->isVisible()) {
        // hide the layer panel
        _showLayersAction->setText("Show Layers");
        _layerPanel->setVisible(false);
    } else {
        // show the layer panel
        _showLayersAction->setText("Hide Layers");
        _layerPanel->setVisible(true);
    }
}

void MainWindow::onMousePressEvent()
{

}

void MainWindow::changeLayerOrder(int oldIndex, int newIndex)
{
    int nbLayers = _layers.size();
    Layer* tmp = _layers.at(oldIndex);
    if(newIndex > oldIndex) {
        for(int i = oldIndex; i < newIndex; ++i) {
            Layer* layer = _layers.at(i+1);
            _layers.replace(i, layer);
            layer->setZValue((qreal) (nbLayers-1-i));
        }
    } else if(newIndex < oldIndex) {
        for(int i = oldIndex; i > newIndex; --i) {
            Layer* layer = _layers.at(i-1);
            _layers.replace(i, layer);
            layer->setZValue((qreal) (nbLayers-1-i));
        }
    }
    _layers.replace(newIndex, tmp);
    tmp->setZValue((qreal) (nbLayers-1-newIndex));
    _scene->update();
}

void MainWindow::createLayer(QString name, Layer* layer, Loader* loader)
{
    if(loader) {
        ProgressDialog progressDiag(this, "Loading "+name);
        connect(loader, &Loader::loadProgressChanged, &progressDiag, &ProgressDialog::updateProgress);
        connect(loader, &Loader::changeText, &progressDiag, &ProgressDialog::changeText);
        loader->load(layer);
        progressDiag.exec();
    }

    if(layer) {
        // increase all the layers z value to leave the z value 0 open
        for(Layer* l: _layers) {
            l->setZValue(l->getZValue()+1);
        }

        // Draw the shapefile on the scene
        addGraphicsItemToScene(layer->draw());
        layer->setZValue(0.0);

        // add the layer to the layer panel
        _layers.append(layer);
        _layerPanel->addLayer(layer);
    }
}
