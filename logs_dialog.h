#ifndef LOGS_DIALOG_H
#define LOGS_DIALOG_H

#include <QDialog>

namespace Ui {
    class LogsDialog;
}


class LogsDialog: public QDialog {
    Q_OBJECT

public:
    explicit LogsDialog(QWidget* parent = nullptr);
    ~LogsDialog();

    void loadLogs(const QStringList& logs) noexcept;

private:
    Ui::LogsDialog* ui;
};

#endif // LOGS_DIALOG_H