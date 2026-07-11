#include "savedata.h"

SaveData::SaveData()
{
    // ==========================================
    // 1. Inicialização do codes.json
    // ==========================================
    QFile fileCodes("codes.json");

    if (fileCodes.exists()) {
        if (fileCodes.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray dadosSalvos = fileCodes.readAll();
            fileCodes.close();
            QJsonDocument documento = QJsonDocument::fromJson(dadosSalvos);

            if (!documento.isNull() && documento.isObject())
                this->data = documento.object();
        }

    } else {
        this->data["Ligar"] = QJsonArray();
        this->data["Desligar"] = QJsonArray();

        for (int i = 16; i <= 30; ++i) {
            this->data[QString::number(i)] = QJsonArray();
        }

        if (fileCodes.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonDocument novoDocumento(this->data);
            fileCodes.write(novoDocumento.toJson(QJsonDocument::Indented));
            fileCodes.close();
            qDebug() << "Arquivo codes.json criado";
        } else {
            qDebug() << "Erro ao criar o arquivo codes.json";
        }
    }

    // ==========================================
    // 2. Inicialização do agendamentos.json
    // ==========================================
    QFile fileAgendamentos(ARQUIVO_AGENDAMENTOS);

    if (!fileAgendamentos.exists()) {
        if (fileAgendamentos.open(QIODevice::WriteOnly | QIODevice::Text)) {
            // Cria um JSON base vazio: "{}"
            QJsonObject rootVazio;
            QJsonDocument docVazio(rootVazio);

            fileAgendamentos.write(docVazio.toJson(QJsonDocument::Indented));
            fileAgendamentos.close();

            qDebug() << "Arquivo de agendamentos criado com sucesso na inicialização!";
        } else {
            qDebug() << "Erro ao criar o arquivo de agendamentos inicial!";
        }
    }
}

void SaveData::save_data(const QJsonObject newData)
{
    bool foiModificado = false;

    for (auto it = newData.constBegin(); it != newData.constEnd(); ++it) {
        QString chave = it.key();
        QJsonValue novoCodigo = it.value();

        QJsonArray arrayAtual = this->data.value(chave).toArray();

        // Verifica se o código já existe no array (Evita duplicatas)
        if (!arrayAtual.contains(novoCodigo)) {
            // Se não existe, adiciona no array
            arrayAtual.append(novoCodigo);

            // Substitui o array antigo pelo novo array no objeto principal
            this->data[chave] = arrayAtual;

            foiModificado = true; // Avisa que teremos que salvar no disco
        }
    }

    // Só reescreve o arquivo JSON se houver códigos novos de verdade
    if (foiModificado) {
        QFile file("codes.json");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonDocument documento(this->data);
            file.write(documento.toJson(QJsonDocument::Indented));
            file.close();
            qDebug() << "Novo código adicionado à lista e salvo com sucesso!";
            emit codes_size_changed();
        }
    } else {
        qDebug() << "O código recebido já estava salvo. Arquivo ignorado.";
    }
}

void SaveData::delete_data(const QString &comando)
{
    if (data.isEmpty()) {
        qDebug() << "Está vazio, nada para apagar.";
        return;
    }

    QJsonArray arrayAtual = data[comando].toArray();

    if (arrayAtual.isEmpty()) {
        qDebug() << "O comando" << comando << "já está sem códigos.";
        return;
    }

    // Remove o ÚLTIMO elemento adicionado a este array
    arrayAtual.removeLast();

    data[comando] = arrayAtual;

    // Salva
    QFile file("codes.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument documento(this->data);
        file.write(documento.toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "Último código de" << comando << "apagado e arquivo salvo com sucesso!";
    } else {
        qDebug() << "Erro ao salvar o arquivo codes.json";
    }

    emit codes_size_changed();

}

void SaveData::delete_all_data()
{
    QFile file("codes.json");
    file.remove();
    qDebug() << "Arquivo apagado";

    // Reinicializa o estado em memória igual ao construtor
    this->data = QJsonObject();
    this->data["Ligar"]    = QJsonArray();
    this->data["Desligar"] = QJsonArray();
    for (int i = 16; i <= 30; ++i)
        this->data[QString::number(i)] = QJsonArray();

    // Recria o arquivo limpo no disco
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(QJsonDocument(this->data).toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "Arquivo recriado limpo";
    }

    emit codes_size_changed();
}

Q_INVOKABLE int SaveData::get_command_count(const QString &comando)
{
    return this->data[comando].toArray().size();
}

QJsonObject SaveData::ler_arquivo()
{
    QFile file(ARQUIVO_AGENDAMENTOS);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QJsonObject(); // Retorna vazio se o arquivo não existir ainda
    }

    QByteArray fileData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(fileData);
    if (doc.isNull() || !doc.isObject()) {
        return QJsonObject();
    }

    return doc.object();
}

void SaveData::salvar_arquivo(const QJsonObject &jsonObj)
{
    QFile file(ARQUIVO_AGENDAMENTOS);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Erro ao salvar o arquivo de agendamentos!";
        return;
    }

    QJsonDocument doc(jsonObj);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

void SaveData::adicionar_regra(QString idEsp, QString hora, QString acao, QString temp, QString dia)
{
    QJsonObject rootObj = ler_arquivo();
    QJsonArray regrasArray;

    if (rootObj.contains(idEsp)) {
        regrasArray = rootObj[idEsp].toArray();
    }

    QJsonObject novaRegra;
    novaRegra["dia"]  = dia;
    novaRegra["hora"] = hora;
    novaRegra["acao"] = acao;
    novaRegra["temp"] = temp;

    regrasArray.append(novaRegra);
    rootObj[idEsp] = regrasArray;

    salvar_arquivo(rootObj);
    qDebug() << "Regra adicionada com sucesso para" << idEsp << ":" << novaRegra;
}

void SaveData::remover_regra(QString idEsp, int index)
{
    QJsonObject rootObj = ler_arquivo();

    if (rootObj.contains(idEsp)) {
        QJsonArray regrasArray = rootObj[idEsp].toArray();

        // Verifica se o índice é válido
        if (index >= 0 && index < regrasArray.size()) {
            regrasArray.removeAt(index);
            rootObj[idEsp] = regrasArray;
            salvar_arquivo(rootObj);
            qDebug() << "Regra no índice" << index << "removida de" << idEsp;
        }
    }
}

QString SaveData::obter_regras(QString idEsp)
{
    QJsonObject rootObj = ler_arquivo();

    if (rootObj.contains(idEsp)) {
        QJsonArray regrasArray = rootObj[idEsp].toArray();
        QJsonDocument doc(regrasArray);
        return QString(doc.toJson(QJsonDocument::Compact)); // Retorna a string pronta pro QML
    }

    return "[]"; // Retorna array vazio se não tiver nada
}

void SaveData::importar_saci(const QString &centroId, int tempPadrao)
{
    qDebug() << "Disparando automação do script Python saci_ci.py...";

    QProcess *process = new QProcess(this);
    QStringList argumentos;

    // Monta os argumentos mapeando os parâmetros passados pela UI
    argumentos << "saci_ci.py"
               << "--id" << centroId
               << "--temp" << QString::number(tempPadrao)
               << "--out" << ARQUIVO_AGENDAMENTOS;

    // Gerencia o término da execução assíncrona do script
    QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
                         if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
                             qDebug() << "Script SACI executado com sucesso. JSON atualizado.";
                             emit agendamentos_atualizados();
                         } else {
                             qDebug() << "Erro crítico ou timeout ao rodar o script Python. Código de saída:" << exitCode;

                             qDebug() << "LOG DE ERRO DO PYTHON:" << process->readAllStandardError();
                         }
                         process->deleteLater();
                     });

    #ifdef Q_OS_WIN
        QString pythonCmd = "python";
    #else
        QString pythonCmd = "python3";
    #endif

    process->start(pythonCmd, argumentos);
}
