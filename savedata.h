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

    void save_data(const QJsonObject data);

    Q_INVOKABLE void delete_data(const QString &comando);

    Q_INVOKABLE void delete_all_data();

    Q_INVOKABLE int get_command_count(const QString &comando);

signals:
    void codes_size_changed();

private:
    QJsonObject data;
};

#endif // SAVEDATA_H