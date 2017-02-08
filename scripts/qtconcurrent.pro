QT += concurrent widgets
CONFIG += console
QMAKE_CXXFLAGS += -std=c++11

CONFIG   += c++11

SOURCES += qtconcurrent.cpp

target.path = .
INSTALLS += target

HEADERS += \
    progress.h
