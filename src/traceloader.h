#ifndef TRACELOADER_H
#define TRACELOADER_H

#include "loader.h"


class TraceLoader: public Loader
{
public:
    TraceLoader(MainWindow* parent = 0, QString filename = 0, Layer* layer = 0):
        Loader(parent, filename, layer) { }
    ~TraceLoader();


private:
    bool concurrentLoad();
    void openNodeTrace(QString filename);
    void openDieselNetNodeFolder(QString dirname);
    void openDieselNetNodeTrace(QString filename, QString node);
};

#endif // TRACELOADER_H
