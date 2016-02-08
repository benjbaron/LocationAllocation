#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <qgraphicsitem.h>
#include <qpixmapcache.h>


class GraphicsView: public QGraphicsView
{
public:
    GraphicsView(QGraphicsScene* scene, QWidget* parent) :
        QGraphicsView(scene, parent)
    {
        _scale = 1;
//        setDragMode(ScrollHandDrag);
    }

    void wheelEvent(QWheelEvent* e)
    {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        // Scale the view / do the zoom
        double scaleFactor = 1.15;
        if(e->delta() > 0) {
            // Zoom in
            scale(scaleFactor, scaleFactor);

        } else {
            // Zooming out
             scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }
    }

private:
    double _scale;
};

#endif // GRAPHICSVIEW_H
