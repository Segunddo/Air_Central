#ifndef SENDDATA_H
#define SENDDATA_H

#include <QObject>
#include <QUdpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkDatagram>
#include <QDebug>
#include <QJsonArray>
#include <QFile>

class SendData : public QObject
{
    Q_OBJECT
public:
    explicit SendData(QObject *parent = nullptr);

    void send_data(QJsonObject jsonCommand);

    Q_INVOKABLE void send_command_status(QString idEsp, QString status);

    Q_INVOKABLE void send_command_temp(QString idEsp, QString temperatura);

    Q_INVOKABLE void requireEspsId();

    Q_INVOKABLE void require_ir_read();

private:

    QJsonArray get_codes_from_file(const QString& chave);

    QUdpSocket *udpSocket;
    int numPort = 8081;
};

#endif // SENDDATA_H
