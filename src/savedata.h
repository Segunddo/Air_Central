#ifndef SAVEDATA_H
#define SAVEDATA_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QProcess>

#define ARQUIVO_AGENDAMENTOS "agendamentos.json"

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
    Q_INVOKABLE void adicionar_regra(QString idEsp, QString hora, QString acao, QString temp, QString dia);

    // Método para rodar o script Python automaticamente
    Q_INVOKABLE void importar_saci(const QString &centroId = "CI", int tempPadrao = 22);

    Q_INVOKABLE void remover_regra(QString idEsp, int index);

    Q_INVOKABLE QString obter_regras(QString idEsp);

signals:
    void codes_size_changed();
    void agendamentos_atualizados();

private:
    QJsonObject data;

    QJsonObject ler_arquivo();
    void salvar_arquivo(const QJsonObject &jsonObj);
};

#endif // SAVEDATA_H
