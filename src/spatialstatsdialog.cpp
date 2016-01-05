#include "spatialstatsdialog.h"

#include <QVBoxLayout>
#include <QSettings>
#include <QFileDialog>
#include "tracelayer.h"

SpatialStatsDialog::SpatialStatsDialog(QWidget *parent, TraceLayer *traceLayer):
    QDialog(parent), _traceLayer(traceLayer), minStartTime(traceLayer->getStartTime()), maxEndTime(traceLayer->getEndTime())
{
    geometryTypeExtension = new QWidget;
    geometryTypeLabel = new QLabel("Choose geometry type");

    /* Cells */
    geometryCellsRadioButton = new QRadioButton("Cells");
    connect(geometryCellsRadioButton, &QRadioButton::toggled, [=](bool checked) {
        if(checked) geometryType = CellType;
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
        if(checked) geometryType = CircleType;
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
        if(geometryType == CellType) {
            geometryCellsRadioButton->setChecked(true);
            geometryCirclesExtension->setVisible(false);
        } else if(geometryType == CircleType) {
            geometryCirclesRadioButton->setChecked(true);
            geometryCirclesExtension->setVisible(true);
        }
    } else {
        geometryCirclesExtension->setVisible(false);
    }

    // get the attributes per traceLayer name
    traceLayerSettingName = QString("savedSpatialStatsTraceLayer%1").arg(_traceLayer->getName());
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

GeometryIndex *SpatialStatsDialog::getGeometryIndex()
{
    // build the geometry index
    QSet<Geometry*> geometries;
    // build the cells from the trace layer
    auto nodes = _traceLayer->getNodes();
    QSet<QPoint> cellGeometries;
    for(auto it = nodes.begin(); it != nodes.end(); ++it) {
        auto jt = (startTime == -1) ? it.value()->begin() : it.value()->lowerBound(startTime);
        long long prevTimestamp = jt.key(); // previous timestamp
        QPointF prevPos = jt.value();       // previous position
        for(jt++; jt != it.value()->end(); ++jt) {
            // start from the second position
            long long timestamp = jt.key(); // current timestamp
            QPointF pos = jt.value();       // current position

            // number of intermediate positions (with the sampling)
            int nbPos = qMax(1,(int) qCeil((timestamp - prevTimestamp) / sampling));
            for(int i = 1; i <= nbPos; ++i) {
                long long t = prevTimestamp + i*sampling; // get the sampling time
                QPointF p = (timestamp - t)*prevPos + (t - prevTimestamp)*pos;
                p /= (timestamp - prevTimestamp);

                if(endTime != -1 && t > endTime) break;

                QPoint cellIdx((int)qFloor(p.x() / geometryCellsSize), (int)qFloor(p.y() / geometryCellsSize));
                if(!cellGeometries.contains(cellIdx)) {
                    Geometry* geom = new Cell(cellIdx.x()*geometryCellsSize, cellIdx.y()*geometryCellsSize, geometryCellsSize);
                    cellGeometries.insert(cellIdx);
                    geometries.insert(geom);
                }

                prevPos = pos;
                prevTimestamp = timestamp;
            }
        }
    }

    // add the circles from the given file
    if(geometryType == CircleType) {
        // build the circles from the given file
        QFile* file = new QFile(geometryCirclesFile);
        if(!file->open(QFile::ReadOnly | QFile::Text))
            return 0;
        while(!file->atEnd()) {
            // line format: "x;y;radius"
            QString line = QString(file->readLine()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
            QStringList fields = line.split(";");
            double x = fields.at(0).toDouble();
            double y = fields.at(1).toDouble();
            double radius = fields.at(2).toDouble();
            Geometry* geom = new Circle(x,y,radius);
            geometries.insert(geom);
        }
    }
    GeometryIndex* geometryIndex = new GeometryIndex(geometries, geometryCellsSize);
    return geometryIndex;
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
    if(geometryType == NoneType)
        flag = true;
    if(geometryType == CellType && geometryCellsSize <= 0.0)
        flag = true;
    if(geometryType == CircleType && geometryCirclesFile.isEmpty())
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


