#ifndef SENDDATA_H
#define SENDDATA_H

#include <QObject>
#include <QUdpSocket>

#include <QNetworkDatagram>
#include <QDebug>

class SendData : public QObject
{
    Q_OBJECT
public:
    explicit SendData(QObject *parent = nullptr);

    Q_INVOKABLE void sendCommand(QString idEsp, QString status, QString temperatura);

private:
    QUdpSocket *udpSocket;
    int numPort = 8081;
};

#endif // SENDDATA_H
