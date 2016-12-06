//
// Created by Benjamin Baron on 21/09/2016.
//

#include "road_traffic_layer.h"

QGraphicsItemGroup* RoadTrafficLayer::draw() {
    /* Draw the shapefile */
    ShapefileLayer::draw();

    qDebug() << "Shapefile drawn:" << _geometryGraphics.size();

    RoadTraffic* roadTraffic = static_cast<RoadTraffic*>(_shapefile);
    QHash<QString, RoadLink*>* roadLinks = roadTraffic->getRoadLinks();

    QMap<double, int> sortedRoadFlows;
    for(auto it = roadLinks->begin(); it != roadLinks->end(); ++it) {
        RoadTrafficDataAggregate* rtd = it.value()->getMeanRoadTrafficData();
        double roadFlow = rtd->getValue(RoadTrafficDataType::FlowRTDType, -1);
        if(roadFlow <= 0.0)
            continue;
        if(!sortedRoadFlows.contains(roadFlow))
            sortedRoadFlows.insert(roadFlow,1);
        else
            sortedRoadFlows[roadFlow]++;
    }

    qDebug() << roadLinks->size() << " road links " << sortedRoadFlows.size() << " sorted road flows";


    int cumSum = 0;
    for(auto it = sortedRoadFlows.begin(); it != sortedRoadFlows.end(); ++it) {
        int count = it.value();
        cumSum += count;
        sortedRoadFlows[it.key()] = cumSum;
    }

    for(auto it = _geometryShapefileFeatures.begin(); it != _geometryShapefileFeatures.end(); ++it) {
        int idx = it.value()->id;
        RoadLink* rl = roadTraffic->getRoadLink(idx);
        qDebug() << "index" << idx << "roadLink" << rl;
        if(rl == nullptr)
            continue;

        double roadFlow = rl->getMeanRoadTrafficData()->getValue(RoadTrafficDataType::FlowRTDType, -1);
        qDebug() << "\tRoad flow" << roadFlow;
        QColor color;
        if(sortedRoadFlows.contains(roadFlow)) {
            if(sortedRoadFlows.value(roadFlow) < (cumSum / 4.0))
                color = COL_LOW;
            else if(sortedRoadFlows.value(roadFlow) < (cumSum / 2.0))
                color = COL_MED_LOW;
            else if(sortedRoadFlows.value(roadFlow) < 3*(cumSum / 4.0))
                color = COL_MED_HIGH;
            else
                color = COL_HIGH;
        }
        if(color.isValid()) {
            qDebug() << "\tColor" << color;
            _geometryGraphics.value(it.key())->setUnselectedColor(color);
        }
    }

    return _groupItem;
}

bool RoadTrafficLayer::load(Loader* loader) {
    return static_cast<RoadTraffic*>(_shapefile)->open(loader);
}

void RoadTrafficLayer::onFeatureSelectedEvent(Geometry* geom, bool mod) {
    ShapefileFeature* shpFeature = _geometryShapefileFeatures.value(geom);
    RoadLink* rl = static_cast<RoadTraffic*>(_shapefile)->getRoadLink(shpFeature->id);
    if(rl == nullptr) {
        qDebug() << "no road traffic data associated with this link" << shpFeature->id;
        return;
    }
    qDebug() << "roadTrafficLinkData" << rl->toString();


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

    if(_roadTrafficExaminerPanel)
        _roadTrafficExaminerPanel->onClosePanel();

    if(!_roadTrafficExaminerPanel)
        _roadTrafficExaminerPanel = new RoadTrafficExaminerPanel(_parent);

    if(_parent)
        _parent->addDockWidget(Qt::RightDockWidgetArea, _roadTrafficExaminerPanel);

    _roadTrafficExaminerPanel->show();
    _roadTrafficExaminerPanel->showRoadTrafficLinkData(rl);
}
