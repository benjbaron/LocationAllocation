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
#include "compute_allocation.h"

class AllocationDialog : public QDialog {
    Q_OBJECT

public:
    AllocationDialog(QWidget* parent = 0);
    int getDeadline()                  const { return deadline; }
    int getNbStorageNodes()            const { return nbStorageNodes; }
    double getDeletionFactor()         const { return deletionFactor; }
    TravelTimeStat getTravelTimeStat() const { return ttStat; }
    DistanceStat getDistanceStat()     const { return dStat; }

    WithinOperator getWithinOperator() const {
        if(enableTravelTime && enableDistance)   return And;
        else if(enableTravelTime || enableDistance)   return Or;
        else return NoneWithin;
    }

    bool isTravelTimeEnabled() const { return enableTravelTime; }
    bool isDistanceEnabled()   const { return enableDistance; }
    bool isMedTravelTime()     const { return ttStat == Med; }
    bool isAvgTraveTime()      const { return ttStat == Avg; }
    bool isAutoDistance()      const { return dStat == Auto; }
    double getTravelTime()     const { return travelTime; }
    double getDistance()       const { return distance; }
    QString getMethod()        const { return method; }

    void allocationParams(AllocationParams* params) {
        params->deadline = deadline;
        params->nbFacilities = nbStorageNodes;
        params->delFactor = deletionFactor;
        params->ttStat = ttStat;
        params->dStat = dStat;
        params->travelTime = travelTime;
        params->distance = distance;
        params->method = method;
        params->computeAllStorageNodes = allStorageNodesPath;
    }

private slots:
    void done();

private:
    QLabel* methodChoiceLabel;
    QComboBox* methodChoiceComboBox;

    QWidget* storageNodeExtension;

    QWidget* nbStorageExtension;
    QRadioButton* nbStorageRadioButton;
    QLabel *nbStorageNodesLabel;
    QLineEdit *nbStorageNodesLineEdit;

    QWidget* allStorageExtension;
    QRadioButton* allStorageRadioButton;
    QLabel* allStorageNodesLabel;
    QLineEdit* allStorageNodesLineEdit;
    QPushButton* allStorageNodesFileButton;

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
    bool enableSetNbStorageNode = true;
    bool enableSetAllStorageNode = false;
    bool showNbStorage, showDeadline, showDel;
    QString method;
    QString allStorageNodesPath;
    TravelTimeStat ttStat;
    DistanceStat dStat;

    bool checkConsistency();
};

#endif // ALLOCATIONDIALOG_H
