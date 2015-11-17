#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

#include "graphicsview.h"
#include "projectiondialog.h"
#include "progressdialog.h"
#include "shapefileloader.h"
#include "shapefilelayer.h"
#include "tracelayer.h"
#include "traceloader.h"
#include "gridlayer.h"
#include "allocationlayer.h"
#include "layer.h"
#include "layerpanel.h"
#include "loader.h"
#include "graphicsscene.h"
#include "osrmwrapper.h"
#include "locationallocationdialog.h"
#include "spatialstats.h"
#include "intermediateposlayer.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // initialize the graphics scene
    _scene = new GraphicsScene();
    GraphicsView* v = new GraphicsView(_scene, this);
    setCentralWidget(v);
    
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
    connect(ui->actionSet_projection, &QAction::triggered, this, &MainWindow::setProjection);
    connect(_showLayersAction, &QAction::triggered, this, &MainWindow::showLayerPanel);
    connect(_scene, &GraphicsScene::mousePressedEvent, this, &MainWindow::onMousePressEvent);
    connect(_layerPanel, &LayerPanel::closedEvent, this, &MainWindow::closedLayerPanel);
    connect(_layerPanel, &LayerPanel::reorderEvent, this, &MainWindow::changeLayerOrder);
    connect(ui->actionAdd_Grid, &QAction::triggered, this, &MainWindow::addGrid);
    connect(ui->actionCompute_allocation, &QAction::triggered, this, &MainWindow::computeAllocation);
    connect(ui->actionExport_Trace_grid, &QAction::triggered, [=](bool checked) {
        if(_traceLayer) {
            QString filename = QFileDialog::getSaveFileName(0,
                                                            tr("Save the Trace layer"),
                                                            QString(),
                                                            tr("Shapefile file (*.shp)"));

            if(filename.isEmpty())
                return;

            qDebug() << "Exporting shapefile grid of" << filename;
            _traceLayer->exportLayerGrid(filename);
        }
    });
    connect(ui->actionShow_intermediate_points, &QAction::triggered, [=](bool checked) {
        if(_traceLayer) {

            qDebug() << "Show intermediate points of" << _traceLayer->getName();
            QString name = "intermediate positions";
            IntermediatePosLayer* layer   = new IntermediatePosLayer(this, name, _traceLayer);
            createLayer(name, layer);
        }
    });

    connect(ui->actionExport_Trace_txt, &QAction::triggered, [=](bool checked) {
       if(_traceLayer) {
           QString filename = QFileDialog::getSaveFileName(0,
                                                           tr("Save the Trace layer"),
                                                           QString(),
                                                           tr("CSV file (*.csv)"));

           if(filename.isEmpty())
               return;

           qDebug() << "Exporting text of" << filename;
           _traceLayer->exportLayerText(filename);
       }
    });
    connect(ui->actionExport_Trace, &QAction::triggered, [=](bool checked) {
        if(_traceLayer) {
            QString filename = QFileDialog::getSaveFileName(0,
                                                            tr("Save the Trace layer"),
                                                            QString(),
                                                            tr("Shapefile file (*.shp)"));

            if(filename.isEmpty())
                return;

            qDebug() << "Exporting" << filename;
            _traceLayer->exportLayer(filename);
        }
    });
    connect(ui->actionExport_ONETrace, &QAction::triggered, [=](bool checked){
       if(_traceLayer) {
           QString filename = QFileDialog::getSaveFileName(0,
                                                           tr("Save the Trace layer"),
                                                           QString(),
                                                           tr("Text file (*.txt)"));

           if(filename.isEmpty())
               return;

           qDebug() << "Exporting ONE trace" << filename;
           _traceLayer->exportLayerONE(filename);
       }
    });
    connect(ui->actionExport_facilities, &QAction::triggered, [=](bool checked){
       if(_traceLayer) {
           QString filename = QFileDialog::getSaveFileName(0,
                                                           tr("Save the facility locations"),
                                                           QString(),
                                                           tr("Text file (*.txt)"));

           if(filename.isEmpty())
               return;

           qDebug() << "Exporting facility locations" << filename;
           _allocLayer->exportFacilities(filename);
       }
    });

    connect(ui->actionSpatial_Stats, &QAction::triggered, [=](bool checked) {
        if(_traceLayer) {
            qDebug() << "Compute spatial Stats";
            if(!_spatialStats) {
                _spatialStats = new SpatialStats(this, "Spatial Stats layer", _traceLayer);
            }

            _spatialStats->computeStats();
            createLayer("Spatial Stats", _spatialStats);
        }
    });

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
                                                    settings.value("defaultShapefilePath", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                    tr("Shapefile (*.shp)"));
    if(filename.isEmpty()) {
        return;
    }
    // Save the filename path in the app settings
    settings.setValue("defaultShapefilePath", QFileInfo(filename).absolutePath());
    QString name = QFileInfo(filename).fileName();

    changeProjection(name, _projOut);

    // instanciate a new layer and a new loader
    ShapefileLayer* layer = new ShapefileLayer(this, name);
    ShapefileLoader* loader = new ShapefileLoader(this, filename, layer);

    createLayer(name, layer, loader);

    QMenu* menu = new QMenu();
    menu->setTitle("Shapefile");
    QAction* actionOpen_showShapefile = menu->addAction("Show shapefile");
    connect(actionOpen_showShapefile, SIGNAL(triggered(bool)), this, SLOT(showShapefile()));
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

    // instanciate a new layer and a new loader
    TraceLayer* layer   = new TraceLayer(this, name);
    TraceLoader* loader = new TraceLoader(this, filename, layer);
    _traceLayer = layer;

    createLayer(name, layer, loader);
}

void MainWindow::setProjection()
{
    changeProjection("global", _projOut);
}

void MainWindow::addGrid()
{
    QString name = "grid";
    GridLayer* layer = new GridLayer(this, name, GRID_SIZE);
    createLayer(name, layer);
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
    OSRMWrapper::getInstance().setProj(_projIn, _projOut);
}

void MainWindow::addRoute()
{

}

void MainWindow::closedLayerPanel()
{
    // recieved a close event from the layer panel
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

void MainWindow::computeAllocation()
{
    // execute the compute location dialog
    LocationAllocationDialog locAllDialog(this, "Location allocation", &_layers);
    int ret = locAllDialog.exec(); // synchronous
    if (ret == QDialog::Rejected) {
        return;
    }
    // Get the candidate and demand layers
    Layer* candidate;
    Layer* demand;
    int deadline, nbFacilities, cellSize;
    long long startTime, endTime;
    locAllDialog.getParameters(candidate, demand, &deadline, &startTime, &endTime, &nbFacilities, &cellSize);
    qDebug() << "MainWindow" << (candidate ? candidate->getName() : "grid") << demand->getName() << deadline << startTime << endTime;

    // run the Allocation layer
    AllocationLayer* allocLayer = new AllocationLayer(this, "Allocation layer", candidate, demand, deadline, startTime, endTime, nbFacilities, cellSize);
    allocLayer->computeAllocationGridTrace();

    createLayer("Allocation", allocLayer);

    _allocLayer = allocLayer;
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
            qDebug() << "layer" << layer->getName() << "z" << layer->getZValue();
        }
    } else if(newIndex < oldIndex) {
        for(int i = oldIndex; i > newIndex; --i) {
            Layer* layer = _layers.at(i-1);
            _layers.replace(i, layer);
            layer->setZValue((qreal) (nbLayers-1-i));
            qDebug() << "layer" << layer->getName() << "z" << layer->getZValue();
        }
    }
    _layers.replace(newIndex, tmp);
    tmp->setZValue((qreal) (nbLayers-1-newIndex));
    qDebug() << "layer" << tmp->getName() << "z" << tmp->getZValue();
    _scene->update();
}

void MainWindow::createLayer(QString name, Layer* layer, Loader* loader)
{
    if(loader) {
        ProgressDialog* progressDiag = new ProgressDialog(this, "Loading "+name);
        connect(loader, &Loader::loadProgressChanged, progressDiag, &ProgressDialog::updateProgress);
        loader->load();
        progressDiag->exec();
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
