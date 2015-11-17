#ifndef ALLOCATIONDIALOG_H
#define ALLOCATIONDIALOG_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDialog>

class AllocationDialog : public QDialog
{
    Q_OBJECT

public:
    AllocationDialog(QWidget* parent = 0);
    int getDeadline() { return deadline; }
    int getNbStorageNodes() { return nbStorageNodes; }

private slots:
    void done() { QDialog::accept(); }

private:
    QLabel *label1;
    QLabel *label2;
    QLineEdit *lineEdit1;
    QLineEdit *lineEdit2;
    QDialogButtonBox *buttonBox;

    int deadline = -1;
    int nbStorageNodes = -1;

    bool checkConsistency();
    bool toggleBoldFont(QLineEdit *lineEdit, bool isValid);
};

#endif // ALLOCATIONDIALOG_H
