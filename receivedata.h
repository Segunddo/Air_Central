#ifndef RECEIVEDATA_H
#define RECEIVEDATA_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include <QSerialPort>

#include "savedata.h"

class ReceiveData : public QObject
{
    Q_OBJECT

public:
    explicit ReceiveData(QObject *parent = nullptr);

    QSerialPort* getSerialPort() { return serialPort; }

    Q_INVOKABLE bool conectar(QString nomePorta);

public slots:
    void set_waited_command(QString commandType);

signals:
    void newDeviceDetected(QString idEsp);

private slots:
    void read_data();
    void decode_data(QJsonObject obj);

private:
    QSerialPort *serialPort;
    QByteArray buffer;
    QString waitedCommand = ""; // Começa vazio
};


#endif // RECEIVEDATA_H
