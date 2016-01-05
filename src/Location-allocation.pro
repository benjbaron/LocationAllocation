#-------------------------------------------------
#
# Project created by QtCreator 2015-10-08T15:37:20
#
#-------------------------------------------------

QT       += core gui concurrent printsupport network

QMAKE_MAC_SDK = macosx10.11
QMAKE_CXXFLAGS += -std=c++11

QMAKE_CXXFLAGS = -mmacosx-version-min=10.8 -std=gnu0x -stdlib=libc+

CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Location-allocation
TEMPLATE = app

SOURCES += allocationdialog.cpp \
           allocationlayer.cpp \
           computeallocation.cpp \
           dockwidgetplots.cpp \
           geometryindex.cpp \
           gridlayer.cpp \
           layerpanel.cpp \
           locationallocationdialog.cpp \
           main.cpp \
           mainwindow.cpp \
           projfactory.cpp \
           progressdialog.cpp \
           projectiondialog.cpp \
           qcustomplot.cpp \
           restserver.cpp \
           shapefilelayer.cpp \
           shapefileloader.cpp \
           spatialstats.cpp \
           spatialstatsdialog.cpp \
           tracelayer.cpp \
           traceloader.cpp \
           weightedallocationlayer.cpp \
           PointLayer.cpp \
           NumberDialog.cpp

HEADERS  += allocationdialog.h \
            allocationlayer.h \
            computeallocation.h \
            constants.h \
            dockwidgetplots.h \
            geometries.h \
            geometryindex.h \
            graphicsscene.h \
            graphicsview.h \
            gridlayer.h \
            intermediateposlayer.h \
            layer.h \
            layerpanel.h \
            loader.h \
            locationallocationdialog.h \
            mainwindow.h \
            projfactory.h \
            progressdialog.h \
            projectiondialog.h \
            qcustomplot.h \
            restserver.h \
            shapefilelayer.h \
            shapefileloader.h \
            spatialstats.h \
            spatialstatsdialog.h \
            tracelayer.h \
            traceloader.h \
            utils.h \
            weightedallocationlayer.h \
            PointLayer.h \
            NumberDialog.h \
            threadlist.h

FORMS    += mainwindow.ui \
            progressdialog.ui \
            layerpanel.ui \
            projectiondialog.ui \
            locationallocationdialog.ui \
            dockwidgetplots.ui

LIBS += -L/usr/local/lib -lproj.9 -lgdal.1 -lqhttp

INCLUDEPATH += /usr/local/include $$PWD/../qhttp/src