#include "progress_dialog.h"
#include "ui_progress_dialog.h"

ProgressDialog::ProgressDialog(QWidget *parent, QString loadingText) :
        QDialog(parent),
        ui(new Ui::ProgressDialog),
        _loadingText(loadingText)
{
    ui->setupUi(this);
    ui->label->setText(loadingText);
    ui->labelProgressBar->setText("0 %");
    ui->progressBar->setValue(0);
    setWindowFlags(Qt::Sheet);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::setLoadingText(QString text)
{
    ui->label->setText(text);
}

void ProgressDialog::updateProgress(qreal value)
{
    ui->labelProgressBar->setText(QString::number((int)(value*100)) + " %");
    ui->progressBar->setValue((int)(value*100));
    if(value == 1.0) {
        accept();
    }
}

void ProgressDialog::changeText(QString text) {
    ui->label->setText(_loadingText + "\n" + text);
}