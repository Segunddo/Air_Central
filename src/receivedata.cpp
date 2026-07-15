#include "receivedata.h"

ReceiveData::ReceiveData(SaveData *saveData, QObject *parent) : QObject(parent) , saveData(saveData)
{
    serialPort = new QSerialPort(this);

    connect(serialPort, &QSerialPort::readyRead,
            this, &ReceiveData::read_data);
}

QStringList ReceiveData::list_ports() {
    QStringList nomesDasPortas;

    const auto portas = QSerialPortInfo::availablePorts();

    nomesDasPortas.append("--");
    for (const QSerialPortInfo &porta : portas) {
        nomesDasPortas.append(porta.portName());
    }
    return nomesDasPortas;
}

bool ReceiveData::conectar(QString nomePorta)
{
    if (serialPort == nullptr) {
        serialPort = new QSerialPort(this);
    }

    if (serialPort->isOpen()) {
        serialPort->close();
        qDebug() << "Desconectando da porta";
        return false;
    } else {
        serialPort->setPortName(nomePorta);

        serialPort->setBaudRate(QSerialPort::Baud115200);

        if (serialPort->open(QIODevice::ReadWrite)) {
            qDebug() << "Conectado com sucesso na porta:" << nomePorta;
            return true;
        } else {
            qDebug() << "Erro ao iniciar!" << serialPort->errorString();
            return false;
        }
    }
}

void ReceiveData::read_data()
{
    buffer.append(serialPort->readAll());

    while (buffer.contains('\n')) {
        int indexQuebra = buffer.indexOf('\n');

        QByteArray linha = buffer.left(indexQuebra).trimmed();

        buffer.remove(0, indexQuebra + 1);

        if (linha.isEmpty()) {
            continue;
        }

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(linha, &error);

        if (doc.isNull() || error.error != QJsonParseError::NoError || !doc.isObject()) {
            qDebug() << "Dado recebido não é um JSON válido ou está corrompido:" << linha;
            continue;
        }

        QJsonObject jsonObject = doc.object();

        decode_data(jsonObject);
    }
}

void ReceiveData::decode_data(QJsonObject jsonObject)
{
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

        QJsonObject novoDado;
        novoDado[waitedCommand] = code;

        saveData->save_data(novoDado);

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