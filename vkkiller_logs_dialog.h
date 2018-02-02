#ifndef VKKILLER_LOGS_DIALOG_H
#define VKKILLER_LOGS_DIALOG_H

#include <QDialog>

namespace Ui {
    class VkKillerLogsDialog;
}


class VkKillerLogsDialog: public QDialog {
    Q_OBJECT

public:
    explicit VkKillerLogsDialog(QWidget* parent = nullptr);
    ~VkKillerLogsDialog();

    void loadLogs(const QStringList& logs) noexcept;

private:
    Ui::VkKillerLogsDialog* ui;
};

#endif // VKKILLER_LOGS_DIALOG_H
