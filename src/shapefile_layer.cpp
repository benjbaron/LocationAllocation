#include "shapefile_layer.h"

#include "number_dialog.h"
#include "proj_factory.h"
#include "shapefile_attribute_dialog.h"

QGraphicsItemGroup* ShapefileLayer::draw() {
    QPen pen = QPen(SHAPEFILE_COL);
    pen.setWidth(SHAPEFILE_WID);
    pen.setCosmetic(true);

    _groupItem = new QGraphicsItemGroup();
    _groupItem->setHandlesChildEvents(false);

    QHash<int,ShapefileFeature*>* features = _shapefile->getFeatures();
    for(auto it = features->begin(); it != features->end(); ++it) {
        GeometryGraphics* item = nullptr;
        Geometry* geometry = it.value()->geometry;

        if(geometry->getGeometryType() == GeometryType::PathGeometryType) {
            item = new ArrowPathGraphics(static_cast<Path*>(geometry));
            pen.setColor(SHAPEFILE_COL);
            item->setPen(pen);
            addGraphicsItem(item);

        } else if(geometry->getGeometryType() == GeometryType::CircleGeometryType) {
            item = new CircleGraphics(static_cast<Circle*>(geometry));
            item->setBrush(QBrush(SHAPEFILE_COL));
            item->setPen(Qt::NoPen);
            addGraphicsItem(item);

        } else if(geometry->getGeometryType() == GeometryType::PolygonGeometryType) {
            item = new PolygonGraphics(static_cast<Polygon*>(geometry));
            pen.setColor(SHAPEFILE_COL);
            item->setPen(pen);
            addGraphicsItem(item);
        }

        if(item != nullptr) {
            connect(item, &GeometryGraphics::mousePressedEvent, this, &ShapefileLayer::onFeatureSelectedEvent);
            _geometryShapefileFeatures.insert(geometry, it.value());
            _geometryGraphics.insert(geometry, item);
        }
    }

    return _groupItem;
}

void ShapefileLayer::computeIntersections() {
    // run the intersection detection and generate a PointLayer
    QSet<QPointF> intersections = _shapefile->getIntersections();
    QString layerName =  getName() + " / intersections";
    _pointLayer = new PointLayer(_parent, layerName, intersections.toList());
    // add the layer to the mainwindow
    getParent()->createLayer(layerName, _pointLayer);

    // add an action to the menu
    QAction* action = _menu->addAction("Export intersection points");
    connect(action, &QAction::triggered, this, &ShapefileLayer::exportIntersectionPoints);
}


void ShapefileLayer::exportIntersectionPoints() {
    if(!_pointLayer) return;

    // choose file
    QString filename = QFileDialog::getSaveFileName(0,
                                                    tr("Save the intersection points"),
                                                    QString(),
                                                    tr("CSV file (*.csv)"));

    if(filename.isEmpty())
        return;

    // choose radius
    NumberDialog numDiag(_parent, "Set radius");
    numDiag.addField("Radius", 0);
    int ret = numDiag.exec(); // synchronous
    if (ret == QDialog::Rejected) {
        return;
    }
    int radius = (int)numDiag.getNumber(0);

    qDebug() << "Exporting" << _pointLayer->get_points().size() << "intersection points in" << filename;
    QFile file(filename);
    if(!file.open(QFile::WriteOnly)) {
        qDebug() << "Unable to write in file "<< filename;
        return;
    }

    QTextStream out(&file);
    for(const QPointF p : _pointLayer->get_points()) {
        out << QString::number(p.x(), 'f', 4) << ";"
            << QString::number(p.y(), 'f', 4) << ";"
            << QString::number(radius) << "\n";
    }
    file.close();

    qDebug() << "[DONE] export intersection points";
}

bool ShapefileLayer::load(Loader* loader) {
    _shapefile->open(loader);
    return true;
}

void ShapefileLayer::showAttributes() {
    qDebug() << "show shapefile attributes";
    if(_shapefileAttributes == nullptr) {
        _shapefileAttributes = new ShapefileAttributeDialog(_parent, this);
        _shapefileAttributes->showShapefileAttributes(_shapefile);

        // connect the feature selection
        connect(_shapefileAttributes, &ShapefileAttributeDialog::rowSelected, [=](int idx) {
            Geometry* geom = _shapefile->getFeature(idx)->geometry;
            if(_selectedGeometry == nullptr) {
                _geometryGraphics.value(geom)->selected(true);
                _selectedGeometry = geom;
            } else {
                _geometryGraphics.value(_selectedGeometry)->selected(false);
                _geometryGraphics.value(geom)->selected(true);
                _selectedGeometry = geom;
            }
        });
    }
    _shapefileAttributes->show();
}

void ShapefileLayer::onFeatureSelectedEvent(Geometry* geom, bool mod) {
    qDebug() << "receive onFeatureSelectedEvent" << _geometryShapefileFeatures.value(geom)->id;
    if(_selectedGeometry == nullptr) {
        _geometryGraphics.value(geom)->selected(true);
        _selectedGeometry = geom;
    } else if (_selectedGeometry != geom) {
        _geometryGraphics.value(_selectedGeometry)->selected(false);
        _geometryGraphics.value(geom)->selected(true);
        _selectedGeometry = geom;
    } else if (_selectedGeometry == geom) {
        _geometryGraphics.value(geom)->selected(false);
        _selectedGeometry = nullptr;
    }

    // emit the signal
    if(_selectedGeometry != nullptr) {
        int idx = _geometryShapefileFeatures.value(_selectedGeometry)->id;
        qDebug() << "send signal for selected feature" << idx;
        featureSelected(idx);
    }
}

void ShapefileLayer::projectPoints() {

    /** get the points file and parse the points */
    QSettings settings;
    QString filename = QFileDialog::getOpenFileName(_parent,
                                                    "Open a points file",
                                                    settings.value("defaultPointProjectPath",
                                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString());

    if(filename.isEmpty()) {
        return;
    }

    settings.setValue("defaultPointProjectPath", QFileInfo(filename).absolutePath());
    QString name = QFileInfo(filename).fileName();

    QString pIn = _parent->getProjIn(name, _parent->getProjOut());

    Loader l;
    QFuture<bool> future;
    ProgressDialog p(_parent);
    QObject::connect(&l, &Loader::loadProgressChanged, &p, &ProgressDialog::updateProgress);

    qDebug() << "remove the connected components that are not connected";
    future = l.load(_shapefile, &Shapefile::removeNotConnectedComponents, &l);
    p.exec();

    future.result(); // wait for the results

    qDebug() << "Project points" << name;

    QFile* file = new QFile(filename);
    if(!file->open(QFile::ReadOnly | QFile::Text))
        return;

    // prepare the projected points
    QHash<QPointF,ProjectedPoint*> projPoints;
    while(!file->atEnd()) {
        QString line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
        if(line.isEmpty()) continue;
        QStringList fields = line.split(" ");
        double lat = fields.at(0).toDouble();
        double lon = fields.at(1).toDouble();

        // convert the coordinates
        double x, y;
        ProjFactory::getInstance().transformCoordinates(pIn, lat, lon, &x, &y);

        if(x < 0 || y < 0)
            continue;

        QPointF pt(x,y);
        ProjectedPoint* pjPt = new ProjectedPoint(pt,QPointF());
        projPoints.insert(pt,pjPt);
    }

    // run the point projection
    future = l.load(_shapefile, &Shapefile::projectPoints, &l, &projPoints);
    p.exec();

    future.result(); // wait for the results

    ProjectedPointsLayer* layer = new ProjectedPointsLayer(_parent, "projected points", projPoints, _shapefile);
    Loader loader;
    _parent->createLayer(name, layer, &loader);

    QString output = QFileDialog::getSaveFileName(0,
                                                    tr("Export the wkt file"),
                                                    QString(),
                                                    tr("WKT file (*.wkt)"));

    if(output.isEmpty()) {
        return;
    }

    future = l.load(_shapefile, &Shapefile::exportWKT, &l, output);
    p.exec();

    future.result(); // wait for the results
}

void ShapefileLayer::exportWKT() {

    QString filename = QFileDialog::getSaveFileName(0,
                                                    tr("Export the WKT file"),
                                                    QString(),
                                                    tr("WKT file (*.wkt)"));

    if(filename.isEmpty())
        return;

    QFile file(filename);
    if(!file.open(QFile::WriteOnly)) {
        qWarning() << "Unable to write in file" << filename;
        return;
    }

    Loader l;
    QFuture<bool> future;
    ProgressDialog p;
    QObject::connect(&l, &Loader::loadProgressChanged, &p, &ProgressDialog::updateProgress);
    future = l.load(_shapefile, &Shapefile::exportWKT, &l, filename);
}