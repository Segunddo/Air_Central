#ifndef RECEIVEDATA_H
#define RECEIVEDATA_H

#include <QObject>
#include <QUdpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QNetworkDatagram>
#include <QDebug>

class ReceiveData : public QObject
{
    Q_OBJECT

public:
    explicit ReceiveData(QObject *parent = nullptr);

signals:
    void newDeviceDetected(QString idEsp);

private slots:
    void readData();
    void decodeData(QJsonObject obj);

private:
    QUdpSocket *udpSocket;
    int numPort = 8080;
};


#endif // RECEIVEDATA_H
