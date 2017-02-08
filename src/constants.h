#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QColor>
#include <QSet>
#include <QTimeZone>

const long long MaxTime = std::numeric_limits<long long>::max();
const int    GRID_SIZE = 2000;

const QTimeZone TZ_EST("America/New_York");

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

const QColor COL_WAZE_ALERT_JAM(56,168,0);
const QColor COL_WAZE_ALERT_ROAD_CLOSED(176,224,0);
const QColor COL_WAZE_ALERT_WEATHERHAZARD(255,170,0);
const QColor COL_WAZE_ALERT_ACCIDENT(255,0,0);

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

const QString RTE_DISPLAY_JOURNEY_TIME  = "Journey time";
const QString RTE_DISPLAY_TRAFFIC_FLOW  = "Traffic flow";
const QString RTE_DISPLAY_SPEED         = "Speed";
const QString RTE_DISPLAY_SPEED_CLASSES = "Speed classes";

const QString RTWDE_DISPLAY_ALL           = "All";
const QString RTWDE_DISPLAY_JAM           = "Jam";
const QString RTWDE_DISPLAY_ROADCLOSED    = "Road closed";
const QString RTWDE_DISPLAY_WEATHERHAZARD = "Weather hazard";
const QString RTWDE_DISPLAY_ACCIDENT      = "Accident";

const QString WAZE_ALERT_JAM           = "JAM";
const QString WAZE_ALERT_WEATHERHAZARD = "WEATHERHAZARD";
const QString WAZE_ALERT_ROAD_CLOSED   = "ROAD_CLOSED";
const QString WAZE_ALERT_ACCIDENT      = "ACCIDENT";

const QString WAZE_SUBALERT_JAM_HEAVY_TRAFFIC = "JAM_HEAVY_TRAFFIC";
const QString WAZE_SUBALERT_JAM_STAND_STILL_TRAFFIC = "JAM_STAND_STILL_TRAFFIC";
const QString WAZE_SUBALERT_JAM_MODERATE_TRAFFIC = "JAM_MODERATE_TRAFFIC";

const QString WAZE_SUBALERT_ACCIDENT_MINOR = "ACCIDENT_MINOR";
const QString WAZE_SUBALERT_ACCIDENT_MAJOR = "ACCIDENT_MAJOR";

const QString WAZE_SUBALERT_ROAD_CLOSED_EVENT = "ROAD_CLOSED_EVENT";
const QString WAZE_SUBALERT_ROAD_CLOSED_CONSTRUCTION = "ROAD_CLOSED_CONSTRUCTION";
const QString WAZE_SUBALERT_ROAD_CLOSED_HAZARD = "ROAD_CLOSED_HAZARD";

const QString WAZE_SUBALERT_HAZARD_ON_ROAD_POT_HOLE = "HAZARD_ON_ROAD_POT_HOLE";
const QString WAZE_SUBALERT_HAZARD_ON_ROAD_CAR_STOPPED = "HAZARD_ON_ROAD_CAR_STOPPED";
const QString WAZE_SUBALERT_HAZARD_ON_SHOULDER_CAR_STOPPED = "HAZARD_ON_SHOULDER_CAR_STOPPED";
const QString WAZE_SUBALERT_HAZARD_ON_ROAD_CONSTRUCTION = "HAZARD_ON_ROAD_CONSTRUCTION";
const QString WAZE_SUBALERT_HAZARD_ON_ROAD_OBJECT = "HAZARD_ON_ROAD_OBJECT";
const QString WAZE_SUBALERT_HAZARD_ON_ROAD_ICE = "HAZARD_ON_ROAD_ICE";
const QString WAZE_SUBALERT_HAZARD_ON_ROAD = "HAZARD_ON_ROAD";
const QString WAZE_SUBALERT_HAZARD_ON_SHOULDER = "HAZARD_ON_SHOULDER";
const QString WAZE_SUBALERT_HAZARD_ON_SHOULDER_MISSING_SIGN = "HAZARD_ON_SHOULDER_MISSING_SIGN";
const QString WAZE_SUBALERT_HAZARD_ON_ROAD_ROAD_KILL = "HAZARD_ON_ROAD_ROAD_KILL";
const QString WAZE_SUBALERT_HAZARD_ON_SHOULDER_ANIMALS = "HAZARD_ON_SHOULDER_ANIMALS";

const QString WAZE_SUBALERT_HAZARD_WEATHER = "HAZARD_WEATHER";
const QString WAZE_SUBALERT_HAZARD_WEATHER_FOG = "HAZARD_WEATHER_FOG";
const QString WAZE_SUBALERT_HAZARD_WEATHER_FREEZING_RAIN = "HAZARD_WEATHER_FREEZING_RAIN";


/* ENUM TYPES */
enum GeometryType   { NoneGeometryType, CircleGeometryType, CellGeometryType, CoordGeometryType, PathGeometryType, PolygonGeometryType };
enum WithinOperator { AndWithin, OrWithin, NoneWithin };
enum TravelTimeStat { NoneTTStat, MedTTStat, AvgTTStat };
enum DistanceStat   { NoneDStat, AutoDStat, FixedDStat };
enum RoadTrafficDataType {
    JourneyTimeRTDType,
    FlowRTDType,
    SpeedRTDType,
    SpeedClassRTDType,
    Speed0RTDType,  // 0-20 mph
    Speed1RTDType,  // 21-25 mph
    Speed2RTDType,  // 26-30 mph
    Speed3RTDType,  // 31-35 mph
    Speed4RTDType,  // 36-40 mph
    Speed5RTDType,  // 41-45 mph
    Speed6RTDType,  // 46-50 mph
    Speed7RTDType,  // 51-55 mph
    Speed8RTDType,  // 56-60 mph
    Speed9RTDType,  // 61-65 mph
    Speed10RTDType, // 66-70 mph
    Speed11RTDType, // 71-75 mph
    Speed12RTDType, // 76-80 mph
    Speed13RTDType, // 81-85 mph
    Speed14RTDType, // 86+ mph
    NoneRTDType
};

enum RoadTrafficExaminerDisplayType {
    DayRTEType,
    MonthRTEType,
    AllPeriodRTEType,
    NoneRTEType
};
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
    WAZE_SUBTYPE_JAM_MODERATE_TRAFFIC,
    WAZE_SUBTYPE_ACCIDENT_MINOR,
    WAZE_SUBTYPE_ACCIDENT_MAJOR,
    WAZE_SUBTYPE_ROAD_CLOSED_HAZARD,
    WAZE_SUBTYPE_ROAD_CLOSED_EVENT,
    WAZE_SUBTYPE_ROAD_CLOSED_CONSTRUCTION,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_POT_HOLE,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_CAR_STOPPED,
    WAZE_SUBTYPE_HAZARD_ON_SHOULDER_CAR_STOPPED,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_CONSTRUCTION,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_OBJECT,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_ICE,
    WAZE_SUBTYPE_HAZARD_ON_ROAD,
    WAZE_SUBTYPE_HAZARD_ON_SHOULDER,
    WAZE_SUBTYPE_HAZARD_ON_SHOULDER_MISSING_SIGN,
    WAZE_SUBTYPE_HAZARD_ON_ROAD_ROAD_KILL,
    WAZE_SUBTYPE_HAZARD_ON_SHOULDER_ANIMALS,
    WAZE_SUBTYPE_HAZARD_WEATHER,
    WAZE_SUBTYPE_HAZARD_WEATHER_FOG,
    WAZE_SUBTYPE_HAZARD_WEATHER_FREEZING_RAIN
};

/* SETTINGS VALUE NAMES */

const QString SETTINGS_ROAD_TRAFFIC_WAZE_DATA_EXAMINER_ALERT_TYPE_DISPLAY = "savedAlertTypeDisplayRoadTrafficWazeDataExaminer";
const QString SETTINGS_ROAD_TRAFFIC_WAZE_DATA_EXAMINER_ALERT_SUBTYPE_DISPLAY = "savedAlertSubTypeDisplayRoadTrafficWazeDataExaminer";
const QString SETTINGS_ROAD_TRAFFIC_EXAMINER_DATE = "savedDateRoadTrafficExaminer";
const QString SETTINGS_ROAD_TRAFFIC_EXAMINER_DISPLAY = "savedDisplayRoadTrafficExaminer";
const QString SETTINGS_ROAD_TRAFFIC_EXAMINER_DISPLAY_TYPE =  "savedDisplayTypeRoadTrafficExaminer";

/* Conversion functions */


// Convert RoadTrafficDataType -> Road traffic examiner display string
static QString roadTrafficDataTypeToString(RoadTrafficDataType rtdType) {
    if(rtdType == RoadTrafficDataType::JourneyTimeRTDType) return RTE_DISPLAY_JOURNEY_TIME;
    else if(rtdType == RoadTrafficDataType::FlowRTDType) return RTE_DISPLAY_TRAFFIC_FLOW;
    else if(rtdType == RoadTrafficDataType::SpeedRTDType) return RTE_DISPLAY_SPEED;
    else if(rtdType == RoadTrafficDataType::SpeedClassRTDType ||
            (rtdType <= RoadTrafficDataType::Speed14RTDType &&
             rtdType >= RoadTrafficDataType::Speed0RTDType))
        return RTE_DISPLAY_SPEED_CLASSES;
    else return "";
}
static RoadTrafficDataType stringToRoadTrafficDataType(const QString& s) {
    if(s == RTE_DISPLAY_JOURNEY_TIME) return RoadTrafficDataType::JourneyTimeRTDType;
    else if(s == RTE_DISPLAY_TRAFFIC_FLOW) return RoadTrafficDataType::FlowRTDType;
    else if(s == RTE_DISPLAY_SPEED) return RoadTrafficDataType::SpeedRTDType;
        else if(s == RTE_DISPLAY_SPEED_CLASSES) return RoadTrafficDataType::SpeedClassRTDType;
    else return NoneRTDType;
}

static WazeAlertType stringToAlertType(const QString& s) {
    if(s == WAZE_ALERT_JAM || s == RTWDE_DISPLAY_JAM) return WazeAlertType::WAZE_TYPE_JAM;
    else if(s == WAZE_ALERT_WEATHERHAZARD || s == RTWDE_DISPLAY_WEATHERHAZARD) return WazeAlertType::WAZE_TYPE_WEATHERHAZARD;
    else if(s == WAZE_ALERT_ROAD_CLOSED || s == RTWDE_DISPLAY_ROADCLOSED) return WazeAlertType::WAZE_TYPE_ROAD_CLOSED;
    else if(s == WAZE_ALERT_ACCIDENT || s == RTWDE_DISPLAY_ACCIDENT) return WazeAlertType::WAZE_TYPE_ACCIDENT;
    else return WazeAlertType::WAZE_TYPE_NONE;
}

static WazeAlertSubType stringToAlertSubType(const QString& s) {
    if(s == WAZE_SUBALERT_JAM_HEAVY_TRAFFIC) return WazeAlertSubType::WAZE_SUBTYPE_JAM_HEAVY_TRAFFIC;
    else if(s == WAZE_SUBALERT_JAM_STAND_STILL_TRAFFIC) return WazeAlertSubType::WAZE_SUBTYPE_JAM_STAND_STILL_TRAFFIC;
    else if(s == WAZE_SUBALERT_JAM_MODERATE_TRAFFIC) return WazeAlertSubType::WAZE_SUBTYPE_JAM_MODERATE_TRAFFIC;

    else if(s == WAZE_SUBALERT_ACCIDENT_MINOR) return WazeAlertSubType::WAZE_SUBTYPE_ACCIDENT_MINOR;
    else if(s == WAZE_SUBALERT_ACCIDENT_MAJOR) return WazeAlertSubType::WAZE_SUBTYPE_ACCIDENT_MAJOR;

    else if(s == WAZE_SUBALERT_ROAD_CLOSED_EVENT) return WazeAlertSubType::WAZE_SUBTYPE_ROAD_CLOSED_EVENT;
    else if(s == WAZE_SUBALERT_ROAD_CLOSED_CONSTRUCTION) return WazeAlertSubType::WAZE_SUBTYPE_ROAD_CLOSED_CONSTRUCTION;
    else if(s == WAZE_SUBALERT_ROAD_CLOSED_HAZARD) return WazeAlertSubType::WAZE_SUBTYPE_ROAD_CLOSED_HAZARD;

    else if(s == WAZE_SUBALERT_HAZARD_ON_ROAD_POT_HOLE) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD_POT_HOLE;
    else if(s == WAZE_SUBALERT_HAZARD_ON_ROAD_CAR_STOPPED) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD_CAR_STOPPED;
    else if(s == WAZE_SUBALERT_HAZARD_ON_SHOULDER_CAR_STOPPED) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_SHOULDER_CAR_STOPPED;
    else if(s == WAZE_SUBALERT_HAZARD_ON_ROAD_CONSTRUCTION) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD_CONSTRUCTION;
    else if(s == WAZE_SUBALERT_HAZARD_ON_ROAD_OBJECT) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD_OBJECT;
    else if(s == WAZE_SUBALERT_HAZARD_ON_ROAD_ICE) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD_ICE;
    else if(s == WAZE_SUBALERT_HAZARD_ON_ROAD) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD;
    else if(s == WAZE_SUBALERT_HAZARD_ON_SHOULDER) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_SHOULDER;
    else if(s == WAZE_SUBALERT_HAZARD_ON_SHOULDER_MISSING_SIGN) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_SHOULDER_MISSING_SIGN;
    else if(s == WAZE_SUBALERT_HAZARD_ON_ROAD_ROAD_KILL) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD_ROAD_KILL;
    else if(s == WAZE_SUBALERT_HAZARD_ON_SHOULDER_ANIMALS) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_SHOULDER_ANIMALS;

    else if(s == WAZE_SUBALERT_HAZARD_WEATHER) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_WEATHER;
    else if(s == WAZE_SUBALERT_HAZARD_WEATHER_FOG) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_WEATHER_FOG;
    else if(s == WAZE_SUBALERT_HAZARD_WEATHER_FREEZING_RAIN) return WazeAlertSubType::WAZE_SUBTYPE_HAZARD_WEATHER_FREEZING_RAIN;
    else return WazeAlertSubType::WAZE_SUBTYPE_NONE;
}

static QString alertSubTypeToString(WazeAlertSubType s) {
    if(s == WazeAlertSubType::WAZE_SUBTYPE_JAM_HEAVY_TRAFFIC) return WAZE_SUBALERT_JAM_HEAVY_TRAFFIC;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_JAM_STAND_STILL_TRAFFIC) return WAZE_SUBALERT_JAM_STAND_STILL_TRAFFIC;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_JAM_MODERATE_TRAFFIC) return WAZE_SUBALERT_JAM_MODERATE_TRAFFIC;

    else if(s == WazeAlertSubType::WAZE_SUBTYPE_ACCIDENT_MINOR) return WAZE_SUBALERT_ACCIDENT_MINOR;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_ACCIDENT_MAJOR) return WAZE_SUBALERT_ACCIDENT_MAJOR;

    else if(s == WazeAlertSubType::WAZE_SUBTYPE_ROAD_CLOSED_EVENT) return WAZE_SUBALERT_ROAD_CLOSED_EVENT;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_ROAD_CLOSED_CONSTRUCTION) return WAZE_SUBALERT_ROAD_CLOSED_CONSTRUCTION;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_ROAD_CLOSED_HAZARD) return WAZE_SUBALERT_ROAD_CLOSED_HAZARD;

    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD_POT_HOLE) return WAZE_SUBALERT_HAZARD_ON_ROAD_POT_HOLE;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD_CAR_STOPPED) return WAZE_SUBALERT_HAZARD_ON_ROAD_CAR_STOPPED;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_SHOULDER_CAR_STOPPED) return WAZE_SUBALERT_HAZARD_ON_SHOULDER_CAR_STOPPED;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD_CONSTRUCTION) return WAZE_SUBALERT_HAZARD_ON_ROAD_CONSTRUCTION;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD_OBJECT) return WAZE_SUBALERT_HAZARD_ON_ROAD_OBJECT;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD_ICE) return WAZE_SUBALERT_HAZARD_ON_ROAD_ICE;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD) return WAZE_SUBALERT_HAZARD_ON_ROAD;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_SHOULDER) return WAZE_SUBALERT_HAZARD_ON_SHOULDER;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_SHOULDER_MISSING_SIGN) return WAZE_SUBALERT_HAZARD_ON_SHOULDER_MISSING_SIGN;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_ROAD_ROAD_KILL) return WAZE_SUBALERT_HAZARD_ON_ROAD_ROAD_KILL;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_ON_SHOULDER_ANIMALS) return WAZE_SUBALERT_HAZARD_ON_SHOULDER_ANIMALS;

    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_WEATHER) return WAZE_SUBALERT_HAZARD_WEATHER;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_WEATHER_FOG) return WAZE_SUBALERT_HAZARD_WEATHER_FOG;
    else if(s == WazeAlertSubType::WAZE_SUBTYPE_HAZARD_WEATHER_FREEZING_RAIN) return WAZE_SUBALERT_HAZARD_WEATHER_FREEZING_RAIN;
    else return RTWDE_DISPLAY_ALL;
}

static RoadTrafficDataType speedClassToRoadTrafficDataType(int c) {
    if(c == 0) return RoadTrafficDataType::Speed0RTDType;
    else if(c == 1) return RoadTrafficDataType::Speed1RTDType;
    else if(c == 2) return RoadTrafficDataType::Speed2RTDType;
    else if(c == 3) return RoadTrafficDataType::Speed3RTDType;
    else if(c == 4) return RoadTrafficDataType::Speed4RTDType;
    else if(c == 5) return RoadTrafficDataType::Speed5RTDType;
    else if(c == 6) return RoadTrafficDataType::Speed6RTDType;
    else if(c == 7) return RoadTrafficDataType::Speed7RTDType;
    else if(c == 8) return RoadTrafficDataType::Speed8RTDType;
    else if(c == 9) return RoadTrafficDataType::Speed9RTDType;
    else if(c == 10) return RoadTrafficDataType::Speed10RTDType;
    else if(c == 11) return RoadTrafficDataType::Speed11RTDType;
    else if(c == 12) return RoadTrafficDataType::Speed12RTDType;
    else if(c == 13) return RoadTrafficDataType::Speed13RTDType;
    else if(c == 14) return RoadTrafficDataType::Speed14RTDType;
    else return RoadTrafficDataType::NoneRTDType;
}

static QString speedClassToLabel(RoadTrafficDataType rtdType) {
    if(rtdType == RoadTrafficDataType::Speed0RTDType) return "0-20 mph";
    else if(rtdType == RoadTrafficDataType::Speed1RTDType)  return "21-25 mph";
    else if(rtdType == RoadTrafficDataType::Speed2RTDType)  return "26-30 mph";
    else if(rtdType == RoadTrafficDataType::Speed3RTDType)  return "31-35 mph";
    else if(rtdType == RoadTrafficDataType::Speed4RTDType)  return "36-40 mph";
    else if(rtdType == RoadTrafficDataType::Speed5RTDType)  return "41-45 mph";
    else if(rtdType == RoadTrafficDataType::Speed6RTDType)  return "46-50 mph";
    else if(rtdType == RoadTrafficDataType::Speed7RTDType)  return "51-55 mph";
    else if(rtdType == RoadTrafficDataType::Speed8RTDType)  return "56-60 mph";
    else if(rtdType == RoadTrafficDataType::Speed9RTDType)  return "61-65 mph";
    else if(rtdType == RoadTrafficDataType::Speed10RTDType) return "66-70 mph";
    else if(rtdType == RoadTrafficDataType::Speed11RTDType) return "71-75 mph";
    else if(rtdType == RoadTrafficDataType::Speed12RTDType) return "76-80 mph";
    else if(rtdType == RoadTrafficDataType::Speed13RTDType) return "81-85 mph";
    else if(rtdType == RoadTrafficDataType::Speed14RTDType) return "85+ mph";
    else return "";
}

static QColor wazeAlertTypeToColor(WazeAlertType wat) {
    switch (wat) {
        case WazeAlertType::WAZE_TYPE_JAM:
            return COL_WAZE_ALERT_JAM;
        case WazeAlertType::WAZE_TYPE_ROAD_CLOSED:
            return COL_WAZE_ALERT_ROAD_CLOSED;
        case WazeAlertType::WAZE_TYPE_WEATHERHAZARD:
            return COL_WAZE_ALERT_WEATHERHAZARD;
        case WazeAlertType::WAZE_TYPE_ACCIDENT:
            return COL_WAZE_ALERT_ACCIDENT;
        default:
            return QColor();

    }
}

#endif // CONSTANTS_H
