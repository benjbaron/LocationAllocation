//
// Created by Benjamin Baron on 21/09/2016.
//

#include "road_traffic_layer.h"
#include "geometries.h"

QGraphicsItemGroup* RoadTrafficLayer::draw() {
    QColor c = Qt::red;
    QPen pen = QPen(c);
    pen.setCosmetic(true);
    pen.setWidth(SHAPEFILE_WID);

    _groupItem = new QGraphicsItemGroup();
    _groupItem->setHandlesChildEvents(false);

    QHash<QString, RoadLink*> roadLinks;
    _roadTraffic->getLinks(&roadLinks);

    QMap<double, int> sortedRoadFlows;
    for(auto it = roadLinks.begin(); it != roadLinks.end(); ++it) {
        RoadTrafficData* rtd = it.value()->getMeanRoadTrafficData();
        double roadFlow = rtd->flow;
        if(roadFlow <= 0.0)
            continue;
        if(!sortedRoadFlows.contains(roadFlow))
            sortedRoadFlows.insert(roadFlow,1);
        else
            sortedRoadFlows[roadFlow]++;
    }

    int cumSum = 0;
    for(auto it = sortedRoadFlows.begin(); it != sortedRoadFlows.end(); ++it) {
        int count = it.value();
        cumSum += count;
        sortedRoadFlows[it.key()] = cumSum;
    }

    int counter = 0;
    for(auto it = roadLinks.begin(); it != roadLinks.end(); ++it) {
        double roadFlow = it.value()->getMeanRoadTrafficData()->flow;
        QColor col;
        if(sortedRoadFlows.contains(roadFlow)) {
            if(sortedRoadFlows.value(roadFlow) < (cumSum / 4.0))
                col = COL_LOW;
            else if(sortedRoadFlows.value(roadFlow) < (cumSum / 2.0))
                col = COL_MED_LOW;
            else if(sortedRoadFlows.value(roadFlow) < 3*(cumSum / 4.0))
                col = COL_MED_HIGH;
            else
                col = COL_HIGH;
        }

        ArrowLineItem* link = new ArrowLineItem(
                it.value()->getLine().x1(),
                it.value()->getLine().y1(),
                it.value()->getLine().x2(),
                it.value()->getLine().y2(),
                counter,
                -1,
                static_cast<void*>(it.value()));

        connect(link, &ArrowLineItem::mousePressedEvent, this, &RoadTrafficLayer::onRoadTrafficLinkSelected);

        link->setPen(pen);
        if(col.isValid())
            link->setUnselectedColor(col);
        addGraphicsItem(link);
        _linkGraphics.append(link);

        counter++;
    }

    return _groupItem;
}

bool RoadTrafficLayer::load(Loader* loader) {
    bool ret = _roadTraffic->open(loader);
    return ret;
}

void RoadTrafficLayer::onRoadTrafficLinkSelected(int nodeId, int linkId, bool mod, void* ptr) {
    RoadLink* rl = static_cast<RoadLink*>(ptr);
    qDebug() << "nodeId" << nodeId << "roadTrafficLinkData" << rl->toString();


    if(_selectedRoadLinkId != -1) {
        // restore the normal attributes
        _linkGraphics.at(_selectedRoadLinkId)->setLinkSelected(false);
    }

    if(_selectedRoadLinkId != nodeId) {
        _selectedRoadLinkId = nodeId;
        _linkGraphics.at(_selectedRoadLinkId)->setLinkSelected(true);
    } else {
        _selectedRoadLinkId = -1;
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
