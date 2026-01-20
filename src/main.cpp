#include <QApplication>
#include <QSettings>
#include <QTimer>
#include "ui/vaultmanagerwindow.h"
#include "models/settings.h"
#include "ui/thememanager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Password Manager");
    app.setOrganizationName("LocalFirst");
    app.setOrganizationDomain("localfirst.pm");
    
    // Prevent app from quitting when windows are temporarily hidden
    app.setQuitOnLastWindowClosed(false);

    // Load and apply settings
    AppSettings *appSettings = new AppSettings();
    
    // Apply theme
    ThemeManager::instance()->applyTheme(appSettings->theme());

    VaultManagerWindow *vaultManager = new VaultManagerWindow();
    vaultManager->setAttribute(Qt::WA_DeleteOnClose);
    vaultManager->show();

    // Connect to quit when vault manager is closed
    QObject::connect(vaultManager, &QWidget::destroyed, &app, &QApplication::quit);
    QObject::connect(vaultManager, &QWidget::destroyed, [appSettings]() {
        delete appSettings;
    });
    
    return app.exec();
}