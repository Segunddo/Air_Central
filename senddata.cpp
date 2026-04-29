#include "senddata.h"

SendData::SendData(QObject *parent) : QObject(parent)
{
    udpSocket = new QUdpSocket(this);
}

void SendData::sendCommand(QString idEsp, QString status, QString temperatura)
{
    QString message = idEsp + "," + status + "," + temperatura + ",2";
    QByteArray datagrama = message.toUtf8();

    udpSocket->writeDatagram(datagrama, QHostAddress("192.168.7.255"), 8081);

    qDebug() << "Classe SendData disparou:" << message;
}
