#include "loginwindow.h"
#include "mainwindow.h"
#include "../crypto/encryption.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>

LoginWindow::LoginWindow(const QString &vaultPath)
    : QWidget(nullptr), m_database(new Database()), m_vaultPath(vaultPath) {
    setAttribute(Qt::WA_DeleteOnClose);
    setupUi();
    loadStyleSheet();
    
    if (!m_database->open(m_vaultPath)) {
        QMessageBox::critical(this, "Error", 
            QString("Failed to open vault at:\n%1").arg(m_vaultPath));
        close();
        return;
    }
    
    if (checkIfVaultExists()) {
        m_createVaultButton->setEnabled(false);
        m_statusLabel->setText(QString("Vault: %1").arg(QFileInfo(m_vaultPath).fileName()));
    } else {
        m_loginButton->setEnabled(false);
        m_statusLabel->setText("Initialize this vault with a master password");
    }
}

LoginWindow::~LoginWindow() {
    if (m_database) {
        delete m_database;
    }
}

void LoginWindow::loadStyleSheet() {
    QFile styleFile(":/src/styles/loginwindow.qss");
    if (!styleFile.open(QFile::ReadOnly)) {
        qWarning("Could not open login window stylesheet");
        return;
    }
    QString styleSheet = QLatin1String(styleFile.readAll());
    setStyleSheet(styleSheet);
}

void LoginWindow::setupUi() {
    setWindowTitle("Password Manager - Unlock Vault");
    setMinimumSize(450, 280);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    
    QLabel *titleLabel = new QLabel("Password Manager", this);
    titleLabel->setObjectName("titleLabel");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *passwordLabel = new QLabel("Master Password:", this);
    m_passwordInput = new QLineEdit(this);
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setPlaceholderText("Enter your master password");
    
    m_loginButton = new QPushButton("Unlock Vault", this);
    m_createVaultButton = new QPushButton("Initialize Vault", this);
    
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(m_passwordInput);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_loginButton);
    mainLayout->addWidget(m_createVaultButton);
    mainLayout->addStretch();
    
    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_createVaultButton, &QPushButton::clicked, this, &LoginWindow::onCreateVaultClicked);
    connect(m_passwordInput, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
}

bool LoginWindow::checkIfVaultExists() {
    QByteArray salt = m_database->getUserSalt();
    return !salt.isEmpty();
}

void LoginWindow::onCreateVaultClicked() {
    QString password = m_passwordInput->text();
    
    if (password.length() < 8) {
        QMessageBox::warning(this, "Weak Password", 
            "Master password must be at least 8 characters long.");
        return;
    }
    
    createVault(password);
}

void LoginWindow::onLoginClicked() {
    QString password = m_passwordInput->text();
    
    if (password.isEmpty()) {
        QMessageBox::warning(this, "Empty Password", 
            "Please enter your master password.");
        return;
    }
    
    unlockVault(password);
}

void LoginWindow::createVault(const QString &masterPassword) {
    QByteArray salt = Encryption::generateSalt();
    QString passwordHash = Encryption::hashPassword(masterPassword);
    
    if (m_database->createUser(passwordHash, salt)) {
        QMessageBox::information(this, "Success", 
            "Vault initialized successfully! You can now unlock it.");
        
        m_createVaultButton->setEnabled(false);
        m_loginButton->setEnabled(true);
        m_statusLabel->setText(QString("Vault: %1").arg(QFileInfo(m_vaultPath).fileName()));
        m_passwordInput->clear();
    } else {
        QMessageBox::critical(this, "Error", 
            "Failed to initialize vault. Please try again.");
    }
}

void LoginWindow::unlockVault(const QString &masterPassword) {
    QString passwordHash = Encryption::hashPassword(masterPassword);
    
    if (m_database->verifyUser(passwordHash)) {
        QByteArray salt = m_database->getUserSalt();
        QByteArray masterKey = Encryption::deriveMasterKey(masterPassword, salt);
        
        MainWindow *mainWindow = new MainWindow(m_database, masterKey);
        mainWindow->setAttribute(Qt::WA_DeleteOnClose);
        mainWindow->show();
        
        // Transfer ownership of database to main window
        m_database = nullptr;
        
        // Hide login window but don't close it
        hide();
        
        // When main window closes, close login window (which triggers vault manager to show)
        connect(mainWindow, &QObject::destroyed, this, &LoginWindow::onMainWindowClosed);
    } else {
        QMessageBox::warning(this, "Authentication Failed", 
            "Incorrect master password. Please try again.");
        m_passwordInput->clear();
        m_passwordInput->setFocus();
    }
}

void LoginWindow::onMainWindowClosed() {
    // Close login window, which will trigger the vault manager to show
    close();
}