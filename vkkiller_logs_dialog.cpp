#include "vkkiller_logs_dialog.h"
#include "ui_vkkiller_logs_dialog.h"

VkKillerLogsDialog::VkKillerLogsDialog(QWidget* parent):
    QDialog (parent),
    ui      (new Ui::VkKillerLogsDialog)
{
    ui->setupUi(this);
}


VkKillerLogsDialog::~VkKillerLogsDialog() {
    delete ui;
}


void VkKillerLogsDialog::loadLogs(const QStringList& logs) noexcept {
    for (auto& entry: logs)
        ui->logsView->append(entry);
}
