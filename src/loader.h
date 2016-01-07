#ifndef LOADER_H
#define LOADER_H

#include <QtConcurrent>
#include <QString>
#include <QObject>

#include "mainwindow.h"

// forward declaration
//class Layer;
#include "layer.h"

class Loader: public QObject
{
    Q_OBJECT
public:
    Loader(Layer* layer = 0):
            _layer(layer) { }

    void load(Layer* layer) {
        _loadResult = layer->run(this);
        _layer->showMenu();
    }

signals:
    void loadProgressChanged(qreal);

protected:
    QFuture<bool> _loadResult;
    Layer* _layer;
};

#endif // LOADER_H
