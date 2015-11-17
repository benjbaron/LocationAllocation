#ifndef CUSTOMSCENE_H
#define CUSTOMSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

class CustomScene : public QGraphicsScene
{
    Q_OBJECT
public:
    CustomScene() {}

    void addItems(QList<QGraphicsItem*> items) {
        for(auto const &x: items) {
            addItem(x);
        }
    }

signals:
    void mousePressedEvent(QGraphicsSceneMouseEvent *);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event) {
        QGraphicsScene::mouseMoveEvent(event);
        this->update();
    }

    void mousePressEvent(QGraphicsSceneMouseEvent * event) {
        QGraphicsScene::mousePressEvent(event);
        if(!event->isAccepted()) {
            emit mousePressedEvent(event);
        }
    }

};

#endif // CUSTOMSCENE_H
