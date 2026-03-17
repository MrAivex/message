#include "server.h"
#include <QDataStream>
#include <QTextStream>
#include <QDebug>
#include <QDir>

Server::Server(QObject *parent) : QTcpServer(parent) {
    loadHistory(); // Загружаем историю при старте
}

void Server::startServer(int port) {
    // QHostAddress::Any позволяет принимать подключения с любых IP-адресов (из других сетей)
    if (this->listen(QHostAddress::Any, port)) {
        qDebug() << "Сервер запущен на порту:" << port;
    } else {
        qDebug() << "Ошибка запуска сервера:" << this->errorString();
    }
}

void Server::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket *socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::readyRead, this, &Server::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Server::disconnected);

    clients.append(socket);
    qDebug() << "Новый клиент подключен. Дескриптор:" << socketDescriptor;

    // Отправляем клиенту статус (Тип 0 - системное сообщение/статус)
    sendToClient(socket, 0, "Успешное подключение к серверу.");

    // Отправляем клиенту всю историю сообщений (Тип 1 - обычное сообщение)
    for (const QString &msg : messageHistory) {
        sendToClient(socket, 1, msg);
    }
}

void Server::readyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_0); // Убедитесь, что на клиенте стоит та же версия!

    // Используем транзакции для надежного чтения TCP потока
    in.startTransaction();

    int type;
    QString message;
    in >> type >> message;

    // Если данные пришли не полностью, ждем следующего пакета
    if (!in.commitTransaction()) {
        return;
    }

    qDebug() << "Получено сообщение типа" << type << ":" << message;

    if (type == 1) { // 1 - это чат-сообщение
        messageHistory.append(message);
        saveMessageToFile(message);
        broadcastMessage(message); // Рассылаем всем
    } else if (type == 2) { // 2 - запрос статуса от клиента (Ping)
        sendToClient(socket, 0, "Сервер активен (Pong)");
    }
}

void Server::disconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    qDebug() << "Клиент отключился.";
    clients.removeOne(socket);
    socket->deleteLater();
}

void Server::sendToClient(QTcpSocket *socket, int type, const QString &text) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    // Записываем тип сообщения и сам текст
    out << type << text;
    socket->write(data);
}

void Server::broadcastMessage(const QString &msg) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << 1 << msg; // Тип 1 - обычное сообщение

    for (QTcpSocket *client : qAsConst(clients)) {
        client->write(data);
    }
}

void Server::loadHistory() {
    QFile file("chat_history.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            messageHistory.append(in.readLine());
        }
        file.close();
        qDebug() << "История загружена. Сообщений:" << messageHistory.size();
    }
}

void Server::saveMessageToFile(const QString &msg) {
    QFile file("chat_history.txt");
    // Открываем файл в режиме добавления
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << msg << "\n";
        file.close();
    }
}
