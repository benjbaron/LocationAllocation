#ifndef LOADER_H
#define LOADER_H

#include <QtConcurrent>
#include <QString>
#include <QObject>

#include "progress_dialog.h"
#include "mainwindow.h"


class Loader: public QObject {
    Q_OBJECT
public:
    Loader() { }

    template<typename Class, typename ...A1, typename ...A2>
    void load(Class* object, bool (Class::*f)(A1...), A2&&... args) {
//    void load(Class* object, std::function<bool (Class&, A...)> f, A... args) {
        _loadResult = QtConcurrent::run(object, f, std::forward<A2>(args)...);
    }

signals:
    void loadProgressChanged(qreal, QString);

protected:
    QFuture<bool> _loadResult;
};

class ProgressConsole: public QObject {
    Q_OBJECT

public:
    ProgressConsole(QString loadingText = "Loading") :
        _loadingText(loadingText) { }

public slots:
    void updateProgress(qreal value, const QString& text){
        printConsoleProgressBar(value, text);
    };

private:
    QString _loadingText;
};

#endif // LOADER_H
