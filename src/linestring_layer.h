//
// Created by Benjamin Baron on 08/02/16.
//

#ifndef LOCALL_LINESTRINGLAYER_H
#define LOCALL_LINESTRINGLAYER_H

#include "layer.h"

struct LineStringDisplay {
    LineStringDisplay(const QList<QPointF>& ls, int w = -1) :
            _linestring(ls), _width(w) {}
    QList<QPointF> _linestring;
    int _width;
};

class LineStringLayer: public Layer {
    Q_OBJECT
public:
    LineStringLayer(MainWindow* parent = 0, const QString& name = 0,
                    const QList<LineStringDisplay*>& ls = QList<LineStringDisplay*>()):
            Layer(parent, name), _linestrings(ls) { }
    QGraphicsItemGroup* draw();
    virtual bool load(Loader* loader);

private:
    QList<LineStringDisplay*> _linestrings;
};

#endif //LOCALL_LINESTRINGLAYER_H
