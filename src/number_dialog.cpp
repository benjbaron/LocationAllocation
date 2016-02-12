//
// Created by Benjamin Baron on 02/01/16.
//

#include "number_dialog.h"
#include "utils.h"

NumberDialog::NumberDialog(QWidget *parent, QString name)
    : QDialog(parent)
{
    /* Number extension */
    numberExtension = new QWidget;
    QHBoxLayout* numberLayout = new QHBoxLayout;
    numberLabel = new QLabel(name);
    numberLineEdit = new QLineEdit();
    numberLabel->setBuddy(numberLineEdit);
    connect(numberLineEdit, &QLineEdit::textChanged, this, [=](QString text) {
        bool ok;
        _number = text.toInt(&ok);
        if(!ok) _number = -1;
        toggleBoldFont(numberLineEdit, ok);
        checkConsistency();
    });
    numberLayout->addWidget(numberLabel);
    numberLayout->addWidget(numberLineEdit);
    numberExtension->setLayout(numberLayout);

    /* Button box */
    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &NumberDialog::done);
    connect(buttonBox, &QDialogButtonBox::rejected, [=]() {
        QDialog::reject();
    });

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(numberExtension);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle("Type " + name);

    /* Restore previous saved values */
    QSettings settings;
    if(settings.contains("savedNumberDialogNumber"))
        numberLineEdit->setText(settings.value("savedNumberDialogNumber").toString());
    checkConsistency();
}

bool NumberDialog::checkConsistency() {
    bool flag = false;
    if(_number == -1)
        flag = true;

    // Disable the "OK" button dependng on the final value of flag
    buttonBox->button(QDialogButtonBox::Ok)->setDisabled(flag);
    return flag;
}

void NumberDialog::done() {
    /* Save all of the attributes */
    QSettings settings;
    settings.setValue("savedNumberDialogNumber", _number);

    QDialog::accept();
}
