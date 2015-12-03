#include "allocationdialog.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QSettings>

// example http://doc.qt.io/qt-5/qtwidgets-dialogs-extension-example.html

AllocationDialog::AllocationDialog(QWidget *parent, bool showNbStorage, bool showDeadline, bool showRest)
    : QDialog(parent), showNbStorage(showNbStorage), showDeadline(showDeadline), showRest(showRest)
{
    if(showNbStorage) {
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
    }

    if(showDeadline) {
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
    }

    if(showRest) {
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

        travelTimeFixedLineEdit = new QLineEdit();
        connect(travelTimeFixedLineEdit, &QLineEdit::textChanged, [=](QString text) {
            bool ok;
            travelTime = text.toDouble(&ok);
            if(!ok) travelTime = -1.0;
            else ok = travelTime > 0.0;
            toggleBoldFont(travelTimeFixedLineEdit, ok);
            checkConsistency();
        });

        travelTimeFixedRadioButton = new QRadioButton("Fixed");
        connect(travelTimeFixedRadioButton, &QRadioButton::toggled, [=](bool checked) {
            travelTimeFixedLineEdit->setEnabled(checked);
            fixedTravelTime = checked;
            qDebug() << "hello / travelTimeFixedRadioButton";
            if(checked) {
                travelTimeFixedLineEdit->setFocus();
                travelTimeFixedLineEdit->textChanged(QString::number(travelTime, 'f', 2));
            }
            checkConsistency();
        });

        QHBoxLayout *travelTimeFixedLayout = new QHBoxLayout;
        travelTimeFixedLayout->setMargin(0);
        travelTimeFixedLayout->addWidget(travelTimeFixedRadioButton);
        travelTimeFixedLayout->addWidget(travelTimeFixedLineEdit);

        travelTimeMedianRadioButton = new QRadioButton("Median travel time");
        connect(travelTimeMedianRadioButton, &QRadioButton::toggled, [=](bool checked) {
            if(checked) travelTime = -2.0;
            checkConsistency();
        });

        travelTimeAverageRadioButton = new QRadioButton("Average travel time");
        connect(travelTimeAverageRadioButton, &QRadioButton::toggled, [=](bool checked) {
            if(checked) travelTime = -1.0;
            checkConsistency();
        });

        QVBoxLayout *travelTimeExtensionLayout = new QVBoxLayout;
        travelTimeExtensionLayout->setMargin(0);
        travelTimeExtensionLayout->addLayout(travelTimeFixedLayout);
        travelTimeExtensionLayout->addWidget(travelTimeMedianRadioButton);
        travelTimeExtensionLayout->addWidget(travelTimeAverageRadioButton);
        travelTimeExtension->setLayout(travelTimeExtensionLayout);

        travelTimeCheckBox = new QCheckBox("Travel time (seconds)");
        connect(travelTimeCheckBox, &QCheckBox::toggled, [=] (bool checked) {
            enableTravelTime = checked;
            travelTimeExtension->setVisible(checked);

            checkConsistency();
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
                distanceFixedLineEdit->textChanged(QString::number(distance, 'f', 2));
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
            checkConsistency();
        });
    }

    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AllocationDialog::done);
    connect(buttonBox, &QDialogButtonBox::rejected, [=]() {
        QDialog::reject();
    });

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    if(showNbStorage) {
        mainLayout->addWidget(nbStorageNodesLabel);
        mainLayout->addWidget(nbStorageNodesLineEdit);
    }
    if(showDeadline) {
        mainLayout->addWidget(deadlineLabel);
        mainLayout->addWidget(deadlineLineEdit);
    }
    if(showRest) {
        mainLayout->addWidget(delFactorLabel);
        mainLayout->addWidget(delFactorLineEdit);
        mainLayout->addWidget(delFactorLineEdit);
        mainLayout->addWidget(travelTimeCheckBox);
        mainLayout->addWidget(travelTimeExtension);
        mainLayout->addWidget(distanceCheckBox);
        mainLayout->addWidget(distanceExtension);
    }

    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    setWindowTitle(tr("Choose parameters"));

    /* Restore previous saved values */
    QSettings settings;
    if(showNbStorage && settings.contains("savedNbStorageNodes"))
        nbStorageNodesLineEdit->setText(QString::number(settings.value("savedNbStorageNodes").toInt()));
    if(showDeadline && settings.contains("savedDeadline"))
        deadlineLineEdit->setText(QString::number(settings.value("savedDeadline").toInt()));

    if(showRest) {
        if(settings.contains("savedDeletionFactor"))
            delFactorLineEdit->setText(QString::number(settings.value("savedDeletionFactor").toDouble(), 'f', 2));

        if(settings.contains("savedEnableTravelTime")) {
            bool enabled = settings.value("savedEnableTravelTime").toBool();
            travelTimeCheckBox->setChecked(enabled);
            travelTimeCheckBox->toggled(enabled);
        } else {
            travelTimeCheckBox->setChecked(false);
            travelTimeCheckBox->toggled(false);
        }

        if(settings.contains("savedEnableDistance")) {
            bool enabled = settings.value("savedEnableDistance").toBool();
            distanceCheckBox->setChecked(enabled);
            distanceCheckBox->toggled(enabled);
        } else {
            distanceCheckBox->setChecked(false);
            distanceCheckBox->toggled(false);
        }

        if(settings.contains("savedTravelTime")) {
            travelTime = settings.value("savedTravelTime").toDouble();
            if(travelTime > 0.0) {
                travelTimeFixedRadioButton->setChecked(true);
                travelTimeFixedRadioButton->toggled(true);
            } else {
                travelTimeFixedRadioButton->toggled(false);
                if (travelTime == -1.0) {
                    travelTimeAverageRadioButton->setChecked(true);
                    travelTimeAverageRadioButton->toggled(true);
                } else if (travelTime == -2.0) {
                    travelTimeMedianRadioButton->setChecked(true);
                    travelTimeMedianRadioButton->toggled(true);
                }
            }
        } else distanceAutoRadioButton->setChecked(true);
        if(settings.contains("savedDistance")) {
            distance = settings.value("savedDistance").toDouble();
            if(distance > 0.0) {
                distanceFixedRadioButton->setChecked(true);
                distanceFixedRadioButton->toggled(true);
            } else {
                distanceFixedRadioButton->toggled(false);
                distanceAutoRadioButton->setChecked(true);
                distanceAutoRadioButton->toggled(true);
            }
        } else {
            travelTimeAverageRadioButton->setChecked(true);
            travelTimeAverageRadioButton->toggled(true);
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

    QDialog::accept();
}

bool AllocationDialog::checkConsistency()
{
    bool flag = false;
    if((showNbStorage && nbStorageNodes <= 0) || (showDeadline && deadline <= 0) || (showRest && deletionFactor < 0.0)) {
        flag = true;
        qDebug() << "deadline" << deadline << "nbStorageNodes" << nbStorageNodes << "deletionFactor" << deletionFactor;
    }
    if(showRest && enableTravelTime && fixedTravelTime && travelTime <= 0.0) {
        flag = true;
        qDebug() << "enableTravelTime" << enableTravelTime << "fixedTravelTime" << fixedTravelTime << "travelTime" << travelTime;
    }
    if(showRest && enableDistance && fixedDistance && distance <= 0.0) {
        flag = true;
        qDebug() << "enableDistance" << enableDistance << "fixedDistance" << fixedDistance << "distance" << distance;

    }

    // Disable the "OK" button dependng on the final value of flag
    buttonBox->button(QDialogButtonBox::Ok)->setDisabled(flag);
    return flag;
}

bool AllocationDialog::toggleBoldFont(QLineEdit* lineEdit, bool isValid)
{
    QFont prevFont(lineEdit->font()); // Get previous font
    if(isValid) {
        prevFont.setBold(false);
        lineEdit->setFont(prevFont);
    } else {
        prevFont.setBold(true);
        lineEdit->setFont(prevFont);
    }
    return isValid;
}


