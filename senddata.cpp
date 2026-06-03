#include "senddata.h"

SendData::SendData(QSerialPort *portaSerial, QObject *parent)
    : QObject(parent), serial(portaSerial)
{
    // Inicializa o motor de automação em background
    timerAutomacao = new QTimer(this);
    connect(timerAutomacao, &QTimer::timeout, this, &SendData::verificar_e_disparar_agendamentos);

    // Roda a cada 30 segundos para garantir que pegará o minuto exato da virada do relógio
    timerAutomacao->start(30000);
}

void SendData::send_all_codes(QJsonObject baseCommand, QJsonArray codigos)
{
    if (codigos.isEmpty()) {
        qDebug() << "Nenhum código IR salvo para este comando.";
        return;
    }

    for (int i = 0; i < codigos.size(); i++) {
        QString codigo = codigos[i].toString();

        QTimer::singleShot(i * 100, this, [this, baseCommand, codigo]() {
            QJsonObject cmd = baseCommand;
            cmd["code"] = codigo;
            send_data(cmd);
        });
    }
}

void SendData::send_command_status(QString idEsp, QString status)
{
    QJsonObject baseCommand;
    baseCommand["command"] = "Dispatch";
    baseCommand["id"]      = idEsp;
    baseCommand["status"]  = status;

    QString chaveBusca = (status == "Ligado") ? "Ligar" : "Desligar";
    QJsonArray codigos = get_codes_from_file(chaveBusca);

    send_all_codes(baseCommand, codigos);
}

void SendData::send_command_temp(QString idEsp, QString temperatura)
{
    QJsonObject baseCommand;
    baseCommand["command"] = "Dispatch";
    baseCommand["id"]      = idEsp;
    baseCommand["temp"]    = temperatura;

    QJsonArray codigos = get_codes_from_file(temperatura);

    send_all_codes(baseCommand, codigos);
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

QJsonArray SendData::get_codes_from_file(const QString& chave)
{
    QJsonArray arrayCodigos;
    QFile file("codes.json");

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (!doc.isNull() && doc.isObject()) {
            QJsonObject data = doc.object();
            if (data.contains(chave)) {
                arrayCodigos = data[chave].toArray();
            }
        }
    } else {
        qDebug() << "Erro: Nao foi possivel abrir codes.json para leitura.";
    }

    return arrayCodigos;
}

void SendData::verificar_e_disparar_agendamentos()
{
    // Pega o horário atual do computador
    QString horaAtual = QTime::currentTime().toString("hh:mm");

    // Trava de segurança: se já checou/disparou as rotinas desse minuto, ignora até o próximo minuto
    if (horaAtual == ultimaHoraDisparada) {
        return;
    }

    QFile file("agendamentos.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(fileData);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonObject rootObj = doc.object();

    for (auto it = rootObj.begin(); it != rootObj.end(); ++it) {
        QString idEsp = it.key();
        QJsonArray rotinas = it.value().toArray();

        for (int i = 0; i < rotinas.size(); ++i) {
            QJsonObject rotina = rotinas[i].toObject();

            // Se a hora salva bater milimetricamente com a hora atual do sistema...
            if (rotina["hora"].toString() == horaAtual) {
                QString acao = rotina["acao"].toString();
                QString temp = rotina["temp"].toString();

                qDebug() << "⏰ [AUTOMAÇÃO] Hora do Gatilho alcançada (" << horaAtual << ") para o dispositivo:" << idEsp;

                send_command_status(idEsp, acao);

                // Se a ação for ligar, dispara o comando de temperatura logo em seguida
                if (acao == "Ligar" && temp != "--") {
                    // Dá um micro delay de 500ms para não atropelar a escrita serial do comando anterior
                    QTimer::singleShot(500, this, [this, idEsp, temp]() {
                        send_command_temp(idEsp, temp);
                    });
                }
            }
        }
    }

    // Marca o minuto atual como verificado/processado com sucesso
    ultimaHoraDisparada = horaAtual;
}
