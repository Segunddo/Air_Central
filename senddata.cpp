#include "senddata.h"

SendData::SendData(QObject *parent) : QObject(parent)
{
    udpSocket = new QUdpSocket(this);
}

void SendData::sendComand(QString idEsp, QString status, QString temperatura)
{
    QString message = idEsp + "," + status + "," + temperatura;
    QByteArray datagrama = message.toUtf8();

    udpSocket->writeDatagram(datagrama, QHostAddress::Broadcast, 8081);

    qDebug() << "Classe SendData disparou:" << message;
}
