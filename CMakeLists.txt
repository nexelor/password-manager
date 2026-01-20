cmake_minimum_required(VERSION 3.16)
project(PasswordManager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)
find_package(OpenSSL REQUIRED)

set(PROJECT_SOURCES
    src/main.cpp
    src/ui/vaultmanagerwindow.cpp
    src/ui/vaultmanagerwindow.h
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/loginwindow.cpp
    src/ui/loginwindow.h
    src/ui/passworddialog.cpp
    src/ui/passworddialog.h
    src/ui/settingsdialog.cpp
    src/ui/settingsdialog.h
    src/ui/thememanager.cpp
    src/ui/thememanager.h
    src/models/passwordentry.cpp
    src/models/passwordentry.h
    src/models/vaultinfo.cpp
    src/models/vaultinfo.h
    src/models/settings.cpp
    src/models/settings.h
    src/crypto/encryption.cpp
    src/crypto/encryption.h
    src/storage/database.cpp
    src/storage/database.h
    src/storage/vaultmanager.cpp
    src/storage/vaultmanager.h
)

add_executable(password-manager ${PROJECT_SOURCES} resources.qrc)

target_link_libraries(password-manager
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    OpenSSL::SSL
    OpenSSL::Crypto
)

install(TARGETS password-manager
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)