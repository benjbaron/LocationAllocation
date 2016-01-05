#ifndef LAYERPANEL_H
#define LAYERPANEL_H

#include <QDockWidget>
#include <QDebug>
#include <QDropEvent>
#include <QListWidget>
#include <QMimeData>
#include <QStyledItemDelegate>

#include "layer.h"

Q_DECLARE_METATYPE(Layer*)

class ListWidget: public QListWidget
{
    Q_OBJECT
public:
    ListWidget(QWidget* parent = 0):
        QListWidget(parent)
    {
        setDragDropMode(QAbstractItemView::DragDrop);
        setDefaultDropAction(Qt::MoveAction);
        setAlternatingRowColors(true);
    }

signals:
    void orderChanged(int, int);

protected:
    void dragEnterEvent(QDragEnterEvent *event) {
        _lastIndex = this->currentIndex().row();
        QListWidget::dragEnterEvent(event);
    }

    void dropEvent(QDropEvent* event) {
        QListWidget::dropEvent(event);
        int newIndex = this->currentIndex().row();
        emit orderChanged(_lastIndex, newIndex);
    }

private:
    int _lastIndex;

};

namespace Ui {
class LayerPanel;
}

class LayerPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit LayerPanel(MainWindow* parent = 0, QList<Layer*>* layers = 0);
    ~LayerPanel();

    void addLayer(Layer* layer);

signals:
    void closedEvent();
    void reorderEvent(int,int);
    
protected:
    void closeEvent(QCloseEvent* event) {
        Q_UNUSED(event)

        emit closedEvent();
    }

private:
    Ui::LayerPanel *ui;
    QList<Layer*>* _layers;
    ListWidget* _listWidget;
};

#endif // LAYERPANEL_H
