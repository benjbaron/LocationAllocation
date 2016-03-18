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

const QColor TRACE_SELECTED_COL  = QColor(231, 76, 60);
const QColor LINK_SELECTED_COL   = QColor(153, 14, 240);
const QColor LINK_UNSELECTED_COL = QColor(52, 73, 94);
const QColor TRACE_LINK_JOINT_COL = QColor(52, 152, 219);

const int    TRACE_SELECTED_WID  = 3;
const int    LINK_SELECTED_WID   = 5;
const int    LINK_UNSELECTED_WID = 2;



class Bounds {
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

class Geometry {
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

class Coord: public QPointF, public Geometry {
public:
    explicit Coord(double x, double y): QPointF(x,y) { }
    explicit Coord(const QPointF &c): QPointF(c) { }
    bool contains(double x, double y) { return x == (*this).x() && y == (*this).y(); }
    QPointF getCenter() { return *this; }
    Bounds getBounds() { return Bounds(x(),x(),y(),y()); }
    GeometryType getGeometryType() { return CoordType; }
    QString toString() { return "Coord ("+QString::number(x())+","+QString::number(y())+")"; }
};

class Circle: public Geometry {
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

class Cell: public QRectF, public Geometry {
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
class GeometryGraphics: public QObject, public QGraphicsItem {
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
    void mousePressEvent(QGraphicsSceneMouseEvent *event) {
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

class CellGraphics: public GeometryGraphics, public QGraphicsRectItem {
public:
    explicit CellGraphics(): GeometryGraphics(), QGraphicsRectItem() { }
    explicit CellGraphics(Cell* cell):
        GeometryGraphics(cell), QGraphicsRectItem(cell->x(), -1*cell->y(), cell->width(), -1*cell->height()) { }

    void setBrush(const QBrush & brush) { return QGraphicsRectItem::setBrush(brush); }
    void setPen(const QPen & pen) { return QGraphicsRectItem::setPen(pen); }
    QRectF boundingRect() const { return QGraphicsRectItem::boundingRect(); }
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) { return QGraphicsRectItem::paint(painter, option, widget); }
};

class CircleGraphics: public GeometryGraphics, public QGraphicsEllipseItem {
public:
    explicit CircleGraphics(): GeometryGraphics(), QGraphicsEllipseItem() { }
    explicit CircleGraphics(Circle* circle):
        GeometryGraphics(circle), QGraphicsEllipseItem(circle->getBounds().getQRectF().x(),
                                                       -1*circle->getBounds().getQRectF().y(),
                                                       circle->getBounds().getQRectF().width(),
                                                       -1*circle->getBounds().getQRectF().height()) { }

    void setBrush(const QBrush & brush) { return QGraphicsEllipseItem::setBrush(brush); }
    void setPen(const QPen & pen) { return QGraphicsEllipseItem::setPen(pen); }
    QRectF boundingRect() const { return QGraphicsEllipseItem::boundingRect(); }
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) { return QGraphicsEllipseItem::paint(painter, option, widget); }
};


class ArrowLineItem: public QObject, public QGraphicsLineItem {
    Q_OBJECT
public:
    ArrowLineItem(double x1, double y1, double x2, double y2, int nodeId, int linkId, double cutStart = 0, double cutEnd = 0):
            QGraphicsLineItem(x1,-1*y1,x2,-1*y2), _selectionOffset(10), _nodeId(nodeId), _linkId(linkId), _isTraceSelected(false), _isLinkSelected(false),
            _unselectedCol(LINK_UNSELECTED_COL), _selectedCol(LINK_SELECTED_COL), _traceSelectedCol(TRACE_SELECTED_COL),
            _unselectedWid(LINK_UNSELECTED_WID), _selectedWid(LINK_SELECTED_WID), _traceSelectedWid(TRACE_SELECTED_WID) {

        setFlags(QGraphicsItem::ItemIsSelectable);
        // create subline
        QLineF cLine = line();
        double factorStart = cutStart / cLine.length();
        double factorEnd   = cutEnd   / cLine.length();
        _subLine = QLineF(cLine.pointAt(0.0+factorStart), cLine.pointAt(1.0-factorEnd));
        createSelectionPolygon();
    }

    enum { Type = UserType + 1 };
    int type() const { return Type; }

    QRectF boundingRect() const {
        return _selectionPolygon.boundingRect();
    }

    QPainterPath shape() const {
        QPainterPath ret;
        ret.addPolygon(_selectionPolygon);
        return ret;
    }

    void setColors(QColor unselectedCol, QColor selectedCol, QColor traceSelectedCol) {
        _unselectedCol = unselectedCol;
        _selectedCol = selectedCol;
        _traceSelectedCol = traceSelectedCol;
    }

    void setWidths(int unselectedWid, int selectedWid, int traceSelectedWid) {
        _unselectedWid = unselectedWid;
        _selectedWid = selectedWid;
        _traceSelectedWid = traceSelectedWid;
    }

    void paint( QPainter* aPainter,
                const QStyleOptionGraphicsItem* aOption,
                QWidget* aWidget /*= nullptr*/ ) {
        Q_UNUSED( aOption );
        Q_UNUSED( aWidget );

        qreal lineAngle = _subLine.angle();
        QLineF head1 = _subLine;
        head1.setAngle(lineAngle+32);
        QLineF head2 = _subLine;
        head2.setAngle(lineAngle-32);

        QPen pen;
        pen.setCosmetic(true);
        pen.setStyle(Qt::SolidLine);

        int size;

        if(_isLinkSelected) {
            pen.setColor(_selectedCol);
            pen.setWidth(_selectedWid);
            size = _selectedWid;
        } else {
            if(_isTraceSelected) {
                pen.setColor(_traceSelectedCol);
                pen.setWidth(_traceSelectedWid);
                size = _traceSelectedWid;
            } else {
                pen.setColor(_unselectedCol);
                pen.setWidth(_unselectedWid);
                size = _unselectedWid;
            }
        }

        head1.setLength( size * 4 );
        head2.setLength( size * 4 );

        aPainter->setPen( pen );
        aPainter->drawLine( _subLine );
        aPainter->drawLine( head1 );
        aPainter->drawLine( head2 );
    }

    void setLabel(QString label) {
        _label = new QGraphicsTextItem(label, this);
        QPointF center = boundingRect().center();
        _label->setPos(center.x() - (_label->boundingRect().width()  / 2.0),
                       center.y() - (_label->boundingRect().height() / 2.0));
    }

    void setTraceSelected(bool selected) {
        _isTraceSelected = selected;
        update();
    }
    void setLinkSelected(bool selected) {
        _isLinkSelected = selected;
        update();
    }

signals:
    void mousePressedEvent(int, int, bool);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) {
        qDebug() << "[ArrowLineItem] Link clicked / Node" << _nodeId << "link" << _linkId << _isTraceSelected << _isLinkSelected;
        bool mod = (event->modifiers() == Qt::ShiftModifier);
        emit mousePressedEvent(_nodeId, _linkId, mod);
    }

private:
    QGraphicsTextItem* _label;
    QLineF _subLine;
    qreal _selectionOffset;
    QPolygonF _selectionPolygon;
    int _nodeId;
    int _linkId;
    bool _isTraceSelected;
    bool _isLinkSelected;

    QColor _unselectedCol, _selectedCol, _traceSelectedCol;
    int _unselectedWid, _selectedWid, _traceSelectedWid;

    void createSelectionPolygon() {
        QPolygonF nPolygon;
        qreal pi = 3.141592653589793238463;
        qreal radAngle = line().angle()* pi / 180;
        qreal dx = _selectionOffset * sin(radAngle);
        qreal dy = _selectionOffset * cos(radAngle);
        QPointF offset1 = QPointF(dx, dy);
        QPointF offset2 = QPointF(-dx, -dy);
        nPolygon << line().p1() + offset1
        << line().p1() + offset2
        << line().p2() + offset2
        << line().p2() + offset1;
        _selectionPolygon = nPolygon;
        update();
    }
};


#endif // GEOMETRIES_H

