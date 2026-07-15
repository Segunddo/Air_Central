#include "senddata.h"

SendData::SendData(QSerialPort *portaSerial, QObject *parent)
    : QObject(parent), serial(portaSerial)
{
    timerFila = new QTimer(this);
    connect(timerFila, &QTimer::timeout, this, &SendData::process_data);
}

void SendData::send_command_status(QString idEsp, QString status)
{
    QJsonObject cmd;
    cmd["command"] = "Dispatch";
    cmd["id"]      = idEsp;
    cmd["status"]  = status;

    cmd["name"]    = (status == "Ligado") ? "Ligar" : "Desligar";

    send_data(cmd);
}

void SendData::send_command_temp(QString idEsp, QString temperatura)
{
    QJsonObject cmd;
    cmd["command"] = "Dispatch";
    cmd["id"]      = idEsp;
    cmd["temp"]    = temperatura;

    cmd["name"]    = temperatura;

    send_data(cmd);
}

void SendData::require_espID_change(const QString idEsp, const QString newId)
{
    QJsonObject jsonRequest;
    jsonRequest["command"] = "Change_ID";
    jsonRequest["id"] = idEsp;
    jsonRequest["new_id"] = newId;

    send_data(jsonRequest);
}

void SendData::requireEspsId()
{
    QJsonObject jsonRequest;
    jsonRequest["command"] = "Require_IDs";
    send_data(jsonRequest);
    qDebug() << "Classe SendData requisitou IDs";
}

void SendData::require_ir_read()
{
    QJsonObject jsonRequest;
    jsonRequest["command"] = "Require_IR";
    send_data(jsonRequest);
    qDebug() << "Classe SendData requisitou leitura de infravermelho";
}

void SendData::send_data(QJsonObject jsonCommand)
{
    QJsonDocument doc(jsonCommand);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n');

    if (serial && serial->isOpen()) {
        serial->write(data);
        qDebug() << "Classe SendData disparou:" << data;
    } else {
        qDebug() << "Erro: Tentando enviar";
        emit errorOccurred("Falha ao enviar: A porta serial não está conectada ou aberta.");
    }
}

void SendData::sinc_esp_data()
{
    filaDeMensagens.clear();

    // ---------------------------------------------------------
    // PASSO 1: Empacotar a Hora Atual (Timestamp Unix)
    // ---------------------------------------------------------
    QJsonObject cmdTime;
    cmdTime["command"] = "Sync_Time";

    qint64 epochTime = QDateTime::currentSecsSinceEpoch();
    cmdTime["timestamp"] = epochTime;

    filaDeMensagens.enqueue(cmdTime);

    // ---------------------------------------------------------
    // PASSO 2: Empacotar os Códigos IR e Agendamentos
    // ---------------------------------------------------------
    QFile fileAgendamentos("agendamentos.json");
    QFile fileCodes("codes.json");

    if (fileAgendamentos.open(QIODevice::ReadOnly) && fileCodes.open(QIODevice::ReadOnly)) {

        QJsonObject rootAgendamentos = QJsonDocument::fromJson(fileAgendamentos.readAll()).object();
        QJsonObject objCodes = QJsonDocument::fromJson(fileCodes.readAll()).object();

        fileAgendamentos.close();
        fileCodes.close();

        for (auto it = rootAgendamentos.begin(); it != rootAgendamentos.end(); ++it) {
            QString idEsp = it.key();
            QJsonArray rotinas = it.value().toArray();

            // Manda a ESP limpar a memória antes de receber os dados novos
            QJsonObject cmdClear;
            cmdClear["command"] = "Clear_Memory";
            cmdClear["id"] = idEsp;
            filaDeMensagens.enqueue(cmdClear);

            // =========================================================
            // NOVA LÓGICA: Envia TODOS os códigos disponíveis no sistema
            // para a Flash do ESP, garantindo o funcionamento manual.
            // =========================================================
            for (auto itCode = objCodes.begin(); itCode != objCodes.end(); ++itCode) {
                QString nomeCodigo = itCode.key();
                QJsonArray rawArray = itCode.value().toArray();

                if (!rawArray.isEmpty()) {
                    QJsonObject cmdCode;
                    cmdCode["command"] = "Add_Code";
                    cmdCode["id"] = idEsp;
                    cmdCode["name"] = nomeCodigo;

                    // Pega o último código gravado
                    cmdCode["raw"] = rawArray.last().toString();

                    filaDeMensagens.enqueue(cmdCode);
                }
            }

            // =========================================================
            // Envia apenas os horários dos agendamentos
            // =========================================================
            for (int i = 0; i < rotinas.size(); ++i) {
                QJsonObject rotina = rotinas[i].toObject();

                QString acao = rotina["acao"].toString();
                QString temp = rotina["temp"].toString();
                QString nomeCodigo = (acao == "Ligar" && temp != "--") ? temp : acao;

                QString diaStr = rotina["dia"].toString();
                int diaNumero = 0;

                if (diaStr == "Dom") diaNumero = 0;
                else if (diaStr == "Seg") diaNumero = 1;
                else if (diaStr == "Ter") diaNumero = 2;
                else if (diaStr == "Qua") diaNumero = 3;
                else if (diaStr == "Qui") diaNumero = 4;
                else if (diaStr == "Sex") diaNumero = 5;
                else if (diaStr == "Sab") diaNumero = 6;

                QJsonObject cmdSchedule;
                cmdSchedule["command"] = "Add_Schedule";
                cmdSchedule["id"] = idEsp;
                cmdSchedule["dia"] = diaNumero;
                cmdSchedule["hora"] = rotina["hora"].toString();
                cmdSchedule["code"] = nomeCodigo;

                filaDeMensagens.enqueue(cmdSchedule);
            }
        }
    } else {
        qDebug() << "Aviso: Não foi possível ler agendamentos.json ou codes.json";
    }

    // ---------------------------------------------------------
    // PASSO 3: Iniciar o Disparo
    // ---------------------------------------------------------
    if (!filaDeMensagens.isEmpty()) {
        qDebug() << "Iniciando envio de" << filaDeMensagens.size() << "pacotes...";

        emit syncStarted(filaDeMensagens.size());

        timerFila->start(350);

    } else {
        emit syncFinished();
    }
}

void SendData::process_data()
{
    if (filaDeMensagens.isEmpty()) {
        timerFila->stop();
        qDebug() << "Sincronização concluída com sucesso!";

        emit syncFinished();
        return;
    }

    QJsonObject pacote = filaDeMensagens.dequeue();
    send_data(pacote);

    emit syncProgress(filaDeMensagens.size());
}