#include "dockwidgetplots.h"
#include "ui_dockwidgetplots.h"

#include "spatialstats.h"
#include "constants.h"

DockWidgetPlots::DockWidgetPlots(QWidget *parent, SpatialStats *spatialStats) :
    QDockWidget(parent),
    ui(new Ui::DockWidgetPlots),
    _spatialStats(spatialStats)
{
    ui->setupUi(this);
}

DockWidgetPlots::~DockWidgetPlots()
{
    delete ui;
}

void DockWidgetPlots::showNodeData(Geometry* geom) {
    GeometryValue* value = _spatialStats->getValue(geom);

    int visitCount = value->visits.size();
    double interVisitAvg = value->interVisitDurationDist.getAverage();
    double travelTimeAvg = value->travelTimes.getAverage();
    double interVisitMed = value->interVisitDurationDist.getMedian();
    double travelTimeMed = value->travelTimes.getMedian();

    ui->label1->setText(QString("Inter-visit times distribution")
                        + "\nMean inter-visit " + QString::number(interVisitAvg)
                        + "\nMedian inter-visit " + QString::number(interVisitMed));
    qDebug() << "Inter-visit times distribution";
    value->interVisitDurationDist.plot(ui->plot1);

    ui->label2->setText(QString("Travel time distribution")
                        + "\nMean travel time " + QString::number(travelTimeAvg)
                        + "\nMedian travel time " + QString::number(travelTimeMed));
    qDebug() << "Travel time distribution";
    value->travelTimes.plot(ui->plot2);

    ui->label3->setText("Visit frequencies");
    qDebug() << "Visit Frequencies";
    plotFrequencies(value->visitFrequency, ui->plot3, 60);

    ui->label4->setText("Visit count " + QString::number(visitCount)
                        + "\nUnique nodes " + QString::number(value->nodes.size())
                        + "\nConnections " + QString::number(value->connections)
                        + "\nScore " + QString::number(visitCount / interVisitAvg)
                        + "\nScore (med) " + QString::number(value->medScore)
                        + "\nScore (avg) " + QString::number(value->avgScore)
                        + "\nIncoming Score (med) " + QString::number(value->medIncomingScore)
                        + "\nIncoming Score (avg) " + QString::number(value->avgIncomingScore));
}

void DockWidgetPlots::showLinkData(Geometry *geom1, Geometry *geom2)
{
    GeometryMatrixValue* value = _spatialStats->getValue(geom1, geom2);
    int visitCount = value->visits.size();
    double interVisitAvg = value->interVisitDurationDist.getAverage();
    double travelTimeAvg = value->travelTimeDist.getAverage();
    double interVisitMed = value->interVisitDurationDist.getMedian();
    double travelTimeMed = value->travelTimeDist.getMedian();

    ui->label1->setText(QString("Inter-visit times distribution")
                        + "\nMean inter-visit " + QString::number(interVisitAvg)
                        + "\nMedian inter-visit " + QString::number(interVisitMed));
    qDebug() << "Inter-visit times distribution";
    value->interVisitDurationDist.plot(ui->plot1);

    ui->label2->setText(QString("Travel time distribution")
                        + "\nMean Travel time " + QString::number(travelTimeAvg)
                        + "\nMedian travel time " + QString::number(travelTimeMed));
    qDebug() << "Travel time distribution";
    value->travelTimeDist.plot(ui->plot2);

    ui->label3->setText("Visit frequency");
    qDebug() << "Visit frequency";
    plotFrequencies(value->visitFrequency, ui->plot3, 60);

    auto val = _spatialStats->getValue(geom1, geom2);
    ui->label4->setText("count " + QString::number(val->visits.size())
                        + "\nScore " + QString::number(visitCount / interVisitAvg)
                        + "\nScore (med) " + QString::number(value->medScore)
                        + "\nScore (avg) " + QString::number(value->avgScore));

}

void DockWidgetPlots::plotFrequencies(QList<long long> frequencies, QCustomPlot* customPlot, long long bins) {
    // clear the other plottable graphs
    customPlot->clearPlottables();
    for(int i = 0; i < customPlot->plottableCount(); ++i) {
        customPlot->removePlottable(i);
    }
    long long min = MaxTime;
    long long max = 0;
    for(long long f : frequencies) {
        if(f < min) min = f;
        if(f > max) max = f;
    }
    int vectorSize = qMax(1, qCeil((double) (max - min) / bins));
    QVector<double> ticks(vectorSize), y(vectorSize);
    QVector<QString> labels(vectorSize);
    for(long long f : frequencies) {
        int i = qFloor((f - min) / (double) bins);
        if(f == max) i = ticks.size()-1;
        y[i] += 1;
    }

    double maxValue = 0.0;
    for(int i = 0; i < vectorSize; ++i) {
        if(y[i] > maxValue) maxValue = y[i];
//        qDebug() << i << labels[i] << y[i];
    }

    for(int i = 0; i < vectorSize; ++i) {
        ticks[i] = i+1;
        if(y[i] > 0)
            labels[i] = QString::number(i);
        else labels[i] = "";
    }


    QCPBars *bars = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    customPlot->addPlottable(bars);
    customPlot->xAxis->setAutoTicks(false);
    customPlot->xAxis->setAutoTickLabels(false);
    customPlot->xAxis->setTickVector(ticks);
    customPlot->xAxis->setTickVectorLabels(labels);
    customPlot->xAxis->setRange(0, ticks.size()+1);
    customPlot->yAxis->setRange(0, maxValue);

    bars->setData(ticks, y);
    customPlot->replot();
}

