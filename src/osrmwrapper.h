#ifndef OSRMWRAPPER_H
#define OSRMWRAPPER_H

#include "../library/osrm.hpp"
#include "../util/git_sha.hpp"
#include "../util/json_renderer.hpp"
#include "../util/routed_options.hpp"
#include "../util/simple_logger.hpp"

#include <osrm/json_container.hpp>
#include <osrm/libosrm_config.hpp>
#include <osrm/route_parameters.hpp>

#include "/usr/local/include/proj_api.h"

#include "constants.h"

#include <QPointF>
#include <QList>
#include <QMap>


class OSRMWrapper
{
public:
    static OSRMWrapper& getInstance()
    {
        static OSRMWrapper instance; // Guaranteed to be destroyed.
                                     // Instantiated on first use.
        return instance;
    }

    int computeRoute(QList<QPointF>* coordinates, QPointF start, QPointF end);

    void revertCoordinates(double x, double y, double* lat, double* lon);
    void transformCoordinates(double lat, double lon, double* x, double* y);
    void setProj(QString projIn, QString projOut) {
        if(!projIn.isEmpty())
            _projIn  = pj_init_plus(projIn.toStdString().c_str());
        if(!projOut.isEmpty())
            _projOut = pj_init_plus(projOut.toStdString().c_str());
    }
    void setProj(projPJ projIn, projPJ projOut) {
        _projIn  = projIn;
        _projOut = projOut;
    }

    char* getOutputProj() const {
        return pj_get_def(_projOut,0);
    }

    char* getInputProj() const {
        return pj_get_def(_projIn,0);
    }

private:
    OSRMWrapper(QString projIn = 0, QString projOut = 0);
    OSRMWrapper(OSRMWrapper const&)     = delete;
    void operator=(OSRMWrapper const&)  = delete;
    int startUp();

    OSRM* _routingMachine;
    projPJ _projIn = 0, _projOut = 0;
    int _isStarted;
};

#endif // OSRMWRAPPER_H
