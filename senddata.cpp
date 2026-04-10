#include "senddata.h"

SendData::SendData(QObject *parent) : QObject(parent)
{
    udpSocket = new QUdpSocket(this);
}

void SendData::sendComand(QString idEsp, QString status, QString temperatura, int ttl)
{
    QString message = idEsp + "," + status + "," + temperatura + "," + QString::number(ttl);
    QByteArray datagrama = message.toUtf8();

    udpSocket->writeDatagram(datagrama, QHostAddress::Broadcast, 8081);

    qDebug() << "Classe SendData disparou:" << message;
}

void SendData::requireEspsId()
{
    QString message = "Require_IDs";
    QByteArray datagrama = message.toUtf8();

    udpSocket->writeDatagram(datagrama, QHostAddress::Broadcast, 8081);
}
