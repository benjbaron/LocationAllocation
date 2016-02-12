#ifndef ALLOCATIONDIALOG_H
#define ALLOCATIONDIALOG_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDialog>
#include <QRadioButton>
#include <QCheckBox>
#include <QWidget>
#include <QComboBox>
#include "utils.h"

class AllocationDialog : public QDialog
{
    Q_OBJECT

public:
    AllocationDialog(QWidget* parent = 0);
    int getDeadline()          { return deadline; }
    int getNbStorageNodes()    { return nbStorageNodes; }
    double getDeletionFactor() { return deletionFactor; }
    TravelTimeStat getTravelTimeStat() { return ttStat; }
    DistanceStat getDistanceStat() { return dStat; }

    WithinOperator getWithinOperator() {
        if(enableTravelTime && enableDistance)   return And;
        else if(enableTravelTime || enableDistance)   return Or;
        else return NoneWithin;
    }

    bool isTravelTimeEnabled() { return enableTravelTime; }
    bool isDistanceEnabled()   { return enableDistance; }
    bool isMedTravelTime()     { return ttStat == Med; }
    bool isAvgTraveTime()      { return ttStat == Avg; }
    bool isAutoDistance()      { return dStat == Auto; }
    double getTravelTime()     { return travelTime; }
    double getDistance()       { return distance; }
    QString getMethod()        { return method; }

private slots:
    void done();

private:
    QLabel* methodChoiceLabel;
    QComboBox* methodChoiceComboBox;

    QWidget* nbStorageExtension;
    QLabel *nbStorageNodesLabel;
    QLineEdit *nbStorageNodesLineEdit;

    QWidget* deadlineExtension;
    QLabel *deadlineLabel;
    QLineEdit *deadlineLineEdit;

    QWidget* delExtension;
    QLabel *delFactorLabel;
    QLineEdit *delFactorLineEdit;

    QWidget* travelTimeExtension;
    QCheckBox* travelTimeCheckBox;
    QLineEdit *travelTimeLineEdit;
    QRadioButton *travelTimeMedianRadioButton;
    QRadioButton *travelTimeAverageRadioButton;

    QWidget* distanceExtension;
    QCheckBox* distanceCheckBox;
    QRadioButton *distanceAutoRadioButton;
    QRadioButton *distanceFixedRadioButton;
    QLineEdit *distanceFixedLineEdit;

    QDialogButtonBox *buttonBox;

    int deadline = -1;
    int nbStorageNodes = -1;
    double deletionFactor = -1.0;
    double distance = -1.0;
    double travelTime = -1.0;
    bool fixedDistance = false;
    bool fixedTravelTime = false;
    bool enableDistance = false;
    bool enableTravelTime = false;
    bool showNbStorage, showDeadline, showDel;
    QString method;
    TravelTimeStat ttStat;
    DistanceStat dStat;

    bool checkConsistency();
};

#endif // ALLOCATIONDIALOG_H
