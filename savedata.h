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

    void save_data(QJsonObject &data);

    Q_INVOKABLE void delete_data();

    Q_INVOKABLE void delete_all_data();

    int get_buffer_size() {return buffer.size();};

signals:
    void buffer_size_changed();

private:
    QJsonObject data;
    QVector<QJsonObject> buffer;
};

#endif // SAVEDATA_H
