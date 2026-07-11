#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>

#include "receivedata.h"
#include "senddata.h"
#include "savedata.h"

int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/qt/qml/Central_Ar_Condicionado/qml/icon.svg"));
    QQuickStyle::setStyle("Basic");

    // 1. Instanciamos apenas as classes que conversam com a Interface
    SaveData saveData;
    ReceiveData receiveData(&saveData);
    SendData sendData(receiveData.getSerialPort());

    QQmlApplicationEngine engine;

    // 2. Expondo as instâncias para o QML
    engine.rootContext()->setContextProperty("receiveData", &receiveData);
    engine.rootContext()->setContextProperty("sendData", &sendData);
    engine.rootContext()->setContextProperty("saveData", &saveData);

    // Carrega o arquivo QML (Padrão Qt 6)
    engine.loadFromModule("Central_Ar_Condicionado", "Main");

    return app.exec();
}
