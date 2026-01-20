#include "passworddialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QFile>

PasswordDialog::PasswordDialog(QWidget *parent)
    : QDialog(parent), m_isEditMode(false) {
    setupUi();
    loadStyleSheet();
    setWindowTitle("Add Password");
}

PasswordDialog::PasswordDialog(const PasswordEntry &entry, QWidget *parent)
    : QDialog(parent), m_isEditMode(true), m_entry(entry) {
    setupUi();
    loadStyleSheet();
    setWindowTitle("Edit Password");
    
    m_titleInput->setText(entry.title());
    m_usernameInput->setText(entry.username());
    m_passwordInput->setText(entry.password());
    m_urlInput->setText(entry.url());
    m_notesInput->setPlainText(entry.notes());
}

void PasswordDialog::loadStyleSheet() {
    QFile styleFile(":/src/styles/passworddialog.qss");
    if (!styleFile.open(QFile::ReadOnly)) {
        qWarning("Could not open password dialog stylesheet");
        return;
    }
    QString styleSheet = QLatin1String(styleFile.readAll());
    setStyleSheet(styleSheet);
}

void PasswordDialog::setupUi() {
    resize(500, 400);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();
    
    m_titleInput = new QLineEdit(this);
    m_usernameInput = new QLineEdit(this);
    m_passwordInput = new QLineEdit(this);
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_urlInput = new QLineEdit(this);
    m_notesInput = new QTextEdit(this);
    m_notesInput->setMaximumHeight(100);
    
    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordLayout->addWidget(m_passwordInput);
    
    m_generateButton = new QPushButton("Generate", this);
    m_generateButton->setObjectName("generateButton");
    m_toggleVisibilityButton = new QPushButton("Show", this);
    
    passwordLayout->addWidget(m_generateButton);
    passwordLayout->addWidget(m_toggleVisibilityButton);
    
    formLayout->addRow("Title:", m_titleInput);
    formLayout->addRow("Username:", m_usernameInput);
    formLayout->addRow("Password:", passwordLayout);
    formLayout->addRow("URL:", m_urlInput);
    formLayout->addRow("Notes:", m_notesInput);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_generateButton, &QPushButton::clicked, this, &PasswordDialog::onGeneratePassword);
    connect(m_toggleVisibilityButton, &QPushButton::clicked, 
            this, &PasswordDialog::onTogglePasswordVisibility);
}

PasswordEntry PasswordDialog::getPasswordEntry() const {
    PasswordEntry entry;
    entry.setTitle(m_titleInput->text());
    entry.setUsername(m_usernameInput->text());
    entry.setPassword(m_passwordInput->text());
    entry.setUrl(m_urlInput->text());
    entry.setNotes(m_notesInput->toPlainText());
    
    return entry;
}

void PasswordDialog::onGeneratePassword() {
    QString password = generateRandomPassword();
    m_passwordInput->setText(password);
    QMessageBox::information(this, "Password Generated", 
        "A strong password has been generated.");
}

void PasswordDialog::onTogglePasswordVisibility() {
    if (m_passwordInput->echoMode() == QLineEdit::Password) {
        m_passwordInput->setEchoMode(QLineEdit::Normal);
        m_toggleVisibilityButton->setText("Hide");
    } else {
        m_passwordInput->setEchoMode(QLineEdit::Password);
        m_toggleVisibilityButton->setText("Show");
    }
}

QString PasswordDialog::generateRandomPassword(int length) {
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*";
    QString password;
    
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        password.append(chars.at(index));
    }
    
    return password;
}