#ifndef RESTSERVER_H
#define RESTSERVER_H

#include "compute_allocation.h"
#include "qhttpserver.hpp"

// forward class declarations
class ComputeAllocation;
class RESTServerPrivate;

class RESTServer: public qhttp::server::QHttpServer {
public:
    explicit     RESTServer(size_t threads, QObject* parent, ComputeAllocation* computeAllocation);
    virtual      ~RESTServer();

protected:
    void        incomingConnection(qintptr handle) override;
    void        timerEvent(QTimerEvent *) override;

    Q_DECLARE_PRIVATE(RESTServer)
    Q_DISABLE_COPY(RESTServer)
    QScopedPointer<RESTServerPrivate>   d_ptr;

    ComputeAllocation* _computeAllocation;
};

#endif // RESTSERVER_H
