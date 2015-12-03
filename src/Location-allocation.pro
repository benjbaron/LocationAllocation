#-------------------------------------------------
#
# Project created by QtCreator 2015-10-08T15:37:20
#
#-------------------------------------------------

QT       += core gui concurrent printsupport

QMAKE_MAC_SDK = macosx10.11
QMAKE_CXXFLAGS += -std=c++11

QMAKE_CXXFLAGS = -mmacosx-version-min=10.8 -std=gnu0x -stdlib=libc+

CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Location-allocation
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
        progressdialog.cpp \
    osrmwrapper.cpp \
    shapefileloader.cpp \
    shapefilelayer.cpp \
    layerpanel.cpp \
    projectiondialog.cpp \
    traceloader.cpp \
    tracelayer.cpp \
    gridlayer.cpp \
    locationallocationdialog.cpp \
    allocationlayer.cpp \
    dockwidgetplots.cpp \
    qcustomplot.cpp \
    spatialstats.cpp \
    weightedallocationlayer.cpp \
    allocationdialog.cpp

HEADERS  += mainwindow.h \
    loader.h \
    progressdialog.h \
    layer.h \
    constants.h \
    graphicsview.h \
    graphicsscene.h \
    osrmwrapper.h \
    shapefileloader.h \
    shapefilelayer.h \
    layerpanel.h \
    projectiondialog.h \
    traceloader.h \
    tracelayer.h \
    gridlayer.h \
    locationallocationdialog.h \
    allocationlayer.h \
    dockwidgetplots.h \
    qcustomplot.h \
    spatialstats.h \
    intermediateposlayer.h \
    weightedallocationlayer.h \
    allocationdialog.h \
    utils.h

FORMS    += mainwindow.ui \
    progressdialog.ui \
    layerpanel.ui \
    projectiondialog.ui \
    locationallocationdialog.ui \
    dockwidgetplots.ui

unix|win32: LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/proj/4.9.1/lib/ -lproj.9

INCLUDEPATH += $$PWD/../../../../../../../usr/local/Cellar/proj/4.9.1/include
DEPENDPATH += $$PWD/../../../../../../../usr/local/Cellar/proj/4.9.1/include


unix|win32: LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/geos/3.4.2/lib/ -lgeos-3.4.2

INCLUDEPATH += $$PWD/../../../../../../../usr/local/Cellar/geos/3.4.2/include
DEPENDPATH += $$PWD/../../../../../../../usr/local/Cellar/geos/3.4.2/include


unix|win32: LIBS += -L$$PWD/../../../../../../../usr/local/Cellar/gdal/1.11.2_1/lib/ -lgdal.1

INCLUDEPATH += $$PWD/../../../../../../../usr/local/Cellar/gdal/1.11.2_1/include
DEPENDPATH += $$PWD/../../../../../../../usr/local/Cellar/gdal/1.11.2_1/include

unix: LIBS += -L$$PWD/../../../../../usr/local/Cellar/boost/1.58.0/lib/ -lboost_system

INCLUDEPATH += $$PWD/../../../../../usr/local/Cellar/boost/1.58.0/include
DEPENDPATH += $$PWD/../../../../../usr/local/Cellar/boost/1.58.0/include

unix: PRE_TARGETDEPS += $$PWD/../../../../../usr/local/Cellar/boost/1.58.0/lib/libboost_system.a


unix: LIBS += -L$$PWD/../../../../../usr/local/Cellar/boost/1.58.0/lib/ -lboost_filesystem

INCLUDEPATH += $$PWD/../../../../../usr/local/Cellar/boost/1.58.0/include
DEPENDPATH += $$PWD/../../../../../usr/local/Cellar/boost/1.58.0/include

unix: PRE_TARGETDEPS += $$PWD/../../../../../usr/local/Cellar/boost/1.58.0/lib/libboost_filesystem.a

unix: LIBS += -L$$PWD/../../../../../usr/local/Cellar/boost/1.58.0/lib/ -lboost_thread-mt

INCLUDEPATH += $$PWD/../../../../../usr/local/Cellar/boost/1.58.0/include
DEPENDPATH += $$PWD/../../../../../usr/local/Cellar/boost/1.58.0/include

unix: PRE_TARGETDEPS += $$PWD/../../../../../usr/local/Cellar/boost/1.58.0/lib/libboost_thread-mt.a

unix: LIBS += -L$$PWD/../../../../../usr/local/Cellar/boost/1.58.0/lib/ -lboost_program_options

INCLUDEPATH += $$PWD/../../../../../usr/local/Cellar/boost/1.58.0/include
DEPENDPATH += $$PWD/../../../../../usr/local/Cellar/boost/1.58.0/include

unix: PRE_TARGETDEPS += $$PWD/../../../../../usr/local/Cellar/boost/1.58.0/lib/libboost_program_options.a

unix: LIBS += -L$$PWD/../../osrm-backend/build/ -lOSRM

INCLUDEPATH += $$PWD/../../osrm-backend/include $$PWD/../../osrm-backend/third_party
DEPENDPATH += $$PWD/../../osrm-backend/include $$PWD/../../osrm-backend/third_party

unix: PRE_TARGETDEPS += $$PWD/../../osrm-backend/build/libOSRM.a

