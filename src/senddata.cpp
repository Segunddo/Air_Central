#include "senddata.h"

SendData::SendData(QSerialPort *portaSerial, QObject *parent)
    : QObject(parent), serial(portaSerial)
{
    // Prepara o timer que vai esvaziar a fila (150ms entre cada envio)
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
    filaDeMensagens.clear(); // Limpa qualquer envio que tenha ficado travado

    // ---------------------------------------------------------
    // PASSO 1: Empacotar a Hora Atual (Timestamp Unix)
    // ---------------------------------------------------------
    QJsonObject cmdTime;
    cmdTime["command"] = "Sync_Time";

    QDateTime agora = QDateTime::currentDateTime();
    // Pega o Epoch UTC e soma a diferença do fuso do PC atual (ex: -10800s para o Brasil)
    qint64 epochTime = agora.toSecsSinceEpoch() + agora.offsetFromUtc();
    cmdTime["timestamp"] = epochTime;

    filaDeMensagens.enqueue(cmdTime);

    // ---------------------------------------------------------
    // PASSO 2: Empacotar os Agendamentos e Códigos
    // ---------------------------------------------------------
    QFile fileAgendamentos("agendamentos.json");
    QFile fileCodes("codes.json");

    if (fileAgendamentos.open(QIODevice::ReadOnly) && fileCodes.open(QIODevice::ReadOnly)) {

        QJsonObject rootAgendamentos = QJsonDocument::fromJson(fileAgendamentos.readAll()).object();
        QJsonObject objCodes = QJsonDocument::fromJson(fileCodes.readAll()).object();

        fileAgendamentos.close();
        fileCodes.close();

        // Itera sobre cada ESP salva nos agendamentos
        for (auto it = rootAgendamentos.begin(); it != rootAgendamentos.end(); ++it) {
            QString idEsp = it.key();
            QJsonArray rotinas = it.value().toArray();

            if (rotinas.isEmpty()) continue;

            // Manda a ESP limpar a memória antes de receber os dados novos
            QJsonObject cmdClear;
            cmdClear["command"] = "Clear_Memory";
            cmdClear["id"] = idEsp;
            filaDeMensagens.enqueue(cmdClear);

            QStringList codigosJaEnviados; // Evita mandar o mesmo código IR repetido pra mesma placa

            for (int i = 0; i < rotinas.size(); ++i) {
                QJsonObject rotina = rotinas[i].toObject();

                QString acao = rotina["acao"].toString();
                QString temp = rotina["temp"].toString();
                QString nomeCodigo = (acao == "Ligar" && temp != "--") ? temp : acao;

                // Empacota o Código IR (se ainda não foi enviado)
                if (!codigosJaEnviados.contains(nomeCodigo) && objCodes.contains(nomeCodigo)) {
                    QJsonObject cmdCode;
                    cmdCode["command"] = "Add_Code";
                    cmdCode["id"] = idEsp;
                    cmdCode["name"] = nomeCodigo;

                    // Extrai os códigos salvos
                    const QJsonArray rawArray = objCodes[nomeCodigo].toArray();
                    QStringList rawStringList;

                    for (QJsonValue val : rawArray) {
                        if (val.isString()) {
                            // Se for String (novo padrão gerado pela Bridge)
                            rawStringList << val.toString();
                        } else {
                            // Suporte a legado caso ainda tenha números soltos no JSON
                            rawStringList << QString::number(val.toInt());
                        }
                    }

                    // JUNTA COM \n (A ESP vai separar e atirar cada linha no loop)
                    cmdCode["raw"] = rawStringList.join("\n");

                    filaDeMensagens.enqueue(cmdCode);
                    codigosJaEnviados.append(nomeCodigo);
                }

                // --- ATUALIZAÇÃO: Conversão do Dia da Semana ---
                // Transforma a String ("Seg", "Ter") no Inteiro (0 a 6) que a ESP espera
                QString diaStr = rotina["dia"].toString();
                int diaNumero = 0;

                if (diaStr == "Dom") diaNumero = 0;
                else if (diaStr == "Seg") diaNumero = 1;
                else if (diaStr == "Ter") diaNumero = 2;
                else if (diaStr == "Qua") diaNumero = 3;
                else if (diaStr == "Qui") diaNumero = 4;
                else if (diaStr == "Sex") diaNumero = 5;
                else if (diaStr == "Sab") diaNumero = 6;

                // Empacota a Tarefa (Agenda)
                QJsonObject cmdSchedule;
                cmdSchedule["command"] = "Add_Schedule";
                cmdSchedule["id"] = idEsp;
                cmdSchedule["dia"] = diaNumero; // <-- Agora manda como Número!
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

        emit syncStarted(filaDeMensagens.size()); // AVISA QUE COMEÇOU
        timerFila->start(150);

    } else {
        emit syncFinished(); // SE A FILA ESTIVER VAZIA, JÁ LIBERA A TELA
    }
}

void SendData::process_data()
{
    if (filaDeMensagens.isEmpty()) {
        timerFila->stop();
        qDebug() << "Sincronização concluída com sucesso!";

        emit syncFinished(); // AVISA QUE TERMINOU TUDO
        return;
    }

    QJsonObject pacote = filaDeMensagens.dequeue();
    send_data(pacote);

    emit syncProgress(filaDeMensagens.size()); // AVISA QUANTOS FALTAM
}
