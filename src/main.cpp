#include "mainwindow.h"
#include "proj_factory.h"
#include "gtfs_layer.h"
#include "geometry_index.h"
#include "spatial_stats.h"
#include <QApplication>
#include <qcommandlineparser.h>

QCoreApplication* createApplication(int &argc, char *argv[]) {
    for (int i = 1; i < argc; ++i)
        if (!qstrcmp(argv[i], "--no-gui"))
            return new QCoreApplication(argc, argv);
    return new QApplication(argc, argv);
}

int main(int argc, char *argv[]) {

    QScopedPointer<QCoreApplication> a(createApplication(argc, argv));

    if (qobject_cast<QApplication *>(a.data())) {

        // start GUI version...
        qDebug() << "GUI version";
        MainWindow w;
        w.show();
        return a->exec();

    }  else {

        // start terminal version...

        QCommandLineParser parser;

        parser.setApplicationDescription("LocAll");
        parser.addHelpOption();
        parser.addVersionOption();

        QCommandLineOption noguiOption(QStringList() << "no-gui", "Does not display the gui, run in command line.");
        parser.addOption(noguiOption);
        QCommandLineOption gtfsOption(QStringList() << "gtfs", "Load a GTFS directory.", "dir", QString());
        parser.addOption(gtfsOption);
        QCommandLineOption traceOption(QStringList() << "trace", "Load a trace file.", "file", QString());
        parser.addOption(traceOption);
        QCommandLineOption traceDirOption(QStringList() << "trace-dir", "Load a trace directory.", "dir", QString());
        parser.addOption(traceDirOption);
        QCommandLineOption pointOption(QStringList() << "points", "Use a points file.", "file", QString());
        parser.addOption(pointOption);
        QCommandLineOption serverOption(QStringList() << "server", "Run the location allocation API server.");
        parser.addOption(serverOption);
        QCommandLineOption projInOption(QStringList() << "proj-in", "Choose the input projection.", "projection",
                                        QString());
        parser.addOption(projInOption);
        QCommandLineOption projOutOption(QStringList() << "proj-out", "Choose the output projection.", "projection",
                                         QString());
        parser.addOption(projOutOption);
        QCommandLineOption samplingOption(QStringList() << "sampling", "Sampling for the spatial stats.", "value",
                                          "-1");
        parser.addOption(samplingOption);
        QCommandLineOption startTimeOption(QStringList() << "starttime", "startTime for the spatial stats.", "value",
                                           "-1");
        parser.addOption(startTimeOption);
        QCommandLineOption endTimeOption(QStringList() << "endtime", "endTime for the spatial stats.", "value", "-1");
        parser.addOption(endTimeOption);
        QCommandLineOption cellSizeOption(QStringList() << "cell-size", "cell size for the spatial stats.", "value",
                                          "-1");
        parser.addOption(cellSizeOption);

        // Process the actual command line arguments given by the user
        parser.process(*(a.data()));

        qDebug() << "parser options" << parser.optionNames();

        if (parser.isSet(projInOption) && parser.isSet(projOutOption)) {
            QString projIn = parser.value(projInOption);
            QString projOut = parser.value(projOutOption);
            ProjFactory::getInstance().setProj(projIn, projOut);
        }
        double sampling = -1;
        double startTime = -1;
        double endTime = -1;
        double cellSize = -1;
        if (parser.isSet(samplingOption))
            sampling = parser.value(samplingOption).toDouble();
        if (parser.isSet(startTimeOption))
            startTime = parser.value(startTimeOption).toDouble();
        if (parser.isSet(endTimeOption))
            endTime = parser.value(endTimeOption).toDouble();
        if (parser.isSet(cellSizeOption))
            cellSize = parser.value(cellSizeOption).toDouble();

        GeometryType pointsType = NoneGeometryType;
        QString pointsFile = QString();
        if (parser.isSet(pointOption)) {
            pointsType = CircleGeometryType;
            pointsFile = parser.value(pointOption);
        }

        Trace* trace = nullptr;

        Loader l;
        QFuture<bool> future;
        ProgressConsole p;
        QObject::connect(&l, &Loader::loadProgressChanged, &p, &ProgressConsole::updateProgress);


        if (parser.isSet(gtfsOption)) {
            QString gtfsPath = parser.value(gtfsOption);
            qDebug() << "load GTFS directory" << gtfsPath << "...";

            trace = new GTFSTrace(gtfsPath, true);
            future = l.load(static_cast<GTFSTrace*>(trace), &GTFSTrace::openTrace, &l);
            future.result();

        } else if (parser.isSet(traceOption)) {
            QString tracePath = parser.value(traceOption);
        } else if (parser.isSet(traceDirOption)) {
            QString traceDir = parser.value(traceDirOption);
        }

        // check if the trace is not null
        if(!trace) a->exit(0);

        qDebug() << "Compute ogrGeometry index";
        GeometryIndex* geometryIndex = trace->makeGeometryIndex(sampling, startTime, endTime,
                                                                cellSize, pointsType, pointsFile);

        qDebug() << "compute spatial stats";
        SpatialStats* spatialStats = new SpatialStats(trace,
                                                      (int) sampling, (long long) startTime, (long long) endTime,
                                                      geometryIndex);
        future = l.load(spatialStats,&SpatialStats::computeStats, &l);
        future.result();

        ComputeAllocation* computeAllocation = new ComputeAllocation(spatialStats);
        RESTServer server(8, nullptr, computeAllocation);
        server.setTimeOut(500);
        server.listen(8080);

        // see http://stackoverflow.com/questions/21911526/qtquick-animation-freezing-on-list-and-open-serial-ports/21913457#21913457

        qDebug() << "Launching REST server";

        return a->exec();
    }
}
