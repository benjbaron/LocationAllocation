//
// Created by Benjamin Baron on 01/03/16.
//

#ifndef LOCALL_COMPUTEALLOCATIONLAYER_H
#define LOCALL_COMPUTEALLOCATIONLAYER_H


#include "layer.h"

// forward class declaration
class WeightedAllocationLayer;

class ComputeAllocationLayer : public Layer {
    Q_OBJECT
public:
    ComputeAllocationLayer(MainWindow* parent = 0, QString name = 0, ComputeAllocation* computeAllocation = nullptr):
            Layer(parent, name), _computeAllocation(computeAllocation) {}

    virtual bool load(Loader *loader);
    virtual QGraphicsItemGroup* draw();
    virtual void addMenuBar();

public slots:
    void computeAllocation();

private:
    ComputeAllocation* _computeAllocation;
    QList<WeightedAllocationLayer*> _allocationLayers;
};


#endif //LOCALL_COMPUTEALLOCATIONLAYER_H
