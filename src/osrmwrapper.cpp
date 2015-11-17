#include "osrmwrapper.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "constants.h"

OSRMWrapper::OSRMWrapper(QString projIn, QString projOut)
{
    // construct the in-projections and out-projections
    setProj(projIn, projOut);
    // start OSRM engine
    // see the OSRM library API
    _isStarted = startUp();
}

int OSRMWrapper::startUp()
{
    if(!_projIn || !_projOut) {
        return 1;
    }

    try
    {
        std::string ip_address;
        int ip_port, requested_thread_num;
        int max_locations_map_matching;
        bool trial_run = false;
        libosrm_config lib_config;
        const char * argv[] = { "Routing" };
        const unsigned init_result = GenerateServerProgramOptions(
            1, argv, lib_config.server_paths, ip_address, ip_port, requested_thread_num,
            lib_config.use_shared_memory, trial_run, lib_config.max_locations_distance_table,
            max_locations_map_matching);

        if (init_result == INIT_OK_DO_NOT_START_ENGINE || init_result == INIT_FAILED)
            return 1;

        qDebug() << "starting up engines," << g_GIT_DESCRIPTION;

        _routingMachine = new OSRM(lib_config);
    }
    catch (std::exception &current_exception)
    {
        qWarning() << "caught exception: " << current_exception.what();
        return 1;
    }

    return 0;
}

int OSRMWrapper::computeRoute(QList<QPointF> *coordinates, QPointF start, QPointF end)
{
    if(!_isStarted) {
        // try to restart it
        if(!(_isStarted = startUp())) {
            return 1;
        }
    }

    RouteParameters route_parameters;
    route_parameters.zoom_level = 18;           // no generalization
    route_parameters.print_instructions = false; // turn by turn instructions
    route_parameters.alternate_route = false;    // get an alternate route, too
    route_parameters.geometry = true;           // retrieve geometry of route
    route_parameters.compression = false;        // polyline encoding
    route_parameters.check_sum = -1;            // see wiki
    route_parameters.service = "viaroute";      // that's routing
    route_parameters.output_format = "json";
    route_parameters.jsonp_parameter = ""; // set for jsonp wrapping
    route_parameters.language = "";        // unused atm
    // route_parameters.hints.push_back(); // see wiki, saves I/O if done properly

    // transform and add the start coordinate
    double start_lon, start_lat;
    revertCoordinates(start.x(), start.y(), &start_lat, &start_lon);
    route_parameters.coordinates.emplace_back(start_lat * COORDINATE_PRECISION,
                                              start_lon * COORDINATE_PRECISION);

    // transform and add the end coordinate
    double end_lon, end_lat;
    revertCoordinates(end.x(), end.y(), &end_lat, &end_lon);
    route_parameters.coordinates.emplace_back(end_lat * COORDINATE_PRECISION,
                                              end_lon * COORDINATE_PRECISION);

    // compute the route
    qDebug() << "route between (" << start_lon << "," << start_lat << ") and (" << end_lon << "," << end_lat << ")";
    osrm::json::Object json_result;
    const int result_code = _routingMachine->RunQuery(route_parameters, json_result);
    qDebug() << "http code:" << result_code;
    if(result_code == 200) {
        std::vector<char> output;
        osrm::json::render(output, json_result);
        std::string str_output(output.begin(),output.end());
        QJsonDocument d = QJsonDocument::fromJson(QString::fromStdString(str_output).toUtf8());
        QJsonObject routeObject = d.object();
        QJsonArray routeArray = routeObject["route_geometry"].toArray();
        foreach(const QJsonValue & value, routeArray) {
            // get the coordinates
            QJsonArray coords = value.toArray();
            double lat = coords.at(0).toDouble();
            double lon = coords.at(1).toDouble();

            // transform the coordinates back to the projected coordinate system
            double x, y;
            transformCoordinates(lat, lon, &x, &y);

            // record the coordinates
            coordinates->append(QPointF(x,y));
        }
    }

    return 0;
}

void OSRMWrapper::transformCoordinates(double lat, double lon, double *x, double *y)
{
    double x1=0,y1=0;
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

void OSRMWrapper::revertCoordinates(double x, double y, double *lat, double *lon)
{
    double lon1=0,lat1=0;
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
