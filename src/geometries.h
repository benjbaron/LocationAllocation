#ifndef GEOMETRIES_H
#define GEOMETRIES_H

#include <QPointF>
#include <QRectF>
#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <cmath>

#include "utils.h"

enum GeometryType { NoneType, CircleType, CellType, CoordType };

class Bounds
{
public:
    explicit Bounds(double x1, double x2, double y1, double y2) {
        minX = qMin(x1,x2);
        maxX = qMax(x1,x2);
        minY = qMin(y1,y2);
        maxY = qMax(y1,y2);
    }
    void getMinMax(double (&min)[2], double (&max)[2]) {
        min[0] = minX; min[1] = minY;
        max[0] = maxX; max[1] = maxY;
    }
    QRectF getQRectF() {
        return QRectF(minX,minY,(maxX-minX),(maxY-minY));
    }
    QPointF getTopLeft() { return QPointF(minX,minY); }
    QPointF getBottomRight() { return QPointF(maxX,maxY); }

private:
    double minX;
    double maxX;
    double minY;
    double maxY;
};

class Geometry
{
public:
    virtual ~Geometry() { }

    virtual bool contains(double x, double y) = 0;
    bool contains(QPointF p) { return contains(p.x(), p.y()); }
    virtual QPointF getCenter() = 0;
    virtual Bounds getBounds() = 0;
    virtual GeometryType getGeometryType() = 0;
    double distance(Geometry* g) {
        return euclideanDistance(getCenter(), g->getCenter());
    }
    virtual QString toString() = 0;
};

class Coord: public QPointF, public Geometry
{
public:
    explicit Coord(double x, double y): QPointF(x,y) { }
    explicit Coord(const QPointF &c): QPointF(c) { }
    bool contains(double x, double y) { return x == (*this).x() && y == (*this).y(); }
    QPointF getCenter() { return *this; }
    Bounds getBounds() { return Bounds(x(),x(),y(),y()); }
    GeometryType getGeometryType() { return CoordType; }
    QString toString() { return "Coord ("+QString::number(x())+","+QString::number(y())+")"; }
};

class Circle: public Geometry
{
public:
    explicit Circle(QPointF center, double radius): _center(center), _radius(radius) { }
    explicit Circle(double x, double y, double radius): _center(x,y), _radius(radius) { }
    explicit Circle(const Circle &c): _center(c._center), _radius(c._radius) { }
    bool contains(double x, double y) {
        double distance = euclideanDistance(x,y,_center.x(),_center.y());
        return std::islessequal(distance,_radius);
    }
    QPointF getCenter() { return _center; }
    Bounds getBounds() { return Bounds(_center.x()-_radius,_center.x()+_radius,_center.y()-_radius,_center.y()+_radius); }
    GeometryType getGeometryType() { return CircleType; }
    QString toString() { return "Circle center ("+QString::number(_center.x())+","+QString::number(_center.y())+") radius" + QString::number(_radius); }

private:
    QPointF _center;
    double _radius;
};

class Cell: public QRectF, public Geometry
{
public:
    explicit Cell(QPointF topLeft, double cellSize): QRectF(topLeft, QSizeF(cellSize,cellSize)) { }
    explicit Cell(double x, double y, double cellSize): QRectF(x,y,cellSize,cellSize) { }
    explicit Cell(const Cell &c): QRectF(c) { }
    bool contains(double x, double y) { return QRectF::contains(x,y); }
    QPointF getCenter() { return QRectF::center(); }
    Bounds getBounds() { return Bounds(QRectF::left(),QRectF::right(),QRectF::top(),QRectF::bottom()); }
    GeometryType getGeometryType() { return CellType; }
    QString toString() { return "Cell topleft ("+ QString::number(topLeft().x()) +", "+QString::number(topLeft().y())+") width " + QString::number(width()); }
};


/* Corresponding Graphics classes */
class GeometryGraphics: public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit GeometryGraphics(): QGraphicsItem(), _geom(NULL) { }
    explicit GeometryGraphics(Geometry* geom): QGraphicsItem(), _geom(geom) { }

    virtual void setBrush(const QBrush & brush) = 0;
    virtual void setPen(const QPen & pen) = 0;

signals:
    void mousePressedEvent(Geometry*, bool);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        if(event->button() == Qt::LeftButton) {
            event->accept();
            emit mousePressedEvent(_geom, (event->modifiers() == Qt::ShiftModifier));
        } else {
            event->ignore();
        }
    }
private:
    Geometry* _geom;
};

class CellGraphics: public GeometryGraphics, public QGraphicsRectItem
{
public:
    explicit CellGraphics(): GeometryGraphics(), QGraphicsRectItem() { }
    explicit CellGraphics(Cell* cell):
        GeometryGraphics(cell), QGraphicsRectItem(*cell) { }

    void setBrush(const QBrush & brush) { return QGraphicsRectItem::setBrush(brush); }
    void setPen(const QPen & pen) { return QGraphicsRectItem::setPen(pen); }
    QRectF boundingRect() const { return QGraphicsRectItem::boundingRect(); }
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) { return QGraphicsRectItem::paint(painter, option, widget); }
};

class CircleGraphics: public GeometryGraphics, public QGraphicsEllipseItem
{
public:
    explicit CircleGraphics(): GeometryGraphics(), QGraphicsEllipseItem() { }
    explicit CircleGraphics(Circle* circle):
        GeometryGraphics(circle), QGraphicsEllipseItem(circle->getBounds().getQRectF()) { }

    void setBrush(const QBrush & brush) { return QGraphicsEllipseItem::setBrush(brush); }
    void setPen(const QPen & pen) { return QGraphicsEllipseItem::setPen(pen); }
    QRectF boundingRect() const { return QGraphicsEllipseItem::boundingRect(); }
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) { return QGraphicsEllipseItem::paint(painter, option, widget); }
};

#endif // GEOMETRIES_H

