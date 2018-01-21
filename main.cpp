#include <QCoreApplication>
#include "vkkiller_server.h"


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    const int port = 1234;
    VkKillerServer* server = new VkKillerServer();
    bool succes = server->start(QHostAddress::Any, port, nullptr);

    if (!succes)
        qFatal("Could not start server");
    else
        qDebug() << "Server is ready!";

    return app.exec();
}
