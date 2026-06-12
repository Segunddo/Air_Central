cmake_minimum_required(VERSION 3.16)

project(Central_Ar_Condicionado VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick SerialPort)
find_package(Qt6${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.5)

# 1. Apontar o executável para a pasta src/
qt_add_executable(appCentral_Ar_Condicionado
    src/main.cpp
)

# 2. Separar as camadas de QML e C++ nos arquivos
qt_add_qml_module(appCentral_Ar_Condicionado
    URI Central_Ar_Condicionado
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/TelaLista.qml
        qml/TelaCadastro.qml
    SOURCES
        src/receivedata.h src/receivedata.cpp
        src/senddata.h src/senddata.cpp
        src/savedata.h src/savedata.cpp
)

# 3. Propriedades do executável
set_target_properties(appCentral_Ar_Condicionado PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 4. Linkar as bibliotecas
target_link_libraries(appCentral_Ar_Condicionado
    PRIVATE Qt6::Quick Qt6::SerialPort
    Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::QuickControls2
)

add_custom_target(Scripts_Python SOURCES scripts/saci_ci.py)

# 5. CÓPIA DO SCRIPT PYTHON (Essencial)
# Copia o script para o mesmo diretório onde o executável do Qt será gerado,
# resolvendo problemas de "arquivo não encontrado" ao usar o QProcess.
file(COPY scripts/saci_ci.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# 6. Instalação
include(GNUInstallDirs)
install(TARGETS appCentral_Ar_Condicionado
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
