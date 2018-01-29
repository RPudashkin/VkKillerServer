#include <QCoreApplication>
#include "vkkiller_server.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    const int port = 1234;
    auto server = std::make_unique<VkKillerServer>();
    bool succes = server->start(QHostAddress::Any, port);

    if (!succes)
        qFatal("Could not start server: ");
    else
        qDebug() << "Server is ready!";

    return app.exec();
}
