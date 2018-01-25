#include <QCoreApplication>
#include "vkkiller_server.h"


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    const int port = 1234;
    VkKillerServer* server = new VkKillerServer();
    QString errMsg;
    bool succes = server->start(QHostAddress::Any, port, &errMsg);

    if (!succes) {
        qFatal("Could not start server: ");
        qFatal(errMsg.toStdString().c_str());
    }
    else
        qDebug() << "Server is ready!";

    return app.exec();
}
