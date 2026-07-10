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

    // Pega os segundos exatos no momento do clique
    qint64 epochTime = QDateTime::currentSecsSinceEpoch();
    cmdTime["timestamp"] = epochTime;

    // Se a ESP ainda precisar da string do dia para facilitar a busca do agendamento,
    // você pode manter, mas o Timestamp já contém essa informação matematicamente.
    int diaNumero = QDate::currentDate().dayOfWeek();
    QStringList dias = {"", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab", "Dom"};
    cmdTime["dia"] = dias[diaNumero];

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

                    // Transforma o array [4500, 4400...] em string "4500,4400..."
                    const QJsonArray rawArray = objCodes[nomeCodigo].toArray();
                    QStringList rawStringList;
                    for (QJsonValue val : rawArray) {
                        rawStringList << QString::number(val.toInt());
                    }
                    cmdCode["raw"] = rawStringList.join(",");

                    filaDeMensagens.enqueue(cmdCode);
                    codigosJaEnviados.append(nomeCodigo);
                }

                // Empacota a Tarefa (Agenda)
                QJsonObject cmdSchedule;
                cmdSchedule["command"] = "Add_Schedule";
                cmdSchedule["id"] = idEsp;
                cmdSchedule["dia"] = rotina["dia"].toString();
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
        timerFila->start(150); // Inicia o motor!
    }
}

void SendData::process_data()
{
    if (filaDeMensagens.isEmpty()) {
        timerFila->stop(); // Desliga o motor quando acabar
        qDebug() << "Sincronização concluída com sucesso!";
        return;
    }

    // Tira o próximo pacote da fila emanda para o Serial
    QJsonObject pacote = filaDeMensagens.dequeue();
    send_data(pacote);
}
