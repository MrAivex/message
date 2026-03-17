#include <QCoreApplication>
#include "server.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Server server;
    server.startServer(2323); // Выбираем любой свободный порт

    return a.exec();
}
