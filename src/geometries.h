#ifndef GEOMETRIES_H
#define GEOMETRIES_H

#include <QPointF>
#include <QRectF>
#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QPainter>
#include <QDebug>
#include <QSet>
#include <cmath>
#include "constants.h"

static double euclideanDistance(double x1, double y1, double x2, double y2) {
    return qSqrt(qPow(x1 - x2,2) + qPow(y1 - y2,2));
}
static double euclideanDistance(QPointF a, QPointF b) {
    return euclideanDistance(a.x(), a.y(), b.x(), b.y());
}

class Bounds {
public:
    explicit Bounds(double x1, double x2, double y1, double y2):
            minX(qMin(x1,x2)),
            maxX(qMax(x1,x2)),
            minY(qMin(y1,y2)),
            maxY(qMax(y1,y2)) {}

    explicit Bounds(const QRectF& r):
            Bounds(r.left(), r.right(), r.top(), r.bottom()) { }

    void getMinMax(double (&min)[2], double (&max)[2]) {
        min[0] = minX; min[1] = minY;
        max[0] = maxX; max[1] = maxY;
    }
    QRectF getQRectF() {
        return QRectF(minX,minY,(maxX-minX),(maxY-minY));
    }
    QPointF getTopLeft() { return QPointF(minX,minY); }
    QPointF getBottomRight() { return QPointF(maxX,maxY); }
    QPointF getCenter() { return QPointF((maxX-minX)/2,(maxY-minY)/2); }
    QString toString() {
        QString str = "Bounds(";
        str += QString::number(minX) + ", "  + QString::number(maxX) +
                ", " + QString::number(minY) + ", " + QString::number(maxY) + ")";
        return str;
    }

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
    QPointF getCenter() { return _center; }
    Bounds getBounds() {
        return _bounds;
    }
    GeometryType getGeometryType() { return _geometryType; }
    double distance(Geometry* g) {
        return euclideanDistance(getCenter(), g->getCenter());
    }
    virtual QString toString() = 0;

protected:
    Geometry(Bounds b, QPointF c, GeometryType g) :
            _bounds(b), _center(c), _geometryType(g) {}

    Bounds _bounds;
    QPointF _center;
    GeometryType _geometryType;
};

class Coord: public QPointF, public Geometry {
public:
    explicit Coord(double x, double y):
            QPointF(x,y),
            Geometry(Bounds(x,x,y,y), QPointF(x,y), GeometryType::CoordGeometryType) { }

    explicit Coord(const QPointF& c): Coord(c.x(), c.y()) { }

    bool contains(double x, double y) { return x == (*this).x() && y == (*this).y(); }
    QString toString() { return "Coord ("+QString::number(x())+","+QString::number(y())+")"; }
};

class Circle: public Geometry {
public:
    explicit Circle(double x, double y, double radius):
            _radius(radius),
            Geometry(Bounds(x-radius,x+radius,y-radius,y+radius), QPointF(x,y), GeometryType::CircleGeometryType) { }

    explicit Circle(QPointF center, double radius):
            Circle(center.x(), center.y(), radius) {}

    explicit Circle(const Circle& c):
            Circle(c._center, c._radius) { }

    bool contains(double x, double y) {
        double distance = euclideanDistance(x,y,_center.x(),_center.y());
        return std::islessequal(distance,_radius);
    }
    QString toString() { return "Circle center ("+QString::number(_center.x())+","+QString::number(_center.y())+") radius" + QString::number(_radius); }

private:
    QPointF _center;
    double _radius;
};

class Cell: public QRectF, public Geometry {
public:
    explicit Cell(const QRectF& rect):
            QRectF(rect),
            Geometry(Bounds(rect), rect.center(), GeometryType::CellGeometryType) { }

    explicit Cell(double x, double y, double cellSize):
            Cell(QRectF(x,y,cellSize, cellSize)) {}

    explicit Cell(QPointF topLeft, double cellSize):
            Cell(QRectF(topLeft,QSizeF(cellSize, cellSize))) { }

    explicit Cell(const Cell& c):
            Cell(c.x(), c.y(), c.width()) { }

    bool contains(double x, double y) { return QRectF::contains(x,y); }
    QString toString() { return "Cell topleft ("+ QString::number(topLeft().x()) +", "+QString::number(topLeft().y())+") width " + QString::number(width()); }
};

class Polygon: public QPolygonF, public Geometry {
public:
    explicit Polygon(const QPolygonF& polygon):
            QPolygonF(polygon),
            Geometry(Bounds(polygon.boundingRect()), polygon.boundingRect().center(), GeometryType::PolygonGeometryType) { }

    explicit Polygon() :
            Polygon(QPolygonF()) { }

    explicit Polygon(const QVector<QPointF>& v) :
            Polygon(QPolygonF(v)) { }

    explicit Polygon(const QRectF& r) :
            Polygon(QPolygonF(r)) { }

    bool contains(double x, double y) { return QPolygonF::containsPoint(QPointF(x,y), Qt::WindingFill); }
    QString toString() { return "Polygon with " + QString::number(size()) + " points" ; }
};

class Path: public QPainterPath, public Geometry {
public:
    explicit Path(const QPainterPath& p):
            QPainterPath(p),
            Geometry(Bounds(p.boundingRect()), p.boundingRect().center(), GeometryType::PathGeometryType) {}

    explicit Path() :
            Path(QPainterPath()) {}

    explicit Path(const QPointF& startPoint) :
            Path(QPainterPath(startPoint)) { }

    explicit Path(const QLineF& line) :
            Path(line.p1()) { lineTo(line.p2()); }

    bool contains(double x, double y) { return QPainterPath::contains(QPointF(x,y)); }
    QString toString() { return "Path" ; }
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

    void selected(bool selected) {
        _isSelected = selected;
        QGraphicsItem::update();
    }
    void setSelectedColor(QColor color) {
        _selectedColor = color;
    }
    void setUnselectedColor(QColor color) {
        _unselectedColor = color;
    }
    void setSelectedWidth(int width) {
        _selectedWidth = width;
    }
    void setUnselectedWidth(int width) {
        _unselectedWidth = width;
    }

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
protected:
    Geometry* _geom;
    bool _isSelected = false;
    QColor _selectedColor = LINK_SELECTED_COL;
    QColor _unselectedColor = LINK_UNSELECTED_COL;
    int _selectedWidth = LINK_SELECTED_WID;
    int _unselectedWidth = LINK_UNSELECTED_WID;
};

class CellGraphics: public GeometryGraphics, public QGraphicsRectItem {
public:
    explicit CellGraphics(): GeometryGraphics(), QGraphicsRectItem() { }
    explicit CellGraphics(Cell* cell):
        GeometryGraphics(cell), QGraphicsRectItem(cell->x(), -1*(cell->y()+cell->height()), cell->width(), cell->height()) { }

    void setBrush(const QBrush & brush) { return QGraphicsRectItem::setBrush(brush); }
    void setPen(const QPen & pen) { return QGraphicsRectItem::setPen(pen); }
    QRectF boundingRect() const { return QGraphicsRectItem::boundingRect(); }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) {
        return QGraphicsRectItem::paint(painter, option, widget);
    }
};

class CircleGraphics: public GeometryGraphics, public QGraphicsEllipseItem {
public:
    explicit CircleGraphics(): GeometryGraphics(), QGraphicsEllipseItem() { }
    explicit CircleGraphics(Circle* circle):
        GeometryGraphics(circle), QGraphicsEllipseItem(circle->getBounds().getQRectF().x(),
                                                       -1*(circle->getBounds().getQRectF().y() + circle->getBounds().getQRectF().height()),
                                                       circle->getBounds().getQRectF().width(),
                                                       circle->getBounds().getQRectF().height()) { }

    void setBrush(const QBrush & brush) { return QGraphicsEllipseItem::setBrush(brush); }
    void setPen(const QPen & pen) { return QGraphicsEllipseItem::setPen(pen); }
    QRectF boundingRect() const { return QGraphicsEllipseItem::boundingRect(); }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) {
        return QGraphicsEllipseItem::paint(painter, option, widget);
    }
};

class PolygonGraphics: public GeometryGraphics, public QGraphicsPolygonItem {
public:
    explicit PolygonGraphics() : GeometryGraphics(), QGraphicsPolygonItem() { }
    explicit PolygonGraphics(Polygon* polygon):
            GeometryGraphics(polygon), QGraphicsPolygonItem() {
        Polygon p;
        for(QPointF pt : *polygon) {
            p.append(QPointF(pt.x(), -1*pt.y()));
        }
        setPolygon(p);
    }

    void setBrush(const QBrush & brush) { return QGraphicsPolygonItem::setBrush(brush); }
    void setPen(const QPen & pen) { return QGraphicsPolygonItem::setPen(pen); }
    QRectF boundingRect() const { return QGraphicsPolygonItem::boundingRect(); }
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) {
        return QGraphicsPolygonItem::paint(painter, option, widget);
    }
};

class ArrowPathGraphics: public GeometryGraphics, public QGraphicsPathItem {
    Q_OBJECT
public:
    explicit ArrowPathGraphics() : GeometryGraphics(), QGraphicsPathItem() {}
    explicit ArrowPathGraphics(Path* path, void* data = nullptr) :
            GeometryGraphics(path), QGraphicsPathItem(*path), _data(data) {

        Path p(*path);
        for(int i = 0; i < path->elementCount(); ++i) {
            p.setElementPositionAt(i, path->elementAt(i).x, -1*path->elementAt(i).y);
        }
        setPath(p);

        GeometryGraphics::setFlags(QGraphicsItem::ItemIsSelectable);
        createSelectionPolygon();
    }

    void setBrush(const QBrush& brush) { return QGraphicsPathItem::setBrush(brush); }
    void setPen(const QPen& pen) { return QGraphicsPathItem::setPen(pen); }

    QRectF boundingRect() const { return _selectionPolygon.boundingRect(); }
    QPainterPath shape() const { return _selectionPolygon; }

    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget) {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        QLineF cLine(path().elementAt(path().elementCount()-1), path().elementAt(path().elementCount()-2));

        qreal lineAngle = cLine.angle();
        QLineF head1 = cLine;
        head1.setAngle(lineAngle+32);
        QLineF head2 = cLine;
        head2.setAngle(lineAngle-32);

        QPen pen;
        pen.setCosmetic(true);
        pen.setStyle(Qt::SolidLine);

        int size;

        if(_isSelected) {
            pen.setColor(_selectedColor);
            pen.setWidth(_selectedWidth);
            size = _selectedWidth;
        } else {
            pen.setColor(_unselectedColor);
            pen.setWidth(_unselectedWidth);
            size = _unselectedWidth;
        }

        head1.setLength( size + 4 );
        head2.setLength( size + 4 );

        painter->setPen(pen);
        painter->drawLine( head1 );
        painter->drawLine( head2 );
        painter->drawPath(path());
    }

private:
    void* _data;
    QPainterPath _selectionPolygon;

    void createSelectionPolygon() {
        QPainterPathStroker stroker;
        stroker.setWidth(20);
        stroker.setJoinStyle(Qt::MiterJoin);
        _selectionPolygon = stroker.createStroke(path()).simplified();
    }
};

// See here: http://www.walletfox.com/course/customqgraphicslineitem.php
class ArrowLineItem: public QObject, public QGraphicsLineItem {
    Q_OBJECT
public:
    ArrowLineItem(double x1, double y1, double x2, double y2, int nodeId, int linkId, void* ptr = nullptr, double cutStart = 0, double cutEnd = 0):
            QGraphicsLineItem(x1,-1*y1,x2,-1*y2), _selectionOffset(10), _nodeId(nodeId), _linkId(linkId), _ptr(ptr), _isTraceSelected(false), _isLinkSelected(false),
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

    ArrowLineItem(QLineF line, int nodeId, int linkId, void* ptr = nullptr, double cutStart = 0, double cutEnd = 0) {}

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

    void setUnselectedColor(QColor unselectedCol) {
        _unselectedCol = unselectedCol;
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

        head1.setLength( size + 1 );
        head2.setLength( size + 1 );

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
    void mousePressedEvent(int, int, bool, void*);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) {
        qDebug() << "[ArrowLineItem] Link clicked / Node" << _nodeId << "link" << _linkId << _isTraceSelected << _isLinkSelected;
        bool mod = (event->modifiers() == Qt::ShiftModifier);
        if(!mod) {
            event->ignore();
        } else {
            emit mousePressedEvent(_nodeId, _linkId, mod, _ptr);
        }
    }

private:
    QGraphicsTextItem* _label;
    QLineF _subLine;
    qreal _selectionOffset;
    QPolygonF _selectionPolygon;
    int _nodeId;
    int _linkId;
    void* _ptr;
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

