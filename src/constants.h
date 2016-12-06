#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QColor>


const long long MaxTime = std::numeric_limits<long long>::max();
const int    GRID_SIZE = 2000;

/* COLORS */

const QColor CELL_COLOR = Qt::red;
const double CELL_OPACITY = 0.5;
const QColor ORANGE = QColor(255, 127, 0);
const QColor BLUE   = Qt::darkBlue;
const QColor GREEN  = Qt::darkGreen;
const QColor RED    = Qt::darkRed;
const QColor BLACK  = Qt::black;

const QColor COL_LOW(56,168,0);
const QColor COL_MED_LOW(176,224,0);
const QColor COL_MED_HIGH(255,170,0);
const QColor COL_HIGH(255,0,0);

// colors taken from http://flatuicolors.com
const QColor SHAPEFILE_COL       = QColor(127, 140, 141);
const QColor TRACE_SELECTED_COL  = QColor(231, 76, 60);
const QColor LINK_SELECTED_COL   = QColor(153, 14, 240);
const QColor LINK_UNSELECTED_COL = QColor(52, 73, 94);

const int    SHAPEFILE_WID       = 2;
const int    TRACE_SELECTED_WID  = 3;
const int    LINK_SELECTED_WID   = 5;
const int    LINK_UNSELECTED_WID = 2;

/* Define the names of the different allocations */
const QString LOCATION_ALLOCATION_MEHTOD_NAME = "Location allocation";
const QString PAGE_RANK_MEHTOD_NAME = "Page Rank";
const QString K_MEANS_MEHTOD_NAME = "k-means";
const QString RANDOM_METHOD_NAME = "random";

const QString RTE_DISPLAY_JOURNEY_TIME = "Journey time";
const QString RTE_DISPLAY_TRAFFIC_FLOW = "Traffic flow";
const QString RTE_DISPLAY_SPEED        = "Speed";

const QString RTWDE_DISPLAY_ALL           = "All";
const QString RTWDE_DISPLAY_JAM           = "Jam";
const QString RTWDE_DISPLAY_ROADCLOSED    = "Road closed";
const QString RTWDE_DISPLAY_WEATHERHAZARD = "Weather hazard";
const QString RTWDE_DISPLAY_ACCIDENT      = "Accident";

const QString WAZE_ALERT_JAM           = "JAM";
const QString WAZE_ALERT_WEATHERHAZARD = "WEATHERHAZARD";
const QString WAZE_ALERT_ROAD_CLOSED   = "ROAD_CLOSED";
const QString WAZE_ALERT_ACCIDENT      = "ACCIDENT";


/* ENUM TYPES */
enum GeometryType   { NoneGeometryType, CircleGeometryType, CellGeometryType, CoordGeometryType, PathGeometryType, PolygonGeometryType };
enum WithinOperator { AndWithin, OrWithin, NoneWithin };
enum TravelTimeStat { NoneTTStat, MedTTStat, AvgTTStat };
enum DistanceStat   { NoneDStat, AutoDStat, FixedDStat };
enum RoadTrafficDataType { JourneyTimeRTDType, FlowRTDType, SpeedRTDType, NoneRTDType };
enum RoadTrafficExaminerDisplayType { DayRTEType, MonthRTEType, AllPeriodRTEType, NoneRTEType };
enum WazeAlertType {
    WAZE_TYPE_NONE,
    WAZE_TYPE_JAM,
    WAZE_TYPE_ROAD_CLOSED,
    WAZE_TYPE_WEATHERHAZARD,
    WAZE_TYPE_ACCIDENT
};
enum WazeAlertSubType {
    WAZE_SUBTYPE_NONE,
    WAZE_SUBTYPE_JAM_HEAVY_TRAFFIC,
    WAZE_SUBTYPE_JAM_STAND_STILL_TRAFFIC,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_POT_HOLE,
    WAZE_SUBTYPE_JAM_MODERATE_TRAFFIC,
    WAZE_SUBTYPE_ROAD_CLOSED_EVENT,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_CAR_STOPPED,
    WAZE_SUBTYPE_HAZARD_ON_SHOULDER_CAR_STOPPED,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_CONSTRUCTION,
    WAZE_SUBTYPE_ACCIDENT_MINOR,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_OBJECT,
    WAZE_SUBTYPE_ACCIDENT_MAJOR,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_ICE,
    WAZE_SUBTYPE_HAZARD_ON_ROAD,
    WAZE_SUBTYPE_HAZARD_ON_SHOULDER,
    WAZE_SUBTYPE_ROAD_CLOSED_HAZARD,
    WAZE_SUBTYPE_HAZARD_WEATHER,
    WAZE_SUBTYPE_HAZARD_ON_SHOULDER_MISSING_SIGN,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_ROAD_KILL,
    WAZE_SUBTYPE_ROAD_CLOSED_CONSTRUCTION,
    WAZE_SUBTYPE_HAZARD_ON_SHOULDER_ANIMALS,
    WAZE_SUBTYPE_HAZARD_WEATHER_FOG,
    WAZE_SUBTYPE_HAZARD_WEATHER_FREEZING_RAIN
};

/* SETTINGS VALUE NAMES */

const QString SETTINGS_ROAD_TRAFFIC_WAZE_DATA_EXAMINER_ALERT_TYPE_DISPLAY = "savedAlertTypeDisplayRoadTrafficWazeDataExaminer";
const QString SETTINGS_ROAD_TRAFFIC_EXAMINER_DATE = "savedDateRoadTrafficExaminer";
const QString SETTINGS_ROAD_TRAFFIC_EXAMINER_DISPLAY = "savedDisplayRoadTrafficExaminer";
const QString SETTINGS_ROAD_TRAFFIC_EXAMINER_DISPLAY_TYPE =  "savedDisplayTypeRoadTrafficExaminer";

/* Conversion functions */


// Convert RoadTrafficDataType -> Road traffic examiner display string
static QString roadTrafficDataTypeToString(RoadTrafficDataType rtdType) {
    if(rtdType == JourneyTimeRTDType) return RTE_DISPLAY_JOURNEY_TIME;
    else if(rtdType == FlowRTDType) return RTE_DISPLAY_TRAFFIC_FLOW;
    else if(rtdType == SpeedRTDType) return RTE_DISPLAY_SPEED;
    else return "";
}
static RoadTrafficDataType stringToRoadTrafficDataType(const QString& s) {
    if(s == RTE_DISPLAY_JOURNEY_TIME) return JourneyTimeRTDType;
    else if(s == RTE_DISPLAY_TRAFFIC_FLOW) return FlowRTDType;
    else if(s == RTE_DISPLAY_SPEED) return SpeedRTDType;
    else return NoneRTDType;
}

// Convert Road traffic waze data examiner display string -> WazeAlertType
static WazeAlertType stringToDisplayToAlertType(const QString &s) {
    if(s == RTWDE_DISPLAY_JAM) return WazeAlertType::WAZE_TYPE_JAM;
    else if(s == RTWDE_DISPLAY_WEATHERHAZARD) return WazeAlertType::WAZE_TYPE_WEATHERHAZARD;
    else if(s == RTWDE_DISPLAY_ROADCLOSED) return WazeAlertType::WAZE_TYPE_ROAD_CLOSED;
    else if(s == RTWDE_DISPLAY_ACCIDENT) return WazeAlertType::WAZE_TYPE_ACCIDENT;
    else return WazeAlertType::WAZE_TYPE_NONE;
}


static WazeAlertType stringToAlertType(const QString& s) {
    if(s == WAZE_ALERT_JAM) return WazeAlertType::WAZE_TYPE_JAM;
    else if(s == WAZE_ALERT_WEATHERHAZARD) return WazeAlertType::WAZE_TYPE_WEATHERHAZARD;
    else if(s == WAZE_ALERT_ROAD_CLOSED) return WazeAlertType::WAZE_TYPE_ROAD_CLOSED;
    else if(s == WAZE_ALERT_ACCIDENT) return WazeAlertType::WAZE_TYPE_ACCIDENT;
    else return WazeAlertType::WAZE_TYPE_NONE;
}




#endif // CONSTANTS_H
