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
            this, &ReceiveData::read_data);
}

void ReceiveData::read_data()
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

        decode_data(jsonObject);
    }
}

void ReceiveData::decode_data(QJsonObject jsonObject)
{
    // Verifica o comando recebido
    QString messageType = jsonObject["command"].toString();

    if (messageType == "Resposta_ID") {

        QString idEsp = jsonObject["id"].toString();
        qDebug() << "Novo ESP descoberto na rede Mesh:" << idEsp;

        emit newDeviceDetected(idEsp);
    }

    else if (messageType == "new_code") {
        if (waitedCommand.isEmpty()) {
            qDebug() << "Recebi um código IR, mas não estou em modo de cadastro. Ignorando.";
            return;
        }

        QString code = jsonObject["code"].toString();

        // Monta o JSON exato que o SaveData precisa (ex: {"Ligar": "0xFFA2"})
        QJsonObject novoDado;
        novoDado[waitedCommand] = code;

        // Salva no arquivo
        SaveData saveData;
        saveData.save_data(novoDado);

        // Limpa a variável
        waitedCommand = "";
        qDebug() << "Código salvo com sucesso!";
    }

    else if (messageType == "status_update") {

        QString idEsp = jsonObject["id"].toString();
        QString status = jsonObject["status"].toString();
        QString temp = jsonObject["temp"].toString();
        qDebug() << "Atualização: " + idEsp + " " + status + " " + temp;
    }
}

void ReceiveData::set_waited_command(QString commandType)
{
    waitedCommand = commandType;
    qDebug() << "ReceiveData agora está aguardando um código para:" << waitedCommand;
}
