#include "projection_dialog.h"
#include "ui_projection_dialog.h"

#include <QPushButton>
#include <QSettings>

#include "proj_factory.h"
#include <QDebug>

ProjectionDialog::ProjectionDialog(QWidget* parent, QString filename, QString projOut) :
        QDialog(parent),
        ui(new Ui::ProjectionDialog),
        _filename(filename),
        _projOut(projOut) {

    ui->setupUi(this);
    this->setWindowTitle("Set projection for " + _filename);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    // Add the stored projections in the combo boxes
    QSettings settings;
    _projIns = settings.value("savedProjIns", QStringList()).toStringList();
    if(_projIns.count() > 0) {
        ui->comboBox_projIn->addItems(_projIns);
    }
    _projIn = ui->comboBox_projIn->currentText();
    _isProjInValid = toggleBoldFont(ui->comboBox_projIn, ProjFactory::isValidProj(_projIn));

    if(_projOut.isEmpty()) {
        _projOuts = settings.value("savedProjOuts", QStringList()).toStringList();
        if(_projOuts.count() > 0) {
            ui->comboBox_projOut->addItems(_projOuts);
        }
        _projOut = ui->comboBox_projOut->currentText();
    } else {
        ui->comboBox_projOut->setCurrentText(_projOut);
        ui->comboBox_projOut->setDisabled(true);
    }
    _isProjOutValid = toggleBoldFont(ui->comboBox_projOut, ProjFactory::isValidProj(_projOut));

    // connect the combo boxes signals
    connect(ui->comboBox_projIn, &QComboBox::editTextChanged, this, &ProjectionDialog::projInEdited);
    connect(ui->comboBox_projOut, &QComboBox::editTextChanged, this, &ProjectionDialog::projOutEdited);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ProjectionDialog::onAccepted);

    checkConsistency();
}

ProjectionDialog::~ProjectionDialog() {
    delete ui;
}

void ProjectionDialog::getProjections(QString* projIn, QString* projOut) {
    QString pIn  = ui->comboBox_projIn->currentText();
    QString pOut = ui->comboBox_projOut->currentText();

    qDebug() << ProjFactory::areSameProj(pIn, pOut) << pIn << pOut;

    if(!ProjFactory::areSameProj(pIn, pOut)) {
        *projIn  = pIn;
        *projOut = pOut;
    }
}

void ProjectionDialog::projInEdited(QString text) {
    _projIn = text;
    if(!text.isEmpty()) {
        _isProjInValid = toggleBoldFont(ui->comboBox_projIn, ProjFactory::isValidProj(text));
    } else {
        _isProjInValid = true;
    }
    checkConsistency();
}

void ProjectionDialog::projOutEdited(QString text) {
    _projOut = text;
    if(!text.isEmpty()) {
        _isProjOutValid = toggleBoldFont(ui->comboBox_projOut, ProjFactory::isValidProj(text));
    } else {
        _isProjOutValid = true;
    }
    checkConsistency();
}

void ProjectionDialog::onAccepted() {
    // save the current projections
    if(!ui->comboBox_projIn->currentText().isEmpty()){
        QString projIn = ui->comboBox_projIn->currentText();
        int idx = _projIns.indexOf(projIn);
        if(idx != -1) {
            _projIns.removeAt(idx);
        }
        _projIns.push_front(projIn);
        if(_projIns.count() > 5) {
            _projIns.removeLast();
        }
    }

    if(!ui->comboBox_projOut->currentText().isEmpty()){
        QString projOut = ui->comboBox_projOut->currentText();
        int idx = _projOuts.indexOf(projOut);
        if(idx != -1) {
            _projOuts.removeAt(idx);
        }
        _projOuts.push_front(projOut);
        if(_projOuts.count() > 5) {
            _projOuts.removeLast();
        }
    }

    // save in the app settings
    QSettings settings;
    settings.setValue("savedProjIns", _projIns);
    settings.setValue("savedProjOuts", _projOuts);

    QDialog::accept();
}

bool ProjectionDialog::checkConsistency() {
    bool flag = false;
    if( (_projIn.isEmpty() && !_projOut.isEmpty()) || (!_projIn.isEmpty() && _projOut.isEmpty()) ) {
        flag = true;
    }
    if(!_isProjInValid || !_isProjOutValid) {
        flag = true;
    }
    if(_projIn.isEmpty() && _projOut.isEmpty()) {
        flag = false;
    }

    // Disable the "OK" button depending on the final value of flag
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(flag);
    return flag;
}

bool ProjectionDialog::toggleBoldFont(QComboBox* combo, bool isValid) {
    QFont prevFont(combo->font()); // Get previous font
    if(isValid) {
        prevFont.setBold(false);
        combo->setFont(prevFont);
    } else {
        prevFont.setBold(true);
        combo->setFont(prevFont);
    }
    return isValid;
}

