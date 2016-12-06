#include "spatial_stats_dialog.h"
#include "trace.h"

SpatialStatsDialog::SpatialStatsDialog(QWidget* parent, Trace* trace):
    QDialog(parent),
    _trace(trace),
    minStartTime(trace->getStartTime()),
    maxEndTime(trace->getEndTime()) {

    geometryTypeExtension = new QWidget;
    geometryTypeLabel = new QLabel("Choose ogrGeometry type");

    /* Cells */
    geometryCellsRadioButton = new QRadioButton("Cells");
    connect(geometryCellsRadioButton, &QRadioButton::toggled, [=](bool checked) {
        if(checked) geometryType = CellGeometryType;
    });

    geometryCellsExtension = new QWidget;
    QHBoxLayout* geometryCellsLayout = new QHBoxLayout;
    geometryCellsSizeLabel = new QLabel("Cell size");
    geometryCellsSizeLineEdit = new QLineEdit;
    geometryCellsSizeLabel->setBuddy(geometryCellsSizeLineEdit);
    connect(geometryCellsSizeLineEdit, &QLineEdit::textChanged, [=](QString text) {
        // record the cell size
        bool ok;
        geometryCellsSize = text.toDouble(&ok);
        if(!ok) geometryCellsSize = -1.0;
        else ok = geometryCellsSize >= 0.0;
        toggleBoldFont(geometryCellsSizeLineEdit, ok);
        bool ret = checkConsistency();
        qDebug() << "geometryCellsSize" << text << geometryCellsSize << "ok" << ok << "ret" << ret;
    });
    geometryCellsLayout->addWidget(geometryCellsSizeLabel);
    geometryCellsLayout->addWidget(geometryCellsSizeLineEdit);
    geometryCellsExtension->setLayout(geometryCellsLayout);

    /* Circles */
    geometryCirclesRadioButton = new QRadioButton("Circles");
    connect(geometryCirclesRadioButton, &QRadioButton::toggled, [=](bool checked) {
        geometryCirclesExtension->setVisible(checked);
        if(checked) geometryType = CircleGeometryType;
    });

    geometryCirclesExtension = new QWidget;
    QHBoxLayout* geometryCirclesLayout = new QHBoxLayout;
    geometryCirclesFileLabel = new QLabel("Circle file");
    geometryCirclesFileLineEdit = new QLineEdit;
    geometryCirclesFileButton = new QPushButton("Browse");
    connect(geometryCirclesFileButton, &QPushButton::clicked, [=] () {
        // Launch QFileDialog
        QFileDialog d(this, "Open a circle file");
        d.setFileMode(QFileDialog::Directory);
        QString filename = d.getOpenFileName(this,
                                             "Open a circle file",
                                             geometryCirclesFile.isEmpty()?QFileInfo(geometryCirclesFile).absolutePath():QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

        if(filename.isEmpty())
            return;

       geometryCirclesFileLineEdit->setText(filename);
    });

    geometryCirclesFileLabel->setBuddy(geometryCirclesFileLineEdit);
    connect(geometryCirclesFileLineEdit, &QLineEdit::textChanged, [=](QString text) {
        // record the file path
        geometryCirclesFile = text;
    });
    geometryCirclesLayout->addWidget(geometryCirclesFileLabel);
    geometryCirclesLayout->addWidget(geometryCirclesFileLineEdit);
    geometryCirclesLayout->addWidget(geometryCirclesFileButton);
    geometryCirclesExtension->setLayout(geometryCirclesLayout);

    QVBoxLayout* geometryTypeLayout = new QVBoxLayout;
    geometryTypeLayout->addWidget(geometryCellsExtension);
    geometryTypeLayout->addWidget(geometryTypeLabel);
    geometryTypeLayout->addWidget(geometryCellsRadioButton);
    geometryTypeLayout->addWidget(geometryCirclesRadioButton);
    geometryTypeLayout->addWidget(geometryCirclesExtension);

    geometryTypeExtension->setLayout(geometryTypeLayout);

    attributesExtension = new QWidget;
    QGridLayout* attributesLayout = new QGridLayout;
    /* Sampling */
    samplingLabel = new QLabel("Sampling (s)");
    samplingLineEdit = new QLineEdit;
    samplingLabel->setBuddy(samplingLineEdit);\
    connect(samplingLineEdit, &QLineEdit::textChanged, [=](QString text) {
        // record the sampling
        bool ok;
        sampling = text.toLongLong(&ok);
        if(!ok) sampling = -2;
        else ok = sampling >= 0;
        toggleBoldFont(samplingLineEdit, ok);
        bool ret = checkConsistency();
        qDebug() << "sampling" << text << sampling << "ok" << ok << "ret" << ret;
    });

    /* Duration label */
    durationLabel = new QLabel;
    durationLabel->setVisible(false);

    qDebug() << "min" << minStartTime << "max" << maxEndTime;
    QString timeBoundsText = "Min: " + QString::number(minStartTime) + "; max: " + QString::number(maxEndTime);
    timeBoundsLabel = new QLabel(timeBoundsText);

    /* Start time */
    startTimeLabel = new QLabel("Start time");
    startTimeLineEdit = new QLineEdit;
    startTimeLabel->setBuddy(startTimeLineEdit);
    connect(startTimeLineEdit, &QLineEdit::textChanged, [=](QString text) {
        // record the start time
        if(text.isEmpty()) {
            startTime = -1;
            if(endTime == -1) durationLabel->setVisible(false);
        } else {
            bool ok;
            startTime = text.toLongLong(&ok);
            if(startTime < minStartTime) ok = false;
            if(!ok) startTime = -2;
            else ok = startTime >= 0;
            toggleBoldFont(startTimeLineEdit, ok);

            if(!ok) durationLabel->setVisible(false);
            else {
                // update the duration label
                durationLabel->setVisible(true);
                long long realEndTime = (endTime == -1) ? maxEndTime : endTime;
                long long duration = realEndTime - startTime;
                durationLabel->setText(QString("Duration: %1 seconds").arg(duration));
                QFont font(durationLabel->font());
                if(duration < 0 || (startTime > 0 && startTime < minStartTime) || (endTime > 0 && endTime > maxEndTime)) { font.setBold(true); durationLabel->setFont(font); }
                else { font.setBold(false); durationLabel->setFont(font); }
            }
        }
        bool ret = checkConsistency();
        qDebug() << "startTime" << text << startTime << "ret" << ret;
    });

    /* End time */
    endTimeLabel = new QLabel("End time");
    endTimeLineEdit = new QLineEdit;
    endTimeLabel->setBuddy(endTimeLineEdit);
    connect(endTimeLineEdit, &QLineEdit::textChanged, [=](QString text) {
        // record the end time
        if(text.isEmpty()) {
            endTime = -1;
            if(startTime == -1) durationLabel->setVisible(false);
        } else {
            bool ok;
            endTime = text.toLongLong(&ok);
            if(endTime > maxEndTime) ok = false;
            if(!ok) endTime = -2;
            else ok = endTime >= 0;
            toggleBoldFont(endTimeLineEdit, ok);

            if(!ok) durationLabel->setVisible(false);
            else {
                // update the duration label
                durationLabel->setVisible(true);
                long long realStartTime = (startTime == -1) ? minStartTime : startTime;
                long long duration = endTime - realStartTime;
                durationLabel->setText(QString("Duration: %1 seconds").arg(duration));
                QFont font(durationLabel->font());
                if(duration < 0 || (startTime > 0 && startTime < minStartTime) || (endTime > 0 && endTime > maxEndTime)) { font.setBold(true); durationLabel->setFont(font); }
                else { font.setBold(false); durationLabel->setFont(font); }
            }
        }
        bool ret = checkConsistency();
        qDebug() << "endTime" << text << endTime << "ret" << ret;
    });

    attributesLayout->addWidget(samplingLabel,0,0);
    attributesLayout->addWidget(samplingLineEdit,0,1);
    attributesLayout->addWidget(startTimeLabel,1,0);
    attributesLayout->addWidget(startTimeLineEdit,1,1);
    attributesLayout->addWidget(endTimeLabel,2,0);
    attributesLayout->addWidget(endTimeLineEdit,2,1);
    attributesLayout->addWidget(durationLabel,3,0,1,2);
    attributesLayout->addWidget(timeBoundsLabel,4,0,1,2);
    attributesExtension->setLayout(attributesLayout);

    /* Button box */
    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SpatialStatsDialog::done);
    connect(buttonBox, &QDialogButtonBox::rejected, [=]() {
        QDialog::reject();
    });

    /* Set up main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(geometryTypeExtension);
    mainLayout->addWidget(attributesExtension);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Spatial Stats"));

    /* Restore previous saved values */
    QSettings settings;
    if(settings.contains("savedSpatialStatsGeometryCellSize"))
        geometryCellsSizeLineEdit->setText(QString::number(settings.value("savedSpatialStatsGeometryCellSize").toDouble()));
    if(settings.contains("savedSpatialStatsGeometryCirclesFile"))
        geometryCirclesFileLineEdit->setText(settings.value("savedSpatialStatsGeometryCirclesFile").toString());
    if(settings.contains("savedSpatialStatsGeometryType")) {
        geometryType = static_cast<GeometryType>(settings.value("savedSpatialStatsGeometryType").toInt());
        if(geometryType == CellGeometryType) {
            geometryCellsRadioButton->setChecked(true);
            geometryCirclesExtension->setVisible(false);
        } else if(geometryType == CircleGeometryType) {
            geometryCirclesRadioButton->setChecked(true);
            geometryCirclesExtension->setVisible(true);
        }
    } else {
        geometryCirclesExtension->setVisible(false);
    }

    // get the attributes per traceLayer name
    traceLayerSettingName = QString("savedSpatialStatsTraceLayer%1").arg(_trace->getName());
    if(settings.contains(traceLayerSettingName)) {
        auto traceLayerSettings = settings.value(traceLayerSettingName).toMap();
        if(traceLayerSettings.contains("savedSpatialStatsSampling")) {
            long long savedSampling = traceLayerSettings.value("savedSpatialStatsSampling").toLongLong();
            QString savedSamplingText = savedSampling == -1 ? "" : QString::number(savedSampling);
            samplingLineEdit->setText(savedSamplingText);
        }
        if(traceLayerSettings.contains("savedSpatialStatsStartTime")) {
            long long savedStartTime = traceLayerSettings.value("savedSpatialStatsStartTime").toLongLong();
            QString savedStartTimeText = savedStartTime == -1 ? "" : QString::number(savedStartTime);
            startTimeLineEdit->setText(savedStartTimeText);
        }
        if(traceLayerSettings.contains("savedSpatialStatsEndTime")) {
            long long savedEndTime = traceLayerSettings.value("savedSpatialStatsEndTime").toLongLong();
            QString savedEndTimeText = savedEndTime == -1 ? "" : QString::number(savedEndTime);
            endTimeLineEdit->setText(savedEndTimeText);
        }
    }

    checkConsistency();
}

void SpatialStatsDialog::done()
{
    /* Save all of the attributes */
    QSettings settings;
    settings.setValue("savedSpatialStatsGeometryType", geometryType);
    settings.setValue("savedSpatialStatsGeometryCellSize", geometryCellsSize);
    settings.setValue("savedSpatialStatsGeometryCirclesFile", geometryCirclesFile);
    QMap<QString,QVariant> traceLayerSettings;
    traceLayerSettings.insert("savedSpatialStatsSampling", QVariant(sampling));
    traceLayerSettings.insert("savedSpatialStatsStartTime", QVariant(startTime));
    traceLayerSettings.insert("savedSpatialStatsEndTime", QVariant(endTime));
    settings.setValue(traceLayerSettingName, traceLayerSettings);

    QDialog::accept();
}

bool SpatialStatsDialog::checkConsistency()
{
    bool flag = false;
    if(geometryType == NoneGeometryType)
        flag = true;
    if(geometryType == CellGeometryType && geometryCellsSize <= 0.0)
        flag = true;
    if(geometryType == CircleGeometryType && geometryCirclesFile.isEmpty())
        flag = true;
    if(!(sampling == -1 || sampling >= 0) || !(startTime == -1 || startTime >= minStartTime) || !(endTime == -1 || (endTime >= 0 && endTime <= maxEndTime)))
        flag = true;

    long long realStartTime = (startTime == -1) ? minStartTime : startTime;
    long long realEndTime =   (endTime == -1)   ? maxEndTime   : endTime;
    long long duration = realEndTime - realStartTime;
    if(duration < 0)
        flag = true;

    // Disable the "OK" button dependng on the final value of flag
    buttonBox->button(QDialogButtonBox::Ok)->setDisabled(flag);
    return flag;
}


