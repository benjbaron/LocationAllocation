#include "allocation_dialog.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QSettings>

// example http://doc.qt.io/qt-5/qtwidgets-dialogs-extension-example.html

AllocationDialog::AllocationDialog(QWidget *parent)
    : QDialog(parent)
{
    methodChoiceLabel = new QLabel("Choose method");
    methodChoiceComboBox = new QComboBox();
    QStringList methodChoiceItems = QStringList() << "" << LOCATION_ALLOCATION_MEHTOD_NAME << PAGE_RANK_MEHTOD_NAME << K_MEANS_MEHTOD_NAME << RANDOM_METHOD_NAME;
    methodChoiceComboBox->addItems(methodChoiceItems);
    // two overloads for QComboBox::currentIndexChanged, need to cast the right method
    // @see https://bugreports.qt.io/browse/QTBUG-30926
    connect(methodChoiceComboBox, (void (QComboBox::*)(const QString&))&QComboBox::currentIndexChanged, [=](const QString& text) {
        method = text;
        if(method.isEmpty()) {
            nbStorageExtension->setVisible(false);
            deadlineExtension->setVisible(false);
            delExtension->setVisible(false);
            showNbStorage = false;
            showDeadline = false;
            showDel = false;
        }
        if(method == LOCATION_ALLOCATION_MEHTOD_NAME || method == PAGE_RANK_MEHTOD_NAME) {
            nbStorageExtension->setVisible(true);
            deadlineExtension->setVisible(true);
            delExtension->setVisible(true);
            showNbStorage = true;
            showDeadline = true;
            showDel = true;
        } else if(method == K_MEANS_MEHTOD_NAME || method == RANDOM_METHOD_NAME) {
            nbStorageExtension->setVisible(true);
            deadlineExtension->setVisible(false);
            delExtension->setVisible(false);
            showNbStorage = true;
            showDeadline = true;
            showDel = false;
        }
        checkConsistency();
    });

    /* Storage extension */
    nbStorageExtension = new QWidget;
    QHBoxLayout *nbStorageLayout = new QHBoxLayout;
    nbStorageNodesLabel = new QLabel("Number of storage nodes:");
    nbStorageNodesLineEdit = new QLineEdit;
    nbStorageNodesLabel->setBuddy(nbStorageNodesLineEdit);

    connect(nbStorageNodesLineEdit, &QLineEdit::textChanged, this, [=](QString text){
        bool ok;
        nbStorageNodes = text.toInt(&ok);
        if(!ok) nbStorageNodes = -1;
        toggleBoldFont(nbStorageNodesLineEdit, ok);
        checkConsistency();
    });
    nbStorageLayout->addWidget(nbStorageNodesLabel);
    nbStorageLayout->addWidget(nbStorageNodesLineEdit);
    nbStorageExtension->setLayout(nbStorageLayout);

    /* Deadline extension */
    deadlineExtension = new QWidget;
    QHBoxLayout *deadlineLayout = new QHBoxLayout;
    deadlineLabel = new QLabel("Deadline:");
    deadlineLineEdit = new QLineEdit;
    deadlineLabel->setBuddy(deadlineLineEdit);
    connect(deadlineLineEdit, &QLineEdit::textChanged, this, [=](QString text){
        bool ok;
        deadline = text.toInt(&ok);
        if(!ok) deadline = -1;
        toggleBoldFont(deadlineLineEdit, ok);
        checkConsistency();
    });
    deadlineLayout->addWidget(deadlineLabel);
    deadlineLayout->addWidget(deadlineLineEdit);
    deadlineExtension->setLayout(deadlineLayout);

    /* Deletion extension */
    delExtension = new QWidget;
    QVBoxLayout *delLayout = new QVBoxLayout;
    delFactorLabel = new QLabel("Deletion factor:");
    delFactorLineEdit = new QLineEdit;
    delFactorLabel->setBuddy(delFactorLineEdit);
    connect(delFactorLineEdit, &QLineEdit::textChanged, this, [=](QString text){
        bool ok;
        deletionFactor = text.toDouble(&ok);
        if(!ok) deletionFactor = -1.0;
        else ok = deletionFactor >= 0.0;
        toggleBoldFont(delFactorLineEdit, ok);
        bool ret = checkConsistency();
        qDebug() << text << deletionFactor << "ok" << ok << "ret" << ret;
    });

    /* Travel time extension */
    travelTimeExtension = new QWidget;

    travelTimeLineEdit = new QLineEdit();
    connect(travelTimeLineEdit, &QLineEdit::textChanged, [=](QString text) {
        bool ok;
        travelTime = text.toDouble(&ok);
        if(!ok) travelTime = -1.0;
        else ok = travelTime > 0.0;
        toggleBoldFont(travelTimeLineEdit, ok);
        checkConsistency();
    });

    travelTimeMedianRadioButton = new QRadioButton("Median travel time");
    connect(travelTimeMedianRadioButton, &QRadioButton::toggled, [=](bool checked) {
        if(checked) ttStat = Med;

        checkConsistency();
    });

    travelTimeAverageRadioButton = new QRadioButton("Average travel time");
    connect(travelTimeAverageRadioButton, &QRadioButton::toggled, [=](bool checked) {
        if(checked) ttStat = Avg;
        checkConsistency();
    });

    QVBoxLayout *travelTimeExtensionLayout = new QVBoxLayout;
    travelTimeExtensionLayout->setMargin(0);
    travelTimeExtensionLayout->addWidget(travelTimeLineEdit);
    travelTimeExtensionLayout->addWidget(travelTimeMedianRadioButton);
    travelTimeExtensionLayout->addWidget(travelTimeAverageRadioButton);
    travelTimeExtension->setLayout(travelTimeExtensionLayout);

    travelTimeCheckBox = new QCheckBox("Travel time (seconds)");
    connect(travelTimeCheckBox, &QCheckBox::toggled, [=] (bool checked) {
        enableTravelTime = checked;
        travelTimeExtension->setVisible(checked);
        checkConsistency();
        if(!checked) ttStat = NoneTT;
    });

    /* Distance extension */
    distanceExtension = new QWidget;

    distanceFixedLineEdit = new QLineEdit();
    connect(distanceFixedLineEdit, &QLineEdit::textChanged, [=](QString text) {
        bool ok;
        distance = text.toDouble(&ok);
        if(!ok) distance = -1.0;
        else ok = distance > 0.0;
        toggleBoldFont(distanceFixedLineEdit, ok);
        checkConsistency();
    });
    distanceFixedRadioButton = new QRadioButton("Fixed");
    connect(distanceFixedRadioButton, &QRadioButton::toggled, [=](bool checked) {
        distanceFixedLineEdit->setEnabled(checked);
        fixedDistance = checked;
        if(checked) {
            distanceFixedLineEdit->setFocus();
            double prevDistance = distance < 0 ? distanceFixedLineEdit->text().toDouble() : distance;
            distanceFixedLineEdit->textChanged(QString::number(prevDistance, 'f', 2));
            dStat = FixedD;
        }
        checkConsistency();
    });

    QHBoxLayout *distanceFixedLayout = new QHBoxLayout;
    distanceFixedLayout->setMargin(0);
    distanceFixedLayout->addWidget(distanceFixedRadioButton);
    distanceFixedLayout->addWidget(distanceFixedLineEdit);

    distanceAutoRadioButton = new QRadioButton("Auto");
    connect(distanceAutoRadioButton, &QRadioButton::toggled, [=](bool checked) {
        if(checked) distance = -1.0;
        if(checked) dStat = Auto;
        checkConsistency();
    });

    QVBoxLayout *distanceExtensionLayout = new QVBoxLayout;
    distanceExtensionLayout->setMargin(0);
    distanceExtensionLayout->setSpacing(0);
    distanceExtensionLayout->addLayout(distanceFixedLayout);
    distanceExtensionLayout->addWidget(distanceAutoRadioButton);
    distanceExtension->setLayout(distanceExtensionLayout);

    distanceCheckBox = new QCheckBox("Distance (meters)");
    connect(distanceCheckBox, &QCheckBox::toggled, [=](bool checked) {
        enableDistance = checked;
        distanceExtension->setVisible(checked);
        if(!checked) dStat = NoneD;
        checkConsistency();
    });
    delLayout->addWidget(delFactorLabel);
    delLayout->addWidget(delFactorLineEdit);
    delLayout->addWidget(delFactorLineEdit);
    delLayout->addWidget(travelTimeCheckBox);
    delLayout->addWidget(travelTimeExtension);
    delLayout->addWidget(distanceCheckBox);
    delLayout->addWidget(distanceExtension);
    delExtension->setLayout(delLayout);

    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AllocationDialog::done);
    connect(buttonBox, &QDialogButtonBox::rejected, [=]() {
        QDialog::reject();
    });

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    mainLayout->addWidget(methodChoiceLabel);
    mainLayout->addWidget(methodChoiceComboBox);
    mainLayout->addWidget(nbStorageExtension);
    mainLayout->addWidget(deadlineExtension);
    mainLayout->addWidget(delExtension);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    setWindowTitle(tr("Choose parameters"));

    /* Restore previous saved values */
    QSettings settings;
    // restore the chosen method
    if(settings.contains("savedAllocationMethod"))
        methodChoiceComboBox->setCurrentText(settings.value("savedAllocationMethod").toString());
    else {
        qDebug() << "setting the current method" << LOCATION_ALLOCATION_MEHTOD_NAME;
        methodChoiceComboBox->setCurrentText(LOCATION_ALLOCATION_MEHTOD_NAME);
    }

    if(showNbStorage && settings.contains("savedNbStorageNodes"))
        nbStorageNodesLineEdit->setText(QString::number(settings.value("savedNbStorageNodes").toInt()));
    if(showDeadline && settings.contains("savedDeadline"))
        deadlineLineEdit->setText(QString::number(settings.value("savedDeadline").toInt()));

    qDebug() << "ShowDel" << showDel << settings.value("savedDeletionFactor") << settings.value("savedEnableDistance") << settings.contains("savedAllocationMethod");

    if(showDel) {
        if(settings.contains("savedDeletionFactor"))
            delFactorLineEdit->setText(QString::number(settings.value("savedDeletionFactor").toDouble(), 'f', 2));

        if(settings.contains("savedEnableTravelTime")) {
            bool enabled = settings.value("savedEnableTravelTime").toBool();
            travelTimeCheckBox->setChecked(enabled);
            travelTimeCheckBox->toggled(enabled);
            travelTimeExtension->setVisible(enabled);
            enableTravelTime = enabled;
            if(!enabled) ttStat = NoneTT;
        } else {
            travelTimeCheckBox->setChecked(false);
            travelTimeCheckBox->toggled(false);
            travelTimeExtension->setVisible(false);
            enableTravelTime = false;
            ttStat = NoneTT;
        }

        if(settings.contains("savedEnableDistance")) {
            bool enabled = settings.value("savedEnableDistance").toBool();
            distanceCheckBox->setChecked(enabled);
            distanceCheckBox->toggled(enabled);
            distanceExtension->setVisible(enabled);
            enableDistance = enabled;
            if(!enabled) dStat = NoneD;
        } else {
            distanceCheckBox->setChecked(false);
            distanceCheckBox->toggled(false);
            distanceExtension->setVisible(false);
            enableDistance = false;
            dStat = NoneD;
        }

        if(settings.contains("savedTravelTime")) {
            travelTime = settings.value("savedTravelTime").toDouble();
            if(travelTime > 0.0) {
                travelTimeLineEdit->setText(QString::number(travelTime));
                qDebug() << travelTime << settings.value("savedTravelTimeStat");
                if(settings.contains("savedTravelTimeStat")) {
                    ttStat = static_cast<TravelTimeStat>(settings.value("savedTravelTimeStat").toInt());
                    if(ttStat == Avg) travelTimeAverageRadioButton->setChecked(true);
                    if(ttStat == Med) travelTimeMedianRadioButton->setChecked(true);
                    else {
                        travelTimeAverageRadioButton->setChecked(true);
                        ttStat = Avg;
                    }
                } else {
                    travelTimeAverageRadioButton->setChecked(true);
                    ttStat = Avg;
                }
            } else {
                travelTimeAverageRadioButton->setChecked(true);
                ttStat = Avg;
            }
        } else {
            travelTimeAverageRadioButton->setChecked(true);
            ttStat = Avg;
        }

        if(settings.contains("savedDistance")) {
            distance = settings.value("savedDistance").toDouble();
            qDebug() <<"savedDistance" << distance << settings.value("savedDistance") << settings.value("savedDistanceStat");
            if(settings.contains("savedDistanceStat")) {
                dStat = static_cast<DistanceStat>(settings.value("savedDistanceStat").toInt());
                if(dStat == FixedD) {
                    if(distance > 0.0) {
                        distanceFixedRadioButton->setChecked(true);
                        distanceFixedLineEdit->setText(QString::number(distance));
                    }
                } else if(dStat == Auto) {
                    distanceAutoRadioButton->setChecked(true);
                } else {
                    distanceAutoRadioButton->setChecked(true);
                    dStat = Auto;
                }
            } else {
                distanceAutoRadioButton->setChecked(true);
                dStat = Auto;
            }
        } else {
            distanceAutoRadioButton->setChecked(true);
            dStat = Auto;
        }
    }
    checkConsistency();
}

void AllocationDialog::done() {
    /* Save all of the attributes */
    QSettings settings;
    settings.setValue("savedNbStorageNodes", nbStorageNodes);
    settings.setValue("savedDeadline", deadline);
    settings.setValue("savedDeletionFactor", deletionFactor);
    settings.setValue("savedEnableTravelTime", enableTravelTime);
    settings.setValue("savedEnableDistance", enableDistance);
    settings.setValue("savedTravelTime", travelTime);
    settings.setValue("savedDistance", distance);
    settings.setValue("savedTravelTimeStat", ttStat);
    settings.setValue("savedDistanceStat", dStat);

    QDialog::accept();
}

bool AllocationDialog::checkConsistency()
{
    bool flag = false;
    if(method.isEmpty())
        flag = true;
    if((showNbStorage && nbStorageNodes <= 0) || (showDeadline && deadline <= 0) || (showDel && deletionFactor < 0.0)) {
        flag = true;
        qDebug() << "deadline" << deadline << "nbStorageNodes" << nbStorageNodes << "deletionFactor" << deletionFactor;
    }
    if(showDel && enableTravelTime && travelTime <= 0.0) {
        flag = true;
        qDebug() << "enableTravelTime" << enableTravelTime << "fixedTravelTime" << fixedTravelTime << "travelTime" << travelTime;
    }
    if(showDel && enableTravelTime && (ttStat != Med && ttStat != Avg)) {
        flag = true;
    }
    if(showDel && enableDistance && dStat == FixedD && distance <= 0.0) {
        flag = true;
        qDebug() << "enableDistance" << enableDistance << "fixedDistance" << fixedDistance << "distance" << distance;

    }

    // Disable the "OK" button dependng on the final value of flag
    buttonBox->button(QDialogButtonBox::Ok)->setDisabled(flag);
    return flag;
}
