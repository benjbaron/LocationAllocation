#ifndef PROGRESS_H
#define PROGRESS_H
#include <QObject>
#include <QDebug>
#include <QtConcurrent>
#include <QProgressDialog>

class Progress : public QObject {
    Q_OBJECT
public:
    Progress(int iterations) {

        // Prepare the vector.
        for (int i = 0; i < iterations; ++i)
            vector.append(i);

//        QObject::connect(&futureWatcher, SIGNAL(progressValueChanged(int)), this, SLOT(setV(int)));
        QObject::connect(&futureWatcher, SIGNAL(finished()), this, SLOT(onFinished()));
        QObject::connect( &futureWatcher, SIGNAL(finished()), &loop, SLOT(quit()));


        // Create a progress dialog.
//        QProgressDialog dialog;
//        dialog.setLabelText(QString("Progressing using %1 thread(s)...").arg(QThread::idealThreadCount()));

        // Create a QFutureWatcher and connect signals and slots.
//        QObject::connect(&futureWatcher, SIGNAL(finished()), &dialog, SLOT(reset()));
//        QObject::connect(&dialog, SIGNAL(canceled()), &futureWatcher, SLOT(cancel()));
//        QObject::connect(&futureWatcher, SIGNAL(progressRangeChanged(int,int)), &dialog, SLOT(setRange(int,int)));
//        QObject::connect(&futureWatcher, SIGNAL(progressValueChanged(int)), &dialog, SLOT(setValue(int)));

        // Start the computation.

        // Display the dialog and start the event loop.
//        dialog.exec();


        // Query the future to check if was canceled.
    }

    void spin(int &iteration)
    {
        const int work = 100000 * 10000 * 40;
        volatile int v = 0;
        for (int j = 0; j < work; ++j)
            ++v;

        qDebug() << "iteration" << iteration << "in thread" << QThread::currentThreadId();
    }


public slots:
    void setV(int progress) {
        qDebug() << "progress value" << progress;
    }
    void run() {
        connect(&futureWatcher, &QFutureWatcher<void>::progressValueChanged, [=](int progress) {
            qDebug() << "progress value" << progress;
        });
        futureWatcher.setFuture(QtConcurrent::map(vector, [this](int &iter) {
            spin(iter);
        }));
        loop.exec();
        futureWatcher.waitForFinished();
    }

    void onFinished() {
        qDebug() << "finished";
        qDebug() << "Canceled?" << futureWatcher.future().isCanceled();
    }

private:
    QVector<int> vector;
    QFutureWatcher<void> futureWatcher;
    QEventLoop loop;

};

#endif // PROGRESS_H

