#include "senddata.h"

SendData::SendData(QSerialPort *portaSerial, QObject *parent)
    : QObject(parent), serial(portaSerial)
{
}

void SendData::send_command_status(QString idEsp, QString status)
{
    QJsonObject jsonCommand;
    jsonCommand["command"] = "Dispatch";
    jsonCommand["id"] = idEsp;
    jsonCommand["status"] = status;

    QString chaveBusca = (status == "Ligado") ? "Ligar" : "Desligar";

    // Pegamos o array de códigos salvos
    QJsonArray arrayCodigos = get_codes_from_file(chaveBusca);

    if (!arrayCodigos.isEmpty()) {
        // Pegamos apenas o código mais recente (último do array)
        // e enviamos como STRING, que é mais leve para a ESP
        jsonCommand["code"] = arrayCodigos.last().toString();
    }

    send_data(jsonCommand);
}

void SendData::send_command_temp(QString idEsp, QString temperatura)
{
    QJsonObject jsonCommand;
    jsonCommand["command"] = "Dispatch"; // Identificador para a ESP saber do que se trata
    jsonCommand["id"] = idEsp;
    jsonCommand["temp"] = temperatura;

    QJsonArray arrayCodigos = get_codes_from_file(temperatura);

    if (!arrayCodigos.isEmpty()) {
        // Pegamos apenas o código mais recente (último do array)
        // e enviamos como STRING, que é mais leve para a ESP
        jsonCommand["code"] = arrayCodigos.last().toString();
    }

    send_data(jsonCommand);
}

void SendData::requireEspsId()
{
    // Apenas pede para a ESP Bridge enviar a lista de nós conhecidos pela Mesh
    QJsonObject jsonRequest;
    jsonRequest["command"] = "Require_IDs";

    send_data(jsonRequest);

    qDebug() << "Classe SendData requisitou IDs";
}

void SendData::require_ir_read()
{
    QJsonObject jsonRequest;
    jsonRequest["command"] = "Require_IR"; // Nome do comando que sua ESP entende para entrar em modo leitura

    send_data(jsonRequest);
    qDebug() << "Classe SendData requisitou leitura de infravermelho";
}

void SendData::send_data(QJsonObject jsonCommand)
{
    // Converte o objeto JSON para um formato que possa ser enviado pela rede
    QJsonDocument doc(jsonCommand);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    data.append('\n');

    // Se a porta estiver aberta e válida, envia os dados!
    if(serial && serial->isOpen()) {
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
