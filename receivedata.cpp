#include "receivedata.h"

ReceiveData::ReceiveData(QObject *parent) : QObject(parent)
{

    udpSocket = new QUdpSocket(this);

    if(udpSocket->bind(QHostAddress::AnyIPv4, numPort)) {
        qDebug() << "Servidor UDP iniciado. Escutando na porta " + std::to_string(numPort);
    } else {
        qDebug() << "Erro ao iniciar servidor UDP!";
    }

    connect(udpSocket, &QUdpSocket::readyRead,
            this, &ReceiveData::readData);
}

void ReceiveData::readData()
{
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagrama = udpSocket->receiveDatagram();
        QByteArray dadosRecebidos = datagrama.data();

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(dadosRecebidos, &error);

        if (doc.isNull() || error.error != QJsonParseError::NoError || !doc.isObject()) {
            qDebug() << "Dado recebido não é um JSON válido ou está corrompido.";
            continue;
        }

        QJsonObject jsonObject = doc.object();

        decodeData(jsonObject);
    }
}

void ReceiveData::decodeData(QJsonObject jsonObject)
{
    // Verifica o comando recebido
    QString messageType = jsonObject["command"].toString();

    if (messageType == "Resposta_ID") {

        QString idEsp = jsonObject["id"].toString();
        qDebug() << "Novo ESP descoberto na rede Mesh:" << idEsp;

        emit newDeviceDetected(idEsp);
    }

    else if (messageType == "Status_Update") {
        QString idEsp = jsonObject["id"].toString();
        QString status = jsonObject["status"].toString();
        QString temp = jsonObject["temp"].toString();
        qDebug() << "Atualização: " + idEsp + " " + status + " " + temp;
    }
}
