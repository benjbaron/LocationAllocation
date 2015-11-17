#include "locationallocationdialog.h"
#include "ui_locationallocationdialog.h"

#include <QPushButton>
#include <QDebug>

#include "layer.h"

LocationAllocationDialog::LocationAllocationDialog(QWidget *parent, QString title, QList<Layer*>* layers) :
    QDialog(parent),
    ui(new Ui::LocationAllocationDialog),
    _title(title),
    _layers(layers)
{
    ui->setupUi(this);
    this->setWindowTitle(title);

    // populate the comboboxes
    for(int i = 0; i < layers->size(); ++i) {
        Layer* layer = layers->at(i);
        ui->combo_candidates->addItem(layer->getName());
        ui->combo_demands->addItem(layer->getName());
    }

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onAccepted()));
    connect(ui->lineEdit_deadline, &QLineEdit::textEdited, [=](QString text) {
        Q_UNUSED(text)
        checkConsistency();
    });
    connect(ui->lineEdit_start, &QLineEdit::textEdited, [=](QString text) {
        Q_UNUSED(text)
        checkConsistency();
    });
    connect(ui->lineEdit_end, &QLineEdit::textEdited, [=](QString text) {
        Q_UNUSED(text)
        checkConsistency();
    });
    connect(ui->lineEdit_facilities, &QLineEdit::textEdited, [=](QString text) {
        Q_UNUSED(text)
        checkConsistency();
    });
    connect(ui->lineEdit_cellSize, &QLineEdit::textEdited, [=](QString text) {
        Q_UNUSED(text)
        checkConsistency();
    });
    connect(ui->checkBox_useGrid, &QCheckBox::toggled, [=](bool checked) {
        ui->combo_candidates->setDisabled(checked);
    });

}

LocationAllocationDialog::~LocationAllocationDialog()
{
    delete ui;
}

void LocationAllocationDialog::getParameters(Layer *&candidate, Layer *&demand, int *deadline, long long *startTime, long long *endTime, int *nbFacilities, int *cellSize) {
    if(ui->checkBox_useGrid->isChecked()) {
        candidate = NULL;
    } else {
        candidate = _candidate;
    }
    demand = _demand;
    *deadline = ui->lineEdit_deadline->text().toInt();
    *startTime = ui->lineEdit_start->text().toLongLong();
    *endTime = ui->lineEdit_end->text().toLongLong();
    *nbFacilities = ui->lineEdit_facilities->text().toInt();
    *cellSize = ui->lineEdit_cellSize->text().toInt();
    qDebug() << "getParameters" << _candidate->getName() << _demand->getName() << *deadline << *startTime << *endTime << *nbFacilities;
}

void LocationAllocationDialog::onAccepted()
{
    _candidate = _layers->at(ui->combo_candidates->currentIndex());
    _demand    = _layers->at(ui->combo_demands->currentIndex());
    qDebug() << "onAccepted" << _candidate->getName() << _demand->getName();
}

void LocationAllocationDialog::checkConsistency()
{
    bool flag = false;
    bool ok;
    ui->lineEdit_deadline->text().toInt(&ok);
    if(!ui->lineEdit_deadline->text().isEmpty() && !ok) flag = true;
    ui->lineEdit_start->text().toLongLong(&ok);
    if(!ui->lineEdit_start->text().isEmpty() && !ok) flag = true;
    ui->lineEdit_end->text().toLongLong(&ok);
    if(!ui->lineEdit_end->text().isEmpty() && !ok) flag = true;
    ui->lineEdit_facilities->text().toInt(&ok);
    if(!ui->lineEdit_facilities->text().isEmpty() && !ok) flag = true;
    int cellSize = ui->lineEdit_cellSize->text().toInt(&ok);
    if(!ui->lineEdit_cellSize->text().isEmpty() && !ok) flag = true;
    if(ui->checkBox_useGrid->isChecked() && cellSize <= 0) flag = true;

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(flag);
}
