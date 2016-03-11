#include "proj_factory.h"

#include <QDebug>

ProjFactory::ProjFactory(QString projIn, QString projOut) {
    // construct the in-projections and out-projections
    setProj(projIn, projOut);
}

void ProjFactory::transformCoordinates(double lat, double lon, double *x, double *y) {
    double x1 = 0;
    double y1 = 0;
    // Transformation of the lat/lon coordinates to projected coordinates
    if(_projIn && _projOut) {
        x1 = lon * DEG_TO_RAD;
        y1 = lat * DEG_TO_RAD;
        pj_transform(_projIn, _projOut, 1, 1, &x1, &y1, NULL);
    } else {
        x1 = lon;
        y1 = lat;
    }

    *x = x1;
    *y = y1;
}

void ProjFactory::revertCoordinates(double x, double y, double *lat, double *lon) {
    double lon1 = 0;
    double lat1 = 0;
    // Transformation of the lat/lon coordinates to projected coordinates
    if(_projIn && _projOut) {
        lon1 = x, lat1 = y;
        pj_transform(_projOut, _projIn, 1, 1, &lon1, &lat1, NULL);
        lon1 *= RAD_TO_DEG;
        lat1 *= RAD_TO_DEG;
    } else {
        lon1 = x;
        lat1 = y;
    }

    *lat = lat1;
    *lon = lon1;
}
