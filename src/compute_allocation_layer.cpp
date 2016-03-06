#include "compute_allocation_layer.h"
#include "allocation_dialog.h"
#include "weighted_allocation_layer.h"

bool ComputeAllocationLayer::load(Loader* loader) {
    return false;
}

QGraphicsItemGroup *ComputeAllocationLayer::draw() {
    return nullptr;
}

void ComputeAllocationLayer::addMenuBar() {
    Layer::addMenuBar();
}

/*
 * Compute the location allocation
*/
void ComputeAllocationLayer::computeAllocation() {
    qDebug() << "Compute location allocation";

    AllocationDialog diag(_parent);
    if (diag.exec() == QDialog::Rejected) {
        return;
    }

    QHash<Geometry*, Allocation*> allocation; // resulting allocation
    AllocationParams params;
    diag.allocationParams(&params);
    loadWithProgressDialog(_computeAllocation, &ComputeAllocation::processAllocationMethod, &params, &allocation);

    // print the resulting allocation
    for(auto it = allocation.begin(); it != allocation.end(); ++it) {
        qDebug() << "candidate" << it.key()->getCenter().x()
        << "allocated" << it.value()->weight << "with" << it.value()->demands.size()
        << "demands" << "deleted" << it.value()->deletedCandidates.size();
    }

    // create a new layer
    QString layerName = "Location allocation";
    WeightedAllocationLayer* layer = new WeightedAllocationLayer(_parent, layerName, allocation);
    _allocationLayers.append(layer);

    // load the layer
    Loader loader;
    _parent->createLayer(layerName, layer, &loader);
}
