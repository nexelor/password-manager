#include <QApplication>
#include <QSettings>
#include "ui/vaultmanagerwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Password Manager");
    app.setOrganizationName("LocalFirst");
    app.setOrganizationDomain("localfirst.pm");
    
    VaultManagerWindow vaultManager;
    vaultManager.show();
    
    return app.exec();
}