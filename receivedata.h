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

#include "savedata.h"

class ReceiveData : public QObject
{
    Q_OBJECT

public:
    explicit ReceiveData(QObject *parent = nullptr);

public slots:
    void set_waited_command(QString commandType);

signals:
    void newDeviceDetected(QString idEsp);

private slots:
    void read_data();
    void decode_data(QJsonObject obj);

private:
    QUdpSocket *udpSocket;
    int numPort = 8080;

    QString waitedCommand = ""; // Começa vazio
};


#endif // RECEIVEDATA_H
