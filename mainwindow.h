#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QTimer>
#include <QVector>
#include <memory>


namespace Ui {
    class MainWindow;
}

class VkKillerServer;
class VkKillerClient;


class MainWindow: public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_startServer_clicked();
    void on_stopServer_clicked();
    void on_enableLogging_stateChanged(int arg1);
    void on_close_clicked();

    void addClient      (const VkKillerClient* client); // add client to cliens list
    void delClient      (const VkKillerClient* client); // delete client from clients list
    void showLogsDialog (QModelIndex index);

private:
    using uPtrToServer = std::unique_ptr<VkKillerServer>;


    QVector<const VkKillerClient*> 	m_clients;
    Ui::MainWindow*                 ui;
    uPtrToServer                    m_server;
};

#endif // MAINWINDOW_H
