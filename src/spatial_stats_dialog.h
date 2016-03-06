#ifndef SPATIALSTATSDIALOG_H
#define SPATIALSTATSDIALOG_H

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDialog>
#include <QRadioButton>
#include <QCheckBox>
#include <QWidget>
#include <QComboBox>
#include "geometry_index.h"

#include "geometries.h"

// forward class declaration
class Trace;

class SpatialStatsDialog: public QDialog {
    Q_OBJECT
public:
    SpatialStatsDialog(QWidget* parent = 0, Trace* trace = 0);
    long long getSampling() { return sampling; }
    long long getStartTime() { return startTime; }
    long long getEndTime() { return endTime; }
    GeometryType getGeometryType() { return geometryType; }
    double getCellSize() { return geometryCellsSize; }
    QString getCircleFile() { return geometryCirclesFile; }

private slots:
    void done();

private:
    QWidget* geometryTypeExtension;
    QLabel* geometryTypeLabel;
    QRadioButton* geometryCellsRadioButton;
    QRadioButton* geometryCirclesRadioButton;

    QWidget* geometryCellsExtension;
    QLabel* geometryCellsSizeLabel;
    QLineEdit* geometryCellsSizeLineEdit;

    QWidget* geometryCirclesExtension;
    QLabel* geometryCirclesFileLabel;
    QLineEdit* geometryCirclesFileLineEdit;
    QPushButton* geometryCirclesFileButton;

    QWidget* attributesExtension;
    QLabel* samplingLabel;
    QLineEdit* samplingLineEdit;
    QLabel* startTimeLabel;
    QLineEdit* startTimeLineEdit;
    QLabel* endTimeLabel;
    QLineEdit* endTimeLineEdit;
    QLabel* durationLabel;
    QLabel* timeBoundsLabel;

    QDialogButtonBox *buttonBox;

    long long sampling = -1;
    long long startTime = -1, minStartTime;
    long long endTime = -1, maxEndTime;
    QString geometryCirclesFile;
    double geometryCellsSize = -1.0;
    GeometryType geometryType = NoneType;
    Trace* _trace;
    QString traceLayerSettingName;

    /* Private functions */
    bool checkConsistency();
};

#endif // SPATIALSTATSDIALOG_H
