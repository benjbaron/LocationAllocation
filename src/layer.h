#ifndef LAYER_H
#define LAYER_H

#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QDebug>
#include <tuple>
#include <QMenu>
#include <ogrsf_frmts.h>
#include <qtconcurrentrun.h>

class MainWindow;
class Loader;

class Layer: public QObject
{
    Q_OBJECT
public:
    Layer(MainWindow* parent = 0, QString name = 0):
        _parent(parent), _name(name) {}

    void addGraphicsItem(QGraphicsItem* item) {
        _groupItem->addToGroup(item);
        _graphicsItems.append(item);
    }
    void setVisible(bool visible) { _groupItem->setVisible(visible); }
    bool isVisible() { return _groupItem->isVisible(); }
    QString getName() { return _name; }
    void setZValue(qreal value) { _groupItem->setZValue(value); }
    qreal getZValue() { return _groupItem->zValue(); }
    MainWindow* getParent() { return _parent; }
    virtual QString getInformation() { return "Layer: " + _name; }
    void showMenu() {
        if(_menu) {
            _menu->setVisible(true);
        }
    }
    void hideMenu() { if(_menu) _menu->setVisible(false); }

    // load whatever has to load
    virtual bool load(Loader* loader) = 0;

    QFuture<bool> run(Loader* loader) {
        return QtConcurrent::run(this, &Layer::load, loader);
    }

    // draw the layer objects
    virtual QGraphicsItemGroup* draw() = 0;

    // get the geometries
    virtual OGRGeometryCollection* getGeometry(long long startTime = 0, long long endTime = 0) { return NULL; }

protected:
    MainWindow* _parent;
    QString _name;
    QGraphicsItemGroup* _groupItem;
    QList<QGraphicsItem*> _graphicsItems;
    QMenu* _menu = 0;
};

#endif // LAYER_H
