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
#include "utils.h"

class AllocationDialog : public QDialog
{
    Q_OBJECT

public:
    AllocationDialog(QWidget* parent = 0, bool showNbStorage = true, bool showDeadline = true, bool showRest = true);
    int getDeadline()          { return deadline; }
    int getNbStorageNodes()    { return nbStorageNodes; }
    double getDeletionFactor() { return deletionFactor; }
    TravelTimeStat getTravelTimeStat() {
        if(!fixedTravelTime && travelTime == -2.0) return Med;
        if(!fixedTravelTime && travelTime == -1.0) return Avg;
    }
    WithinOperator getWithinOperator() {
        if(enableTravelTime && enableDistance)   return And;
        if(enableTravelTime || enableDistance)   return Or;
        if(!enableTravelTime && !enableDistance) return None;
    }

    bool isTravelTimeEnabled() { return enableTravelTime; }
    bool isDistanceEnabled()   { return enableDistance; }
    bool isFixedTravelTime()   { return enableTravelTime && fixedTravelTime; }
    bool isMedTravelTime()     { return !fixedTravelTime && travelTime == -2.0; }
    bool isAvgTraveTime()      { return !fixedTravelTime && travelTime == -1.0; }
    bool isFixedDistance()     { return enableDistance && fixedDistance; }
    bool isAutoDistance()      { return !fixedDistance; }
    double getTravelTime()     { return travelTime; }
    double getDistance()       { return distance; }

private slots:
    void done();

private:
    QLabel *nbStorageNodesLabel;
    QLabel *deadlineLabel;
    QLabel *delFactorLabel;
    QLineEdit *nbStorageNodesLineEdit;
    QLineEdit *deadlineLineEdit;
    QLineEdit *delFactorLineEdit;

    QWidget* travelTimeExtension;
    QCheckBox* travelTimeCheckBox;
    QRadioButton *travelTimeFixedRadioButton;
    QLineEdit *travelTimeFixedLineEdit;
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
    bool showNbStorage, showDeadline, showRest;

    bool checkConsistency();
    bool toggleBoldFont(QLineEdit *lineEdit, bool isValid);
};

#endif // ALLOCATIONDIALOG_H
