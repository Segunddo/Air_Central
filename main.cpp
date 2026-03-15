#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "receivedata.h"
#include "senddata.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // 1. Instanciamos a nossa classe C++
    ReceiveData receiveData;
    SendData sendData;

    QQmlApplicationEngine engine;

    // 2. Expondo a instância para o QML.
    engine.rootContext()->setContextProperty("receiveData", &receiveData);
    engine.rootContext()->setContextProperty("sendData", &sendData);

    // Carrega o arquivo QML
    engine.loadFromModule("Central_Ar_Condicionado", "Main");

    return app.exec();
}
