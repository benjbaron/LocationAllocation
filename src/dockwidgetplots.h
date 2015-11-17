#ifndef DOCKWIDGETPLOTS_H
#define DOCKWIDGETPLOTS_H

#include <QDockWidget>
class QCustomPlot;

class SpatialStats;

namespace Ui {
class DockWidgetPlots;
}

class DockWidgetPlots : public QDockWidget
{
    Q_OBJECT

public:
    explicit DockWidgetPlots(QWidget *parent = 0, SpatialStats* spatialStats = 0);
    ~DockWidgetPlots();

    void showCellData(QPoint cell);
    void showLinkData(QPoint cell1, QPoint cell2);
    void plotFrequencies(QList<long long> frequencies, QCustomPlot* customPlot, long long bins);

private:
    Ui::DockWidgetPlots *ui;
    SpatialStats* _spatialStats;

};

#endif // DOCKWIDGETPLOTS_H
