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

FIND_LIBRARY(proj.9 REQUIRED)
FIND_LIBRARY(gdal.1 REQUIRED)
FIND_LIBRARY(qhttp REQUIRED)

set(SOURCE_FILES
        allocationdialog.cpp
        computeallocation.cpp
        dockwidgetplots.cpp
        geometryindex.cpp
        gridlayer.cpp
        layerpanel.cpp
        main.cpp
        mainwindow.cpp
        projfactory.cpp
        progressdialog.cpp
        projectiondialog.cpp
        qcustomplot.cpp
        restserver.cpp
        shapefilelayer.cpp
        spatialstats.cpp
        spatialstatsdialog.cpp
        tracelayer.cpp
        weightedallocationlayer.cpp
        PointLayer.cpp
        NumberDialog.cpp)

set(FORM_FILES
        dockwidgetplots.ui
        layerpanel.ui
        mainwindow.ui
        progressdialog.ui
        projectiondialog.ui)

set(HEADER_FILES
        allocationdialog.h
        computeallocation.h
        constants.h
        dockwidgetplots.h
        geometries.h
        geometryindex.h
        graphicsscene.h
        graphicsview.h
        gridlayer.h
        intermediateposlayer.h
        layer.h
        layerpanel.h
        loader.h
        mainwindow.h
        projfactory.h
        progressdialog.h
        projectiondialog.h
        qcustomplot.h
        restserver.h
        shapefilelayer.h
        spatialstats.h
        spatialstatsdialog.h
        tracelayer.h
        utils.h
        weightedallocationlayer.h
        PointLayer.h
        NumberDialog.h
        threadlist.h)

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
        qhttp)