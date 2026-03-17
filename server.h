#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QFile>

class Server : public QTcpServer {
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    void startServer(int port);

protected:
    // Вызывается при новом подключении
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void readyRead();
    void disconnected();

private:
    QVector<QTcpSocket*> clients;
    QVector<QString> messageHistory;

    void loadHistory();
    void saveMessageToFile(const QString &msg);
    void sendToClient(QTcpSocket *socket, int type, const QString &text);
    void broadcastMessage(const QString &msg);
};

#endif // SERVER_H
