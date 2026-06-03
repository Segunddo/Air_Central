cmake_minimum_required(VERSION 3.16)

project(Central_Ar_Condicionado VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick SerialPort)
find_package(Qt6${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appCentral_Ar_Condicionado
    main.cpp
)

qt_add_qml_module(appCentral_Ar_Condicionado
    URI Central_Ar_Condicionado
    VERSION 1.0
    QML_FILES
        Main.qml
        TelaLista.qml
        TelaCadastro.qml
    SOURCES
        receivedata.h receivedata.cpp
        senddata.h senddata.cpp
        savedata.h savedata.cpp
)

set_target_properties(appCentral_Ar_Condicionado PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appCentral_Ar_Condicionado
    PRIVATE Qt6::Quick Qt6::SerialPort
    Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::QuickControls2
)

include(GNUInstallDirs)
install(TARGETS appCentral_Ar_Condicionado
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
