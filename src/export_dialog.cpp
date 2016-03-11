//
// Created by Benjamin Baron on 09/03/16.
//

#include <qboxlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qdialogbuttonbox.h>
#include <qsettings.h>
#include <QPushButton>
#include "export_dialog.h"
#include "utils.h"

ExportDialog::ExportDialog(QWidget *parent, const QString &name, long long min, long long max):
        QDialog(parent), _name(name) {

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    QWidget*     samplingExtension  = new QWidget;
    QHBoxLayout* samplingLayout     = new QHBoxLayout();
    QLabel*      samplingLabel      = new QLabel();
    QLineEdit*   samplingLineEdit   = new QLineEdit();
    QWidget*     startTimeExtension = new QWidget;
    QHBoxLayout* startTimeLayout    = new QHBoxLayout();
    QLabel*      startTimeLabel     = new QLabel();
    QLineEdit*   startTimeLineEdit  = new QLineEdit();
    QWidget*     endTimeExtension   = new QWidget;
    QHBoxLayout* endTimeLayout      = new QHBoxLayout();
    QLabel*      endTimeLabel       = new QLabel();
    QLineEdit*   endTimeLineEdit    = new QLineEdit();
    QLabel*      textLabel          = new QLabel();
    QLabel*      durationLabel      = new QLabel();

    samplingLabel->setText("Sampling");
    startTimeLabel->setText("Start time");
    endTimeLabel->setText("End time");
    textLabel->setText("Min time "+QString::number(min)+"; max time "+QString::number(max));
    durationLabel->setText("Duration "+QString::number(max-min));

    samplingLabel->setBuddy(samplingLineEdit);
    startTimeLabel->setBuddy(startTimeLineEdit);
    endTimeLabel->setBuddy(endTimeLineEdit);

    samplingLayout->addWidget(samplingLabel);
    samplingLayout->addWidget(samplingLineEdit);
    startTimeLayout->addWidget(startTimeLabel);
    startTimeLayout->addWidget(startTimeLineEdit);
    endTimeLayout->addWidget(endTimeLabel);
    endTimeLayout->addWidget(endTimeLineEdit);

    samplingExtension->setLayout(samplingLayout);
    startTimeExtension->setLayout(startTimeLayout);
    endTimeExtension->setLayout(endTimeLayout);

    mainLayout->addWidget(samplingExtension);
    mainLayout->addWidget(startTimeExtension);
    mainLayout->addWidget(endTimeExtension);
    mainLayout->addWidget(textLabel);
    mainLayout->addWidget(durationLabel);

    connect(samplingLineEdit, &QLineEdit::textChanged, this, [=](QString text){
        bool ok;
        long long number = text.toLongLong(&ok);
        if(!ok) number = -1;

        _sampling = number;
        toggleBoldFont(samplingLineEdit, ok);
        checkConsistency();
    });

    connect(startTimeLineEdit, &QLineEdit::textChanged, this, [=](QString text) {
        bool ok;
        long long number = text.toLongLong(&ok);
        if(text.isEmpty()) {
            number = min;
            ok = true;
        } else if(!ok || (number < min) || (number > max )|| (_endTime != -1 && number > _endTime)) {
            number = -1;
            ok = false;
        }

        _startTime = number;

        long long maxTime = _endTime   == -1 ? max : _endTime;
        long long minTime = _startTime == -1 ? min : _startTime;

        durationLabel->setText("Duration "+QString::number(maxTime-minTime));
        toggleBoldFont(durationLabel, ok);
        toggleBoldFont(startTimeLineEdit, ok);
        checkConsistency();
    });

    connect(endTimeLineEdit, &QLineEdit::textChanged, this, [=](QString text) {
        bool ok;
        long long number = text.toLongLong(&ok);
        if(text.isEmpty()) {
            number = max;
            ok = true;
        } else if(!ok || (number < min) || (number > max )|| (_startTime != -1 && number < _startTime)) {
            number = -1;
            ok = false;
        }

        _endTime = number;

        long long maxTime = _endTime   == -1 ? max : _endTime;
        long long minTime = _startTime == -1 ? min : _startTime;

        durationLabel->setText("Duration "+QString::number(maxTime-minTime));
        toggleBoldFont(durationLabel, ok);
        toggleBoldFont(endTimeLineEdit, ok);
        checkConsistency();
    });


    /* Button box */
    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ExportDialog::done);
    connect(buttonBox, &QDialogButtonBox::rejected, [=]() {
        QDialog::reject();
    });

    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle("Type " + name);

    /* Restore previous saved values */
    QSettings settings;
    QString settingName = "savedExportDialog-"+name;
    if(settings.contains(settingName)) {
        QVariantList numbers = settings.value(settingName).toList();
        qDebug() << numbers;

        if(numbers.size() == 3) {
            samplingLineEdit->setText(QString::number(numbers.at(0).toLongLong()));
            startTimeLineEdit->setText(QString::number(numbers.at(1).toLongLong()));
            endTimeLineEdit->setText(QString::number(numbers.at(2).toLongLong()));
        }
    }
    checkConsistency();
}

void ExportDialog::done() {
    /* Save all of the attributes */
    QSettings settings;
    QVariantList numbers;
    numbers << _sampling << _startTime << _endTime;

    QString settingName = "savedExportDialog-"+_name;
    settings.setValue(settingName, numbers);
    qDebug() << numbers << settings.value(settingName).toList();
    QDialog::accept();
}

bool ExportDialog::checkConsistency() {
    bool flag = false;
    if(_sampling  == -1) flag = true;
    if(_startTime == -1) flag = true;
    if(_endTime   == -1) flag = true;

    // Disable the "OK" button depending on the final value of flag
    buttonBox->button(QDialogButtonBox::Ok)->setDisabled(flag);
    return flag;
}
