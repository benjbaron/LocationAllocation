#ifndef LAYER_H
#define LAYER_H

#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QDebug>
#include <tuple>
#include <QMenu>
#include <ogrsf_frmts.h>
#include <qtconcurrentrun.h>

#include "compute_allocation.h"
#include "progress_dialog.h"
#include "mainwindow.h"
#include "loader.h"

class MainWindow;

class Layer: public QObject {

    Q_OBJECT
public:
    Layer(MainWindow* parent = 0, QString name = 0):
        _parent(parent), _name(name) {}

    void addGraphicsItem(QGraphicsItem* item) {
        _groupItem->addToGroup(item);
    }
    void setVisible(bool visible) { _groupItem->setVisible(visible); }
    bool isVisible() { return _groupItem->isVisible(); }

    QString getName() { return _name; }
    void setZValue(qreal value) { _groupItem->setZValue(value); }
    qreal getZValue() { return _groupItem->zValue(); }
    MainWindow* getParent() { return _parent; }
    virtual QString getInformation() { return "Layer: " + _name; }

    template<typename Class, typename ...A1, typename ...A2>
    QFuture<bool> loadWithProgressDialog(Class* object, bool (Class::*f) (A1 ...), A2&&... args) {
        Loader l;
        ProgressDialog p(_parent);
        connect(&l, &Loader::loadProgressChanged, &p, &ProgressDialog::updateProgress);
        QFuture<bool> future = l.load(object, f, &l, std::forward<A2>(args)...);
        p.exec();
        return future;
    }

    // menu bar methods
    void showMenu() {
        if(_menu) {
            _menu->setVisible(true);
        }
    }
    void hideMenu() { if(_menu) _menu->setVisible(false); }
    virtual void addMenuBar() { }

    // load whatever has to load
    virtual bool load(Loader* loader) = 0;

    // run the layer in an other QtConcurrent thread
    QFuture<bool> run(Loader* loader) {
        return QtConcurrent::run(this, &Layer::load, loader);
    }

    // draw the layer objects
    virtual QGraphicsItemGroup* draw() = 0;

protected:
    MainWindow* _parent;
    QString _name;
    QGraphicsItemGroup* _groupItem;
    QMenu* _menu = 0;
};

#endif // LAYER_H
