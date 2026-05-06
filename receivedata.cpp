#include "receivedata.h"

ReceiveData::ReceiveData(QObject *parent) : QObject(parent)
{
    serialPort = new QSerialPort(this);

    // Conecta o sinal de dados chegando ao nosso slot de leitura
    connect(serialPort, &QSerialPort::readyRead,
            this, &ReceiveData::read_data);
}

bool ReceiveData::conectar(QString nomePorta)
{
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

void ReceiveData::read_data()
{
    buffer.append(serialPort->readAll());

    while (buffer.contains('\n')) {
        // Encontra onde está o fim da linha
        int indexQuebra = buffer.indexOf('\n');

        QByteArray linha = buffer.left(indexQuebra).trimmed();

        buffer.remove(0, indexQuebra + 1);

        // Se a linha for vazia, ignora
        if (linha.isEmpty()) {
            continue;
        }

        // Tenta converter o texto recebido para JSON
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
