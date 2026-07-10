#ifndef SENDDATA_H
#define SENDDATA_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QQueue>
#include <QDebug>
#include <QJsonArray>
#include <QFile>
#include <QTimer>
#include <QDate>

class SendData : public QObject
{
    Q_OBJECT
public:
    explicit SendData(QSerialPort *portaSerial, QObject *parent = nullptr);

    void send_data(QJsonObject jsonCommand);

    Q_INVOKABLE void sinc_esp_data();
    Q_INVOKABLE void send_command_status(QString idEsp, QString status);
    Q_INVOKABLE void send_command_temp(QString idEsp, QString temperatura);
    Q_INVOKABLE void requireEspsId();
    Q_INVOKABLE void require_ir_read();
    Q_INVOKABLE void require_espID_change(const QString idEsp, const QString newId);

signals:
    void errorOccurred(QString errorMessage);

private slots:
    void process_data();

private:
    QSerialPort *serial;

    QTimer *timerFila;
    QQueue<QJsonObject> filaDeMensagens;
};

#endif // SENDDATA_H
