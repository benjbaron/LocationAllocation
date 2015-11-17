#ifndef PROJECTIONDIALOG_H
#define PROJECTIONDIALOG_H

#include <QDialog>
#include <QComboBox>

namespace Ui {
class ProjectionDialog;
}

class ProjectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectionDialog(QWidget *parent = 0, QString filename = 0, QString projOut = 0);
    ~ProjectionDialog();
    void getProjections(QString* projIn, QString *projOut);

private slots:
    void projInEdited(QString text);
    void projOutEdited(QString text);
    void onAccepted();

private:
    bool checkConsistency();
    bool isValidProj(QString proj);
    bool toggleBoldFont(QComboBox * combo, bool isValid);

    Ui::ProjectionDialog *ui;
    QString _filename;
    QString _projIn = "", _projOut = "";
    QStringList _projIns, _projOuts;
    bool _isProjInValid = false, _isProjOutValid = false;
};

#endif // PROJECTIONDIALOG_H
