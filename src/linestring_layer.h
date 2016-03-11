//
// Created by Benjamin Baron on 08/02/16.
//

#ifndef LOCALL_LINESTRINGLAYER_H
#define LOCALL_LINESTRINGLAYER_H

#include "layer.h"

class LineStringLayer: public Layer {
public:
    LineStringLayer(MainWindow* parent = 0, const QString& name = 0, const QList<QList<QPointF>*>& ls = QList<QList<QPointF>*>()):
            Layer(parent, name), _linestrings(ls) { }
    QGraphicsItemGroup* draw();
    virtual bool load(Loader* loader);

private:
    QList<QList<QPointF>*> _linestrings;
};

#endif //LOCALL_LINESTRINGLAYER_H
