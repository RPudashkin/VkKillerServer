#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <memory>

namespace Ui {
    class MainWindow;
}

class VkKillerServer;


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

private:
    using uPtrToServer = std::unique_ptr<VkKillerServer>;

    Ui::MainWindow* ui;
    uPtrToServer    m_server;
};

#endif // MAINWINDOW_H
