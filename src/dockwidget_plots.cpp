#include "dockwidget_plots.h"
#include "ui_dockwidget_plots.h"

#include "spatial_stats.h"

DockWidgetPlots::DockWidgetPlots(QWidget* parent, SpatialStats* spatialStats) :
    QDockWidget(parent),
    ui(new Ui::DockWidgetPlots),
    _spatialStats(spatialStats) {
        ui->setupUi(this);
}

DockWidgetPlots::~DockWidgetPlots() {
    delete ui;
}

void DockWidgetPlots::showNodeData(Geometry* geom) {
    GeometryValue* value;
    _spatialStats->getValue(&value, geom);

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

void DockWidgetPlots::showLinkData(Geometry *geom1, Geometry *geom2) {
    GeometryMatrixValue* value;
    _spatialStats->getValue(&value,geom1, geom2);
    int visitCount       = value->visits.size();
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

    ui->label4->setText("count " + QString::number(visitCount)
                        + "\nScore " + QString::number(visitCount / interVisitAvg)
                        + "\nScore (med) " + QString::number(value->medScore)
                        + "\nScore (avg) " + QString::number(value->avgScore));

}
