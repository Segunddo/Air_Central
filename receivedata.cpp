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

        // Aqui dividimos a string na ",". Ex : CI 101,Ligado,30°C
        QStringList parts = messenger.split(",");

        if(parts.size() == 3) {
            QString idEsp = parts[0];
            QString status = parts[1];
            QString temperature = parts[2];

            emit newDeviceDetected(idEsp, status, temperature);
        }
    }
}
