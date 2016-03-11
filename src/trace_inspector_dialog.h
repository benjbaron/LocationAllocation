//
// Created by Benjamin Baron on 10/03/16.
//

#ifndef LOCALL_TRACE_INSPECTOR_DIALOG_H
#define LOCALL_TRACE_INSPECTOR_DIALOG_H


#include <qdialog.h>
#include "trace.h"

class TraceInspectorDialog : public QDialog {
Q_OBJECT

public:
    TraceInspectorDialog(QWidget* parent = 0, const QString &name = "", Trace* trace = nullptr);
    QString getNodeId() const {
        return chooseNodeId->currentText();
    }

private slots:
    void done();

private:
    QComboBox* chooseNodeId;
    QDialogButtonBox* buttonBox;
    QString _name;
    QString _settingsName;
};


#endif //LOCALL_TRACE_INSPECTOR_DIALOG_H
