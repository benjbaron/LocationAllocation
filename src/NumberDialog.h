//
// Created by Benjamin Baron on 02/01/16.
//

#ifndef LOCALL_NUMBERDIALOG_H
#define LOCALL_NUMBERDIALOG_H


#include <qdialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qdialogbuttonbox.h>

class NumberDialog : public QDialog
{
    Q_OBJECT

public:
    NumberDialog(QWidget* parent = 0, QString name = "");
    int getNumber() { return _number; }

private slots:
    void done();

private:
    QLabel*numberLabel;
    QLineEdit* numberLineEdit;
    QWidget* numberExtension;
    QDialogButtonBox *buttonBox;

    int _number = -1;

    bool checkConsistency();
};


#endif //LOCALL_NUMBERDIALOG_H
