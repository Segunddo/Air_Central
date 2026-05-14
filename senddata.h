#ifndef SENDDATA_H
#define SENDDATA_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QJsonArray>
#include <QFile>
#include <QTimer>

class SendData : public QObject
{
    Q_OBJECT
public:
    explicit SendData(QSerialPort *portaSerial, QObject *parent = nullptr);

    void send_data(QJsonObject jsonCommand);

    Q_INVOKABLE void send_command_status(QString idEsp, QString status);
    Q_INVOKABLE void send_command_temp(QString idEsp, QString temperatura);
    Q_INVOKABLE void requireEspsId();
    Q_INVOKABLE void require_ir_read();
    Q_INVOKABLE void require_espID_change(QString idEsp, QString newId);


signals:
    void errorOccurred(QString errorMessage);

private:
    // NOVO: envia todos os códigos de um array em sequência com delay
    void send_all_codes(QJsonObject baseCommand, QJsonArray codigos);

    QJsonArray get_codes_from_file(const QString& chave);

    QSerialPort *serial;
};

#endif // SENDDATA_H