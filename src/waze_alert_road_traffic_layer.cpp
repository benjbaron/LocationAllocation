//
// Created by Benjamin Baron on 17/11/2016.
//

#include "waze_alert_road_traffic_layer.h"
#include "geometry_layer.h"

void WazeAlertRoadTrafficLayer::addMenuBar() {
    _menu = new QMenu();
    _menu->setTitle("Waze alerts road Traffic");
    _parent->addMenu(_menu);

    // add action to the menu to show the buffered road links
    QAction* actionBufferedRoadLinks = _menu->addAction("Show buffered road links");
    connect(actionBufferedRoadLinks, &QAction::triggered, this, [=](bool checked){
        qDebug() << "Show buffered road links";
        if(!_shapefile) {
            return;
        }

        QString layerName = "Buffered road links";
        GeometryLayer* geometryLayer = new GeometryLayer(_parent, layerName,
                                                         static_cast<WazeAlertRoadTraffic*>(_shapefile)->getBufferedRoadLinks()->keys());
        Loader loader;
        _parent->createLayer(layerName, geometryLayer, &loader);

        /* display the corresponding geometry index */
        layerName = "Geometry index";

        geometryLayer = new GeometryLayer(_parent, layerName,
                                          static_cast<WazeAlertRoadTraffic*>(_shapefile)->getGeometryIndex()->getGrid());
        _parent->createLayer(layerName, geometryLayer, &loader);
    });

    QAction* actionShowProjectedAlerts = _menu->addAction("Show projected alerts");
    connect(actionShowProjectedAlerts, &QAction::triggered, this, [=](bool checked){
        qDebug() << "Show projected alerts";
        if(!_shapefile) {
            return;
        }

        QString layerName = "Projected alerts";
        ProjectedWazeAlertsLayer* layer = new ProjectedWazeAlertsLayer(_parent, layerName,
                                                               static_cast<WazeAlertRoadTraffic*>(_shapefile)->getProjectedPoints(), _shapefile);

        Loader loader;
        _parent->createLayer(layerName, layer, &loader);
    });
}

QGraphicsItemGroup* WazeAlertRoadTrafficLayer::draw() {
    /* Draw the shapefile */
    ShapefileLayer::draw();

    qDebug() << "Shapefile drawn:" << _geometryGraphics.size();

    WazeAlertRoadTraffic* wazeAlertRoadTraffic = static_cast<WazeAlertRoadTraffic*>(_shapefile);
    QHash<QString, RoadLinkWazeAlerts*>* rla = wazeAlertRoadTraffic->getRoadLinksAlerts();

    // Display the road links with colors that vary as a function of the number of alerts aggregated per road link
    QMap<int, int> sortedRoadLinks;
    for(auto it = rla->begin(); it != rla->end(); ++it) {
        int nbAlerts = it.value()->alerts.size();
        if(nbAlerts <= 0.0)
            continue;
        if(!sortedRoadLinks.contains(nbAlerts))
            sortedRoadLinks.insert(nbAlerts, 1);
        else
            sortedRoadLinks[nbAlerts]++;
    }

    qDebug() << rla->size() << " road links " << sortedRoadLinks.size() << " sorted road flows";

    int cumSum = 0;
    for(auto it = sortedRoadLinks.begin(); it != sortedRoadLinks.end(); ++it) {
        int count = it.value();
        cumSum += count;
        sortedRoadLinks[it.key()] = cumSum;
    }

    for(auto it = _geometryShapefileFeatures.begin(); it != _geometryShapefileFeatures.end(); ++it) {
        int idx = it.value()->id;
        RoadLinkWazeAlerts* rlwa = wazeAlertRoadTraffic->getRoadLinkAlerts(idx);
        qDebug() << "index" << idx << "roadLinkAlerts" << rlwa;
        if(rlwa == nullptr)
            continue;

        int nbAlerts = rlwa->alerts.size();
        qDebug() << "\tNumber of alerts" << nbAlerts;
        QColor color;
        if(sortedRoadLinks.contains(nbAlerts)) {
            if(sortedRoadLinks.value(nbAlerts) < (cumSum / 4.0))
                color = COL_LOW;
            else if(sortedRoadLinks.value(nbAlerts) < (cumSum / 2.0))
                color = COL_MED_LOW;
            else if(sortedRoadLinks.value(nbAlerts) < 3*(cumSum / 4.0))
                color = COL_MED_HIGH;
            else
                color = COL_HIGH;
        }
        if(color.isValid()) {
            qDebug() << "\tColor" << color;
            _geometryGraphics.value(it.key())->setUnselectedColor(color);
        }
    }

    /* Display the alerts to display (in set _alertsToDisplay) */
    if(_alertsDisplayed == nullptr && !_alertsToDisplay.isEmpty()) {
        _alertsDisplayed = new QGraphicsItemGroup();
        _alertsDisplayed->setHandlesChildEvents(false);
    }
    addGraphicsItem(_alertsDisplayed);

    return _groupItem;
}

void WazeAlertRoadTrafficLayer::displayAlerts(const QSet<WazeAlert*>& alerts) {
    int radius = 20;

    /* remove alerts from the alerts that are displayed */
    QMutableHashIterator<WazeAlert*,GeometryGraphics*> it(_alertsToDisplay);
    while(it.hasNext()) {
        it.next();
        WazeAlert* wazeAlert = it.key();
        if(!alerts.contains(wazeAlert)) {
            // remove the Waze alert from the set of displayed alerts
            _alertsDisplayed->removeFromGroup(it.value());
            it.remove();
        }
    }

    // add the new alerts
    for(WazeAlert* wazeAlert : alerts) {
        if(_alertsToDisplay.contains(wazeAlert))
            continue;
        QPointF p = wazeAlert->pos;
        GeometryGraphics* item = new CircleGraphics(new Circle(p, radius));
        item->setPen(Qt::NoPen);
        QColor col(wazeAlertTypeToColor(wazeAlert->type));
        if(col.isValid()) {
            item->setBrush(QBrush(col));
        }
        _alertsToDisplay.insert(wazeAlert, item);
        _alertsDisplayed->addToGroup(item);
    }
}

bool WazeAlertRoadTrafficLayer::load(Loader* loader) {
    return static_cast<WazeAlertRoadTraffic*>(_shapefile)->open(loader);
}

void WazeAlertRoadTrafficLayer::onFeatureSelectedEvent(Geometry* geom, bool mod) {
    ShapefileFeature* shpFeature = _geometryShapefileFeatures.value(geom);
    RoadLinkWazeAlerts* rlwa = static_cast<WazeAlertRoadTraffic*>(_shapefile)->getRoadLinkAlerts(shpFeature->id);
    if(rlwa == nullptr) {
        qDebug() << "no road traffic data associated with this road link Waze data" << shpFeature->id;
        return;
    }
    qDebug() << "roadTrafficLinkWazeData" << rlwa->alerts.size();


    // update selected geometry
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

    if(_roadTrafficWazeDataExaminerPanel)
        _roadTrafficWazeDataExaminerPanel->onClosePanel();

    if(!_roadTrafficWazeDataExaminerPanel)
        _roadTrafficWazeDataExaminerPanel = new RoadTrafficWazeDataExaminerPanel(_parent,
                                                                                 static_cast<WazeAlertRoadTraffic*>(_shapefile),
                                                                                 this);

    if(_parent)
        _parent->addDockWidget(Qt::RightDockWidgetArea, _roadTrafficWazeDataExaminerPanel);

    _roadTrafficWazeDataExaminerPanel->show();
    _roadTrafficWazeDataExaminerPanel->showRoadTrafficLinkWazeData(rlwa);
}
