//
// Created by Benjamin Baron on 09/03/16.
//

#ifndef LOCALL_EXPORTDIALOG_H
#define LOCALL_EXPORTDIALOG_H


#include <qdialog.h>

class ExportDialog : public QDialog {
    Q_OBJECT
public:
    ExportDialog(QWidget* parent = 0, const QString& name = "", long long min = 0, long long max = 0);
    long long getSampling() { return _sampling; }
    long long getStartTime() { return _startTime; }
    long long getEndTime() { return _endTime; }

private slots:
    void done();

private:
    long long _sampling = -1;
    long long _startTime = -1;
    long long _endTime = -1;
    QString _name;
    QDialogButtonBox* buttonBox;

    bool checkConsistency();
};


#endif //LOCALL_EXPORTDIALOG_H
