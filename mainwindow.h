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
class QListWidgetItem;


class MainWindow: public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_startServer_clicked         ();
    void on_stopServer_clicked          ();
    void on_enableLogging_stateChanged  (int arg1);
    void on_close_clicked               ();

    void markClientAsOnline  (const VkKillerClient* client);
    void markClientAsOffline (const VkKillerClient* client);
    void showLogsDialog      (QModelIndex      index);
    void changeLoggingEnabled(QListWidgetItem* item);

private:
    using uPtrToServer = std::unique_ptr<VkKillerServer>;

    QVector<const VkKillerClient*> 	m_clients;
    /*
     * key   - client->id()
     * value - row at clientsList
     */
    QMap<qintptr, size_t>           m_rowsAtClientsList;
    Ui::MainWindow*                 ui;
    uPtrToServer                    m_server;
};

#endif // MAINWINDOW_H
