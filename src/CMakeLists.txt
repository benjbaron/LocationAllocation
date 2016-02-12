cmake_minimum_required(VERSION 3.3)
project(LocAll)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_PREFIX_PATH "/usr/local/opt/qt5/lib/cmake")
INCLUDE_DIRECTORIES(/usr/local/include)
INCLUDE_DIRECTORIES(../qhttp/src)
LINK_DIRECTORIES(/usr/local/lib)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

FIND_PACKAGE(Qt5Widgets REQUIRED)
FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui REQUIRED)
FIND_PACKAGE(Qt5Concurrent REQUIRED)
FIND_PACKAGE(Qt5PrintSupport REQUIRED)
FIND_PACKAGE(Qt5Network REQUIRED)

FIND_LIBRARY(geos REQUIRED)
FIND_LIBRARY(proj.9 REQUIRED)
FIND_LIBRARY(gdal.1 REQUIRED)
FIND_LIBRARY(qhttp REQUIRED)

set(SOURCE_FILES
        allocation_dialog.cpp
        compute_allocation.cpp
        dockwidget_plots.cpp
        geometry_index.cpp
        grid_layer.cpp
        layer_panel.cpp
        main.cpp
        mainwindow.cpp
        proj_factory.cpp
        progress_dialog.cpp
        projection_dialog.cpp
        qcustomplot.cpp
        rest_server.cpp
        shapefile_layer.cpp
        spatial_stats.cpp
        spatial_stats_dialog.cpp
        trace_layer.cpp
        weighted_allocation_layer.cpp
        point_layer.cpp
        number_dialog.cpp
        gtfs_layer.cpp
        linestring_layer.cpp)

set(FORM_FILES
        dockwidget_plots.ui
        layer_panel.ui
        mainwindow.ui
        progress_dialog.ui
        projection_dialog.ui)

set(HEADER_FILES
        allocation_dialog.h
        compute_allocation.h
        constants.h
        dockwidget_plots.h
        geometries.h
        geometry_index.h
        graphicsscene.h
        graphicsview.h
        grid_layer.h
        intermediate_pos_layer.h
        layer.h
        layer_panel.h
        loader.h
        mainwindow.h
        proj_factory.h
        progress_dialog.h
        projection_dialog.h
        qcustomplot.h
        rest_server.h
        shapefile_layer.h
        spatial_stats.h
        spatial_stats_dialog.h
        trace_layer.h
        utils.h
        weighted_allocation_layer.h
        point_layer.h
        number_dialog.h
        threadlist.h
        gtfs_layer.h
        csv_parser.h
        linestring_layer.h)

add_executable(LocAll ${SOURCE_FILES}
                      ${HEADER_FILES}
                      ${FORM_FILES})

TARGET_LINK_LIBRARIES(LocAll
        Qt5::Widgets
        Qt5::Core
        Qt5::Gui
        Qt5::Concurrent
        Qt5::PrintSupport
        Qt5::Network
        proj.9
        gdal.1
        geos
        qhttp)