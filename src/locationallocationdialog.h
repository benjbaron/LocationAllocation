#ifndef LOCATIONALLOCATIONDIALOG_H
#define LOCATIONALLOCATIONDIALOG_H

#include <QDialog>

// forward declarations
class Layer;

namespace Ui {
class LocationAllocationDialog;
}

class LocationAllocationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LocationAllocationDialog(QWidget *parent = 0, QString title = 0, QList<Layer*>* layers = 0);
    ~LocationAllocationDialog();

    void getParameters(Layer* &candidate, Layer* &demand, int* deadline, long long* startTime, long long* endTime, int* nbFacilities, int *cellSize);

public slots:
    void onAccepted();

private:
    void checkConsistency();

    Ui::LocationAllocationDialog *ui;
    QString _title;
    QList<Layer*>* _layers;
    Layer* _candidate;
    Layer* _demand;
};

#endif // LOCATIONALLOCATIONDIALOG_H
