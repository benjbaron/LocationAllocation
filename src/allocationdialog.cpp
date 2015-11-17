#include "allocationdialog.h"

#include <QVBoxLayout>

AllocationDialog::AllocationDialog(QWidget *parent)
    : QDialog(parent)
{
    label1 = new QLabel("Number of storage nodes:");
    lineEdit1 = new QLineEdit;
    label1->setBuddy(lineEdit1);

    connect(lineEdit1, &QLineEdit::textEdited, this, [=](QString text){
        bool ok;
        nbStorageNodes = text.toInt(&ok);
        if(!ok) nbStorageNodes = -1;
        toggleBoldFont(lineEdit1, ok);
        checkConsistency();
    });

    label2 = new QLabel("Deadline:");
    lineEdit2 = new QLineEdit;
    label2->setBuddy(lineEdit2);
    connect(lineEdit2, &QLineEdit::textEdited, this, [=](QString text){
        bool ok;
        deadline = text.toInt(&ok);
        if(!ok) deadline = -1;
        toggleBoldFont(lineEdit2, ok);
        checkConsistency();
    });


    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &AllocationDialog::done);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(label1);
    mainLayout->addWidget(lineEdit1);
    mainLayout->addWidget(label2);
    mainLayout->addWidget(lineEdit2);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    setWindowTitle(tr("Choose parameters"));

    checkConsistency();
}

bool AllocationDialog::checkConsistency()
{
    bool flag = false;
    if(deadline <= 0 && nbStorageNodes <= 0) {
        flag = true;
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


