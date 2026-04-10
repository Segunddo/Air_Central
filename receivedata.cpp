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

        QString tipoMensagem = jsonObject["command"].toString();

        if (tipoMensagem == "List_IDs") {
            // Supondo que a ESP Bridge devolva: {"tipo": "Lista_IDs", "nodes": ["CI101", "CI102"]}
            QJsonArray listaDeIds = jsonObject["nodes"].toArray();

            qDebug() << "Lista de nós Mesh recebida com" << listaDeIds.size() << "dispositivos.";

            for (int i = 0; i < listaDeIds.size(); i++) {
                QString idEsp = listaDeIds[i].toString();
                emit newDeviceDetected(idEsp);
            }
        }
    }
}
