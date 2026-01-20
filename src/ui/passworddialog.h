#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include "../models/passwordentry.h"
#include "../models/settings.h"

class PasswordDialog : public QDialog {
    Q_OBJECT

public:
    explicit PasswordDialog(VaultSettings *vaultSettings = nullptr, QWidget *parent = nullptr);
    explicit PasswordDialog(const PasswordEntry &entry, VaultSettings *vaultSettings = nullptr, QWidget *parent = nullptr);
    
    PasswordEntry getPasswordEntry() const;

private slots:
    void onGeneratePassword();
    void onTogglePasswordVisibility();

private:
    QLineEdit *m_titleInput;
    QLineEdit *m_usernameInput;
    QLineEdit *m_passwordInput;
    QLineEdit *m_urlInput;
    QTextEdit *m_notesInput;
    QPushButton *m_generateButton;
    QPushButton *m_toggleVisibilityButton;
    
    bool m_isEditMode;
    PasswordEntry m_entry;

    VaultSettings *m_vaultSettings;
    
    void setupUi();
    QString generateRandomPassword(int length = 16);
};

#endif