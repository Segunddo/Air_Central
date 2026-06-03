#include "savedata.h"

SaveData::SaveData()
{
    QFile file("codes.json");

    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray dadosSalvos = file.readAll();
            file.close();
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

        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonDocument novoDocumento(this->data);
            file.write(novoDocumento.toJson(QJsonDocument::Indented));
            file.close();
            qDebug() << "Arquivo criado";
        } else
            qDebug() << "Erro ao criar o arquivo";
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

void SaveData::adicionar_regra(QString idEsp, QString hora, QString acao, QString temp)
{
    QJsonObject rootObj = ler_arquivo();
    QJsonArray regrasArray;

    // Se o ESP já tiver regras, pega o array atual
    if (rootObj.contains(idEsp)) {
        regrasArray = rootObj[idEsp].toArray();
    }

    // Cria a nova regra
    QJsonObject novaRegra;
    novaRegra["hora"] = hora;
    novaRegra["acao"] = acao;
    novaRegra["temp"] = temp;

    // Adiciona na lista e salva
    regrasArray.append(novaRegra);
    rootObj[idEsp] = regrasArray;

    salvar_arquivo(rootObj);
    qDebug() << "Regra adicionada para" << idEsp << ":" << novaRegra;
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
