#include "senddata.h"

SendData::SendData(QObject *parent) : QObject(parent)
{
    udpSocket = new QUdpSocket(this);
}

void SendData::send_command_status(QString idEsp, QString status)
{
    QJsonObject jsonCommand;
    jsonCommand["command"] = "Dispatch"; // Identificador para a ESP saber do que se trata
    jsonCommand["id"] = idEsp;
    jsonCommand["status"] = status;

    send_data(jsonCommand);
}

void SendData::send_command_temp(QString idEsp, QString temperatura)
{
    QJsonObject jsonCommand;
    jsonCommand["command"] = "Dispatch"; // Identificador para a ESP saber do que se trata
    jsonCommand["id"] = idEsp;
    jsonCommand["temp"] = temperatura;

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

void SendData::send_data(QJsonObject jsonCommand)
{
    // Converte o objeto JSON para um formato que possa ser enviado pela rede
    QJsonDocument doc(jsonCommand);
    QByteArray datagrama = doc.toJson(QJsonDocument::Compact);

    // Envia para a ESP Bridge (Pode continuar sendo Broadcast na rede local,
    // ou o IP fixo da ESP que está fazendo a ponte)
    udpSocket->writeDatagram(datagrama, QHostAddress::Broadcast, 8081);

    qDebug() << "Classe SendData disparou JSON:" << datagrama;
}
