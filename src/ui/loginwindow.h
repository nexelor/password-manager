#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../storage/database.h"

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(const QString &vaultPath);
    ~LoginWindow();

private slots:
    void onLoginClicked();
    void onCreateVaultClicked();

private:
    QLineEdit *m_passwordInput;
    QPushButton *m_loginButton;
    QPushButton *m_createVaultButton;
    QLabel *m_statusLabel;
    
    Database *m_database;
    QString m_vaultPath;
    
    void setupUi();
    bool checkIfVaultExists();
    void createVault(const QString &masterPassword);
    void unlockVault(const QString &masterPassword);
};

#endif