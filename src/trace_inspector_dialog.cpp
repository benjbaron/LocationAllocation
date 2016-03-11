//
// Created by Benjamin Baron on 10/03/16.
//

#include "trace_inspector_dialog.h"

TraceInspectorDialog::TraceInspectorDialog(QWidget *parent, const QString &name, Trace *trace):
        QDialog(parent), _name(name), _settingsName("savedTraceInspector-"+name) {

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    chooseNodeId = new QComboBox();
    QLabel* chooseLabel = new QLabel("Choose a node");

    QHash<QString, QMap<long long, QPointF>*> nodes;
    trace->getNodes(&nodes);
    for(QString nodeId : nodes.keys()) {
        chooseNodeId->addItem(nodeId);
    }

    /* Button box */
    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TraceInspectorDialog::done);
    connect(buttonBox, &QDialogButtonBox::rejected, [=]() {
        QDialog::reject();
    });

    mainLayout->addWidget(chooseLabel);
    mainLayout->addWidget(chooseNodeId);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(_name);

    QSettings settings;
    if(settings.contains(_settingsName))
        chooseNodeId->setCurrentText(settings.value(_settingsName).toString()); }

void TraceInspectorDialog::done() {
    QSettings settings;
    settings.setValue(_settingsName, chooseNodeId->currentText());

    QDialog::accept();
}

