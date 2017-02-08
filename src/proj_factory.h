#ifndef PROJFACTORY_H
#define PROJFACTORY_H

#include "proj_api.h"

#include "constants.h"

#include <QPointF>
#include <QList>
#include <QDebug>
#include <QMap>


class ProjFactory {
public:
    static ProjFactory& getInstance() {
        static ProjFactory instance; // Guaranteed to be destroyed.
                                     // Instantiated on first use.
        return instance;
    }

    void revertCoordinates(double x, double y, double* lat, double* lon);
    void transformCoordinates(const QString& projIn, double lat, double lon, double* x, double* y);
    void transformCoordinates(double lat, double lon, double* x, double* y);
    void setProj(const QString& projIn, const QString& projOut) {
        setProjIn(projIn);
        setProjOut(projOut);
    }

    void setProjIn(const QString& projIn) {
        if(!projIn.isEmpty())
            _projIn  = pj_init_plus(projIn.toStdString().c_str());
    }

    void setProjOut(const QString& projOut) {
        if(!projOut.isEmpty())
            _projOut = pj_init_plus(projOut.toStdString().c_str());
    }

    void getProj(projPJ* proj, const QString& p) {
        *proj = pj_init_plus(p.toStdString().c_str());
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

    static bool areSameProj(QString proj1, QString proj2) {
        if(proj1.isEmpty() && proj2.isEmpty())
            return true;

        projPJ pj1 = pj_init_plus(proj1.toStdString().c_str());
        projPJ pj2 = pj_init_plus(proj2.toStdString().c_str());

        return strcmp(pj_get_def(pj1,0), pj_get_def(pj2,0)) == 0;
    }

    static bool isValidProj(QString proj) {
        projPJ pj = pj_init_plus(proj.toStdString().c_str());
        bool ret = true;
        if(!pj) {
            ret = false;
        }
        pj_free(pj);
        return ret;
    }

private:
    ProjFactory(const QString& projIn = 0, const QString& projOut = 0);
    ProjFactory(ProjFactory const&)     = delete;
    void operator=(ProjFactory const&)  = delete;

    projPJ _projIn  = 0;
    projPJ _projOut = 0;
};

#endif // PROJFACTORY_H
