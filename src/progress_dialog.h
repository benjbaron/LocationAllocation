#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = 0, QString loadingText = "Loading");
    ~ProgressDialog();

    void setLoadingText(QString text);

public slots:
    void updateProgress(qreal value);
    void changeText(QString text);

private:
    Ui::ProgressDialog *ui;
    QString _loadingText;

};

#endif // PROGRESSDIALOG_H
