#include <QCoreApplication>
#include "server.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Server server;
    server.startServer(3128); // Выбираем любой свободный порт

    return a.exec();
}
