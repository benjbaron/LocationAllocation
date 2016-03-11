//
// Created by Benjamin Baron on 02/01/16.
//

#ifndef LOCALL_NUMBERDIALOG_H
#define LOCALL_NUMBERDIALOG_H


#include <qdialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qdialogbuttonbox.h>
#include <qboxlayout.h>

class NumberDialog : public QDialog {
    Q_OBJECT

public:
    NumberDialog(QWidget* parent = 0, const QString& name = "", int nbFields = 1);
    void addField(const QString& name, int i);
    double getNumber(int i) {
        return _numbers.at(i);
    }

private slots:
    void done();

private:
    QList<QLabel*> labels;
    QList<QLineEdit*> lineEdits;
    QDialogButtonBox* buttonBox;
    QVBoxLayout* mainLayout;

    QList<double> _numbers;
    QString _name;

    bool checkConsistency();
};


#endif //LOCALL_NUMBERDIALOG_H
