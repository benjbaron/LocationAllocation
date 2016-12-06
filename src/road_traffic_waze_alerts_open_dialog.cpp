//
// Created by Benjamin Baron on 28/11/2016.
//

#include "road_traffic_waze_alerts_open_dialog.h"
#include "ui_road_traffic_open_dialog.h"


RoadTrafficWazeAlertsOpenDialog::RoadTrafficWazeAlertsOpenDialog(QWidget* parent, const QString& name):
        RoadTrafficOpenDialog(parent, name) {

    QHBoxLayout* wazeLayout     = new QHBoxLayout();
    QLabel*      wazeLabel      = new QLabel();
    QLineEdit*   wazeLineEdit   = new QLineEdit();
    QPushButton* wazePushButton = new QPushButton();

    wazeLabel->setText("Browse the Waze alert data");
    wazePushButton->setText("Browse");
    wazeLabel->setBuddy(wazeLineEdit);

    wazeLayout->addWidget(wazeLineEdit);
    wazeLayout->addWidget(wazePushButton);

    QFrame *line = new QFrame(this);
    line->setObjectName(QStringLiteral("line"));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    ui->verticalLayout->addWidget(line);
    ui->verticalLayout->addWidget(wazeLabel);
    ui->verticalLayout->addLayout(wazeLayout);

    connect(wazeLineEdit, &QLineEdit::textChanged, [=](QString text) {
        // record the file path
        _wazeAlertFilePath = text;
    });
}

bool RoadTrafficWazeAlertsOpenDialog::checkConsistency() {
    bool flag = RoadTrafficOpenDialog::checkConsistency();

    if(flag) {
        return flag;
    } else {
        if(_wazeAlertFilePath.isEmpty()) {
            flag = true;
        }

        // Disable the "OK" button depending on the final value of flag
        ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(flag);
        return flag;
    }
}

void RoadTrafficWazeAlertsOpenDialog::saveFields() {
    RoadTrafficOpenDialog::saveFields();
    /* Save the waze alert file */
    QSettings settings;
    QString settingWazeAlertPath = "savedRoadTrafficWazeAlertsOpenDialog-"+_name+"-waze";
    settings.setValue(settingWazeAlertPath, _wazeAlertFilePath);
}