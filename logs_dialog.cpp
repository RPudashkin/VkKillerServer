#include "logs_dialog.h"
#include "ui_logs_dialog.h"


LogsDialog::LogsDialog(QWidget* parent):
    QDialog (parent),
    ui      (new Ui::LogsDialog)
{
    ui->setupUi(this);
}


LogsDialog::~LogsDialog() {
    delete ui;
}


void LogsDialog::loadLogs(const QStringList& logs) noexcept {
    for (auto& entry: logs)
        ui->logsView->append(entry);
}