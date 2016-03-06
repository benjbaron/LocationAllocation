#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QString>
#include "utils.h"

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = 0, QString loadingText = "Loading");
    ~ProgressDialog();

public slots:
    void updateProgress(qreal value, const QString& text);

private:
    Ui::ProgressDialog* ui;
    QString _loadingText;
};

#endif // PROGRESSDIALOG_H
