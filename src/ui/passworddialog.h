#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include "../models/passwordentry.h"

class PasswordDialog : public QDialog {
    Q_OBJECT

public:
    explicit PasswordDialog(QWidget *parent = nullptr);
    explicit PasswordDialog(const PasswordEntry &entry, QWidget *parent = nullptr);
    
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
    
    void setupUi();
    void loadStyleSheet();
    QString generateRandomPassword(int length = 16);
};

#endif