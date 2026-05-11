#include "senddata.h"

SendData::SendData(QSerialPort *portaSerial, QObject *parent)
    : QObject(parent), serial(portaSerial)
{
}

void SendData::send_all_codes(QJsonObject baseCommand, QJsonArray codigos)
{
    if (codigos.isEmpty()) {
        qDebug() << "Nenhum código IR salvo para este comando.";
        return;
    }

    for (int i = 0; i < codigos.size(); i++) {
        QString codigo = codigos[i].toString();

        QTimer::singleShot(i * 100, this, [this, baseCommand, codigo]() {
            QJsonObject cmd = baseCommand;
            cmd["code"] = codigo;
            send_data(cmd);
        });
    }
}

void SendData::send_command_status(QString idEsp, QString status)
{
    QJsonObject baseCommand;
    baseCommand["command"] = "Dispatch";
    baseCommand["id"]      = idEsp;
    baseCommand["status"]  = status;

    QString chaveBusca = (status == "Ligado") ? "Ligar" : "Desligar";
    QJsonArray codigos = get_codes_from_file(chaveBusca);

    send_all_codes(baseCommand, codigos);
}

void SendData::send_command_temp(QString idEsp, QString temperatura)
{
    QJsonObject baseCommand;
    baseCommand["command"] = "Dispatch";
    baseCommand["id"]      = idEsp;
    baseCommand["temp"]    = temperatura;

    QJsonArray codigos = get_codes_from_file(temperatura);

    send_all_codes(baseCommand, codigos);
}

void SendData::require_espID_change(QString idEsp, QString newId)
{
    QJsonObject jsonRequest;
    jsonRequest["command"] = "Change_ID";
    jsonRequest["id"] = idEsp;
    jsonRequest["new_id"] = newId;

    send_data(jsonRequest);
}

void SendData::requireEspsId()
{
    QJsonObject jsonRequest;
    jsonRequest["command"] = "Require_IDs";
    send_data(jsonRequest);
    qDebug() << "Classe SendData requisitou IDs";
}

void SendData::require_ir_read()
{
    QJsonObject jsonRequest;
    jsonRequest["command"] = "Require_IR";
    send_data(jsonRequest);
    qDebug() << "Classe SendData requisitou leitura de infravermelho";
}

void SendData::send_data(QJsonObject jsonCommand)
{
    QJsonDocument doc(jsonCommand);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n');

    if (serial && serial->isOpen()) {
        serial->write(data);
        qDebug() << "Classe SendData disparou:" << data;
    } else {
        qDebug() << "Erro: Tentando enviar";
        emit errorOccurred("Falha ao enviar: A porta serial não está conectada ou aberta.");
    }
}

QJsonArray SendData::get_codes_from_file(const QString& chave)
{
    QJsonArray arrayCodigos;
    QFile file("codes.json");

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (!doc.isNull() && doc.isObject()) {
            QJsonObject data = doc.object();
            if (data.contains(chave)) {
                arrayCodigos = data[chave].toArray();
            }
        }
    } else {
        qDebug() << "Erro: Nao foi possivel abrir codes.json para leitura.";
    }

    return arrayCodigos;
}