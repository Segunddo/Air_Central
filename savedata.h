#ifndef SAVEDATA_H
#define SAVEDATA_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

using namespace std;

class SaveData: public QObject
{
    Q_OBJECT

public:
    SaveData();

    // Salvamento dos códigos IR
    void save_data(const QJsonObject data);

    Q_INVOKABLE void delete_data(const QString &comando);

    Q_INVOKABLE void delete_all_data();

    Q_INVOKABLE int get_command_count(const QString &comando);

    // Salvamento dos horários
    Q_INVOKABLE void adicionar_regra(QString idEsp, QString hora, QString acao, QString temp);

    Q_INVOKABLE void remover_regra(QString idEsp, int index);

    Q_INVOKABLE QString obter_regras(QString idEsp);

signals:
    void codes_size_changed();

private:
    QJsonObject data;

    QJsonObject ler_arquivo();
    void salvar_arquivo(const QJsonObject &jsonObj);

    const QString ARQUIVO_AGENDAMENTOS = "agendamentos.json";
};

#endif // SAVEDATA_H
