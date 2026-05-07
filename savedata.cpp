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

void SaveData::save_data(QJsonObject &newData)
{
    bool foiModificado = false;

    for (auto it = newData.begin(); it != newData.end(); ++it) {
        QString chave = it.key();
        QJsonValue novoCodigo = it.value();

        QJsonArray arrayAtual = this->data[chave].toArray();

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
        // Adiciona ao buffer para caso deseje apagar dps
        buffer.push_back(newData);
        emit buffer_size_changed();

        QFile file("codes.json");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonDocument documento(this->data);
            file.write(documento.toJson(QJsonDocument::Indented));
            file.close();
            qDebug() << "Novo código adicionado à lista e salvo com sucesso!";
        }
    } else {
        qDebug() << "O código recebido já estava salvo. Arquivo ignorado.";
    }
}

void SaveData::delete_data()
{
    bool foiModificado = false;

    if (buffer.isEmpty()) {
        qDebug() << "O buffer está vazio, nada para apagar.";
        return;
    }

    QJsonObject deleteData = buffer.last();

    for (auto it = deleteData.begin(); it != deleteData.end(); ++it) {
        QString chave = it.key();
        QJsonValue codigoParaRemover = it.value();

        QJsonArray arrayAtual = this->data[chave].toArray();

        // Procura a posição do item no QJsonArray
        for (int i = 0; i < arrayAtual.size(); ++i) {
            if (arrayAtual.at(i) == codigoParaRemover) {

                // Se encontrou, apaga usando o índice!
                arrayAtual.removeAt(i);

                // Substitui o array antigo pelo novo array no objeto principal
                this->data[chave] = arrayAtual;

                foiModificado = true; // Avisa que teremos que salvar no disco

                // Interrompe o loop do array, já que achamos e apagamos o valor
                break;
            }
        }
    }

    if (foiModificado) {
        QFile file("codes.json");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonDocument documento(this->data);
            file.write(documento.toJson(QJsonDocument::Indented));
            file.close();
            qDebug() << "Novo código removido da lista e salvo com sucesso!";
        } else {
            qDebug() << "Erro ao salvar o código apagado da lista";
        }
    }

    buffer.removeLast();
    emit buffer_size_changed();

}

void SaveData::delete_all_data()
{
    QFile file("codes.json");
    file.remove();
    qDebug() << "Arquivo apagado";
    SaveData();
}
