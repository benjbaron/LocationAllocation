#ifndef LOADER_H
#define LOADER_H

#include <QtConcurrent>
#include <QString>
#include <QObject>

#include "mainwindow.h"

// forward declaration
class Layer;

class Loader: public QObject
{
    Q_OBJECT
public:
    Loader(MainWindow* parent = 0, QString filename = 0, Layer* layer = 0):
        _parent(parent), _filename(filename), _layer(layer)
    {
        _projOut = parent->getProjOut();
    }

    void load() {
        _loadResult = QtConcurrent::run(this, &Loader::concurrentLoad);
    }

signals:
    void loadProgressChanged(qreal);

protected:
    virtual bool concurrentLoad() = 0;

    MainWindow* _parent;
    QString _filename;
    QFuture<bool> _loadResult;
    Layer* _layer;
    QString _projOut;
};

#endif // LOADER_H
