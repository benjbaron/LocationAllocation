#include "rest_server.h"

#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"
#include "threadlist.h"

#include "loader.h"
#include "geometries.h"

using namespace qhttp::server;
QAtomicInt  gHandledConnections;
///////////////////////////////////////////////////////////////////////////////

class ClientHandler : public QObject {
Q_OBJECT

public:
    explicit ClientHandler(ComputeAllocation* computeAllocation): _computeAllocation(computeAllocation) { }

    void setup(QThread *th) {
        moveToThread(th);

        QObject::connect(th, &QThread::finished, [this]() {
            deleteLater();
        });
    }

signals:
    void disconnected();

public slots:
    void start(int sokDesc, int bend) {
        qhttp::TBackend backend = static_cast<qhttp::TBackend>(bend);
        QHttpConnection *conn = QHttpConnection::create((qintptr) sokDesc,
                                                        backend,
                                                        this);

        QObject::connect(conn, &QHttpConnection::disconnected,
                         this, &ClientHandler::disconnected);

        conn->onHandler([this](QHttpRequest *req, QHttpResponse *res) {
            req->onEnd([this, req, res]() {
                gHandledConnections.ref();
                res->addHeader("connection", "close");

                QRegExp exp("^/allocation/(loc|pgrk|kmeans|rnd)$");

                if (exp.indexIn(req->url().path()) != -1) {
                    QString method = exp.capturedTexts()[1];
                    QString queryStr = req->url().query();
                    QUrlQuery query(queryStr);

                    QString originalReq = "{}";
                    QHash<Geometry *, Allocation *> allocation;

                    // convert the method name
                    if (method == "loc") method = LOCATION_ALLOCATION_MEHTOD_NAME;
                    else if (method == "pgrk") method = PAGE_RANK_MEHTOD_NAME;
                    else if (method == "kmeans") method = K_MEANS_MEHTOD_NAME;
                    else if (method == "rnd") method = RANDOM_METHOD_NAME;

                    if (method == LOCATION_ALLOCATION_MEHTOD_NAME) { // allocation allocation

                        int nbFacilities = query.queryItemValue("nbFacilities").toInt();
                        double deadline = query.queryItemValue("deadline").toDouble();
                        double delFactor = query.queryItemValue("delFactor").toDouble();

                        TravelTimeStat ttStat = NoneTT;
                        double travelTime = 0.0;
                        if (query.hasQueryItem("travelTime")) {
                            QString tt = query.queryItemValue("travelTime");
                            QRegExp exp1("^(med|avg|[\\d.]+)");
                            if (exp1.indexIn(tt) != -1) {
                                QString stat = exp1.capturedTexts()[1];
                                if (stat == "med") ttStat = Med;
                                else if (stat == "avg") ttStat = Avg;
                                travelTime = stat.toDouble();
                            }
                        }

                        DistanceStat dStat = NoneD;
                        double distance = 0.0;
                        if (query.hasQueryItem("distance")) {
                            QString d = query.queryItemValue("distance");
                            QRegExp exp1("^(auto|[\\d.]+)");
                            if (exp1.indexIn(d) != -1) {
                                QString stat = exp1.capturedTexts()[1];
                                if (stat == "auto") dStat = Auto;
                                else {
                                    dStat = FixedD;
                                    distance = stat.toDouble();
                                }
                            }
                        }

                        AllocationParams params(deadline,nbFacilities,delFactor,ttStat,dStat,travelTime,distance,method);

                        qDebug() << "Method" << method << "nbFacilities" << nbFacilities << "deadline" << deadline <<
                        "delFactor" << delFactor << "travelStat" << ttStat << "travelTime" << travelTime << "dstat" <<
                        dStat << "distance" << distance;

                        /* run the allocation function */
                        Loader l;
                        ProgressConsole p;
                        connect(&l, &Loader::loadProgressChanged, &p, &ProgressConsole::updateProgress);
                        l.load(_computeAllocation, &ComputeAllocation::processAllocationMethod, &l, &params, &allocation);

                        originalReq = QString(
                                "{\"method\":\"%1\",\"nbFacilities\":\"%2\",\"deadline\":\"%3\",\"delFactor\":\"%4\",\"travelTime\":\"%5\",\"distance\":\"%6\"}").arg(
                                method, QString::number(nbFacilities), QString::number(deadline),
                                QString::number(delFactor), QString::number(travelTime), QString::number(distance));

                    } else if (method == PAGE_RANK_MEHTOD_NAME) { // page rank

                    } else if (method == K_MEANS_MEHTOD_NAME) { // kmeans

                    } else if (method == RANDOM_METHOD_NAME) { // random
                        int nbFacilities = query.queryItemValue("nbFacilities").toInt();

                        qDebug() << "Method" << method << "nbFacilities" << nbFacilities;
                        Loader l;
                        ProgressConsole p;
                        connect(&l, &Loader::loadProgressChanged, &p, &ProgressConsole::updateProgress);
                        l.load(_computeAllocation, &ComputeAllocation::runRandomAllocation, &l, nbFacilities, &allocation);

                        originalReq = QString("{\"method\":\"%1\",\"nbFacilities\":\"%2\"}").arg(method,
                                                                                                QString::number(nbFacilities));
                    }

                    QString allocationStr = constructResponse(allocation);
                    QString respBody = QString("{\"originalReq\":%1, \"allocationResult\":%2}").arg(originalReq,
                                                                                                    allocationStr);
                    res->setStatusCode(qhttp::ESTATUS_OK);
                    res->addHeader("Content-Type", "application/json");
                    res->end(respBody.toUtf8());

                }

            });
        });

    };

private:
    ComputeAllocation* _computeAllocation;
    QString constructResponse(QHash<Geometry*, Allocation*> const &allocation);

};

///////////////////////////////////////////////////////////////////////////////

class RESTServerPrivate
{
    Q_DECLARE_PUBLIC(RESTServer)
    RESTServer* const   q_ptr;

public:
    QBasicTimer     itimer;
    QElapsedTimer   ielapsed;

    quint64         itotalHandled = 0;   ///< total connections being handled.

    ThreadList               ithreads;
    QVector<ClientHandler*>  iclients;

public:
    explicit    RESTServerPrivate(RESTServer* q) : q_ptr(q) {
    }

    virtual    ~RESTServerPrivate() {
    }

    void        start(size_t threads) {
        printf("\nDateTime,AveTps,miliSecond,Count,TotalCount\n");
        itimer.start(10000, Qt::CoarseTimer, q_ptr);
        ielapsed.start();

        // create threads
        if ( threads > 1 ) {
            ithreads.create(threads);

            for ( size_t i = 0;    i < threads;    i++ ) {
                ClientHandler *ch = new ClientHandler(q_ptr->_computeAllocation);
                ch->setup( ithreads.at(i) );
                iclients.append( ch );
            }

            ithreads.startAll();
        }

    }

    void        log() {
        quint32  tempHandled = gHandledConnections.load();

        itotalHandled   += tempHandled;
        quint32 miliSec  = (quint32) ielapsed.elapsed();
        float   aveTps   = float(tempHandled * 1000.0) / float(miliSec);
        QString dateTime = QLocale::c().toString(
                QDateTime::currentDateTime(),
                "yyyy-MM-dd hh:mm:ss");

        printf("%s,%.1f,%u,%u,%llu\n",
               qPrintable(dateTime),
               aveTps, miliSec,
               tempHandled, itotalHandled
        );

        fflush(stdout);

        gHandledConnections.store(0);
        ielapsed.start();
    }
};


RESTServer::RESTServer(size_t threads, QObject *parent, ComputeAllocation* computeAllocation) :
        QHttpServer(parent), d_ptr(new RESTServerPrivate(this)), _computeAllocation(computeAllocation)
{
    d_func()->start(threads);
}

RESTServer::~RESTServer() {
    stopListening();
}

///////////////////////////////////////////////////////////////////////////////

void RESTServer::incomingConnection(qintptr handle) {
    static quint64 counter = 0;

    Q_D(RESTServer);

    if ( d->iclients.size() > 1 ) { //multi-thread
        size_t index = counter % d->iclients.size();
        counter++;

        QMetaObject::invokeMethod(d_func()->iclients.at(index),
                                  "start",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, handle),
                                  Q_ARG(int, backendType())
        );
    } else { // single-thread
        ClientHandler* cli = new ClientHandler(_computeAllocation);
        QObject::connect(cli,   &ClientHandler::disconnected,
                         cli,   &ClientHandler::deleteLater);
        cli->start((int)handle, (int)backendType());
    }
}

void RESTServer::timerEvent(QTimerEvent *e) {
    Q_D(RESTServer);

    if ( e->timerId() == d->itimer.timerId() ) {
        d->log();
    }

    QHttpServer::timerEvent(e);
}


QString ClientHandler::constructResponse(QHash<Geometry*, Allocation*> const &allocation) {
    // format allocation response
    QString allocationStr = "{";
    int allocationCtr = 0;
    for (auto it = allocation.begin(); it != allocation.end(); ++it) {
        allocationStr += QString(
                "\"%1\": {\"x\":\"%2\", \"y\":\"%3\", \"weight\":\"%4\", \"nbAllocated\":\"%5\", \"nbDeleted\":\"%6\", \"rank\":\"%7\"},").arg(
                QString::number(it.value()->rank), QString::number(it.key()->getCenter().x()),
                QString::number(it.key()->getCenter().y()), QString::number(it.value()->weight),
                QString::number(it.value()->demands.size()),
                QString::number(it.value()->deletedCandidates.size()),
                QString::number(it.value()->rank));
        allocationCtr++;
    }
    if (allocationCtr == allocation.size()) {
        // delete the last comma
        allocationStr = allocationStr.left(allocationStr.size() - 1);
    }
    allocationStr += "}";
    return allocationStr;
}

#include "rest_server.moc"