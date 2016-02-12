#ifndef PROJFACTORY_H
#define PROJFACTORY_H

#include "proj_api.h"

#include "constants.h"

#include <QPointF>
#include <QList>
#include <QMap>


class ProjFactory
{
public:
    static ProjFactory & getInstance()
    {
        static ProjFactory instance; // Guaranteed to be destroyed.
                                     // Instantiated on first use.
        return instance;
    }

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
        if(_projOut)
            return pj_get_def(_projOut,0);
        return (char*)"";
    }

    char* getInputProj() const {
        if(_projIn)
            return pj_get_def(_projIn,0);
        return (char*)"";
    }

private:
    ProjFactory(QString projIn = 0, QString projOut = 0);
    ProjFactory(ProjFactory const&)     = delete;
    void operator=(ProjFactory const&)  = delete;

    projPJ _projIn = 0, _projOut = 0;
};

#endif // PROJFACTORY_H
