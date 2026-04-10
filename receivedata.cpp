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
        QString messenger = QString::fromUtf8(datagrama.data());

        qDebug() << "Recebido do ESP:" << messenger;

        // Pega todos os ID's que receber na lista e separa por "," para adicionar 1 por 1
        if (messenger.startsWith("Require_IDs")) {

            QStringList listaDeIds = messenger.split(",", Qt::SkipEmptyParts);

            // O 'for' começa em '1' para ignorar o "Require_IDs" (que está no índice 0)
            for (int i = 1; i < listaDeIds.size(); i++) {
                QString idEsp = listaDeIds[i];
                emit newDeviceDetected(idEsp);
            }
        }
    }
}
