cmake_minimum_required(VERSION 3.16)

project(Central_Ar_Condicionado VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick SerialPort)

qt_standard_project_setup(REQUIRES 6.9)

qt_add_executable(appCentral_Ar_Condicionado

)

qt_add_qml_module(appCentral_Ar_Condicionado
    URI Central_Ar_Condicionado
    VERSION 1.0
    QML_FILES
    TelaLista.qml
        SOURCES receivedata.h receivedata.cpp
        SOURCES senddata.h senddata.cpp
        SOURCES savedata.h savedata.cpp
        RESOURCES
        QML_FILES Main.qml
        QML_FILES TelaCadastro.qml
        SOURCES main.cpp
        SOURCES
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appCentral_Ar_Condicionado PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appCentral_Ar_Condicionado
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appCentral_Ar_Condicionado
    PRIVATE Qt6::Quick Qt6::SerialPort
)

include(GNUInstallDirs)
install(TARGETS appCentral_Ar_Condicionado
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
