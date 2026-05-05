#ifndef SAVEDATA_H
#define SAVEDATA_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

class SaveData: public QObject
{
    Q_OBJECT

public:
    SaveData();

    void save_data(QJsonObject &data);

private:
    QJsonObject data;
};

#endif // SAVEDATA_H
