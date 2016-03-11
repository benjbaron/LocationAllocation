//
// Created by Benjamin Baron on 02/01/16.
//

#include "number_dialog.h"
#include "utils.h"

Q_DECLARE_METATYPE(QList<double>)

NumberDialog::NumberDialog(QWidget* parent, const QString& name, int nbFields) :
        QDialog(parent), _name(name) {

    mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    for(int i = 0; i < nbFields; ++i) {
        QHBoxLayout* layout = new QHBoxLayout();
        QLabel* label = new QLabel();
        QLineEdit* lineEdit = new QLineEdit();
        labels.append(label);
        lineEdits.append(lineEdit);
        _numbers.append(-1);
        label->setBuddy(lineEdit);
        connect(lineEdit, &QLineEdit::textChanged, this, [=](QString text){
            bool ok;
            double number = text.toDouble(&ok);
            int idx = lineEdits.indexOf(lineEdit);
            if(!ok) number = -1;

            _numbers[idx] = number;
            toggleBoldFont(lineEdit, ok);
            checkConsistency();
        });
        layout->addWidget(label);
        layout->addWidget(lineEdit);
        QWidget* extension = new QWidget();
        extension->setLayout(layout);
        mainLayout->addWidget(extension);
    }


    /* Button box */
    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &NumberDialog::done);
    connect(buttonBox, &QDialogButtonBox::rejected, [=]() {
        QDialog::reject();
    });

    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle("Type " + name);

    /* Restore previous saved values */
    QSettings settings;
    QString settingName = "savedNumberDialogNumber-"+name;
    if(settings.contains(settingName)) {
        QList<double> numbers = settings.value(settingName).value< QList<double> >();
        for(int i = 0; i < numbers.size(); ++i) {
            lineEdits.at(i)->setText(QString::number(numbers.at(i)));
        }
    }
    checkConsistency();
}

bool NumberDialog::checkConsistency() {
    bool flag = false;
    for(int i = 0; i < _numbers.size(); ++i) {
        if(_numbers.at(i) == -1) {
            flag = true;
            break;
        }
    }

    // Disable the "OK" button depending on the final value of flag
    buttonBox->button(QDialogButtonBox::Ok)->setDisabled(flag);
    return flag;
}

void NumberDialog::done() {
    /* Save all of the attributes */
    QSettings settings;
    settings.setValue("savedNumberDialogNumber-"+_name, QVariant::fromValue< QList<double> >(_numbers));
    QDialog::accept();
}

void NumberDialog::addField(const QString &name, int i) {
    labels.at(i)->setText(name);
}
