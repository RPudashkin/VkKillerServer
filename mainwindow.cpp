#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "vkkiller_server.h"


MainWindow::MainWindow(QWidget* parent):
    QMainWindow (parent),
    ui          (new Ui::MainWindow),
    m_server    (std::make_unique<VkKillerServer>())
{
    ui->setupUi(this);
    ui->label_serverStatus->setText("Server status: <font color = 'red'><b>disabled</b></font>");
}


MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_startServer_clicked() {
    //QHostAddress address = ui->lineEdit_IPv4->text();
    QHostAddress address = QHostAddress::Any;
    quint16 port = ui->spinBox_Port->value();

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
    if (arg1)
        m_server->loggingEnabled = true;
    else
        m_server->loggingEnabled = false;
}


void MainWindow::on_close_clicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "VkKiller Warning", "Are you sure?",
            QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_server->isWorking())
            m_server->stop();
        close();
    }
}
