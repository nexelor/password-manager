#include <QApplication>
#include <QSettings>
#include "ui/vaultmanagerwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Password Manager");
    app.setOrganizationName("LocalFirst");
    app.setOrganizationDomain("localfirst.pm");
    
    // Prevent app from quitting when windows are temporarily hidden
    app.setQuitOnLastWindowClosed(false);

    VaultManagerWindow *vaultManager = new VaultManagerWindow();
    vaultManager->setAttribute(Qt::WA_DeleteOnClose);
    vaultManager->show();

    // Connect to quit when vault manager is closed
    QObject::connect(vaultManager, &QWidget::destroyed, &app, &QApplication::quit);
    
    return app.exec();
}