#include "vaultmanagerwindow.h"
#include "loginwindow.h"
#include "settingsdialog.h"
#include "thememanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QFileInfo>
#include <QInputDialog>
#include <QFile>

VaultManagerWindow::VaultManagerWindow(AppSettings *appSettings, QWidget *parent)
    : QWidget(parent), m_vaultManager(new VaultManager()), m_appSettings(appSettings) {
    setupUi();
    refreshVaultList();
    showNoSelectionActions();
    
    // Connect to theme changes
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, 
            this, &VaultManagerWindow::onThemeChanged);
}

VaultManagerWindow::~VaultManagerWindow() {
    delete m_vaultManager;
}

void VaultManagerWindow::setupUi() {
    setWindowTitle("Password Manager");
    resize(800, 500);
    setMinimumSize(700, 400);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Left panel - Vault List
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setObjectName("leftPanel");
    leftPanel->setMinimumWidth(300);
    leftPanel->setMaximumWidth(400);
    
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(20, 20, 20, 20);
    leftLayout->setSpacing(15);
    
    // Header with title and settings button
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *vaultListLabel = new QLabel("Your Vaults", leftPanel);
    vaultListLabel->setObjectName("vaultListLabel");
    QFont labelFont = vaultListLabel->font();
    labelFont.setPointSize(14);
    labelFont.setBold(true);
    vaultListLabel->setFont(labelFont);

    m_settingsButton = new QPushButton("⚙", leftPanel);
    m_settingsButton->setObjectName("settingsButton");
    m_settingsButton->setToolTip("Settings");
    m_settingsButton->setMaximumWidth(40);
    m_settingsButton->setMaximumHeight(40);
    QFont settingsFont = m_settingsButton->font();
    settingsFont.setPointSize(16);
    m_settingsButton->setFont(settingsFont);
    
    headerLayout->addWidget(vaultListLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_settingsButton);
    
    m_vaultList = new QListWidget(leftPanel);
    m_vaultList->setObjectName("vaultList");
    
    leftLayout->addLayout(headerLayout);
    leftLayout->addWidget(m_vaultList);
    
    // Right panel - Actions
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setObjectName("rightPanel");
    
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(40, 40, 40, 40);
    rightLayout->setSpacing(20);
    
    // Title
    m_titleLabel = new QLabel("Password Manager", rightPanel);
    m_titleLabel->setObjectName("titleLabel");
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    
    // Info label
    m_infoLabel = new QLabel(rightPanel);
    m_infoLabel->setObjectName("infoLabel");
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    
    // Stacked widget for action buttons
    m_actionStack = new QStackedWidget(rightPanel);
    
    // Page 0: No selection actions
    QWidget *noSelectionPage = new QWidget();
    noSelectionPage->setObjectName("noSelectionPage");
    QVBoxLayout *noSelLayout = new QVBoxLayout(noSelectionPage);
    noSelLayout->setSpacing(15);
    
    m_createButton = new QPushButton("Create New Vault", noSelectionPage);
    m_createButton->setObjectName("createButton");
    m_openExistingButton = new QPushButton("Open Existing Vault", noSelectionPage);
    m_openExistingButton->setObjectName("openExistingButton");
    
    noSelLayout->addStretch();
    noSelLayout->addWidget(m_createButton);
    noSelLayout->addWidget(m_openExistingButton);
    noSelLayout->addStretch();
    
    // Page 1: Selection actions
    QWidget *selectionPage = new QWidget();
    selectionPage->setObjectName("selectionPage");
    QVBoxLayout *selLayout = new QVBoxLayout(selectionPage);
    selLayout->setSpacing(15);
    
    m_openButton = new QPushButton("Open Vault", selectionPage);
    m_openButton->setObjectName("openButton");
    m_renameButton = new QPushButton("Rename Vault", selectionPage);
    m_renameButton->setObjectName("renameButton");
    m_deleteButton = new QPushButton("Delete Vault", selectionPage);
    m_deleteButton->setObjectName("deleteButton");
    
    selLayout->addStretch();
    selLayout->addWidget(m_openButton);
    selLayout->addWidget(m_renameButton);
    selLayout->addWidget(m_deleteButton);
    selLayout->addStretch();
    
    m_actionStack->addWidget(noSelectionPage);
    m_actionStack->addWidget(selectionPage);
    
    rightLayout->addWidget(m_titleLabel);
    rightLayout->addWidget(m_infoLabel);
    rightLayout->addSpacing(20);
    rightLayout->addWidget(m_actionStack);
    rightLayout->addStretch();
    
    // Add panels to main layout
    mainLayout->addWidget(leftPanel, 1);
    mainLayout->addWidget(rightPanel, 2);
    
    // Connections
    connect(m_createButton, &QPushButton::clicked, this, &VaultManagerWindow::onCreateVault);
    connect(m_openExistingButton, &QPushButton::clicked, this, &VaultManagerWindow::onOpenExisting);
    connect(m_openButton, &QPushButton::clicked, this, &VaultManagerWindow::onOpenSelected);
    connect(m_renameButton, &QPushButton::clicked, this, &VaultManagerWindow::onRenameVault);
    connect(m_deleteButton, &QPushButton::clicked, this, &VaultManagerWindow::onDeleteVault);
    connect(m_settingsButton, &QPushButton::clicked, this, &VaultManagerWindow::onOpenSettings);
    connect(m_vaultList, &QListWidget::itemSelectionChanged, this, &VaultManagerWindow::onVaultSelectionChanged);
    connect(m_vaultList, &QListWidget::itemDoubleClicked, this, &VaultManagerWindow::onVaultDoubleClicked);
}

void VaultManagerWindow::refreshVaultList() {
    m_vaultList->clear();
    
    QList<VaultInfo> vaults = m_vaultManager->getRecentVaults();
    
    // Sort by last accessed (most recent first)
    std::sort(vaults.begin(), vaults.end(), [](const VaultInfo &a, const VaultInfo &b) {
        return a.lastAccessed() > b.lastAccessed();
    });
    
    for (const VaultInfo &vault : vaults) {
        QString displayText = QString("%1\n%2")
            .arg(vault.name())
            .arg(vault.lastAccessed().toString("Last accessed: MMM dd, yyyy"));
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, vault.path());
        item->setData(Qt::UserRole + 1, vault.name());
        m_vaultList->addItem(item);
    }
}

void VaultManagerWindow::showNoSelectionActions() {
    m_actionStack->setCurrentIndex(0);
    if (m_vaultList->count() == 0) {
        m_infoLabel->setText("Welcome! Create your first vault or open an existing one.");
    } else {
        m_infoLabel->setText("Select a vault from the list or create a new one.");
    }
}

void VaultManagerWindow::showSelectionActions() {
    m_actionStack->setCurrentIndex(1);
    QListWidgetItem *item = m_vaultList->currentItem();
    if (item) {
        QString vaultName = item->data(Qt::UserRole + 1).toString();
        m_infoLabel->setText(QString("Selected: %1").arg(vaultName));
    }
}

void VaultManagerWindow::onVaultSelectionChanged() {
    if (m_vaultList->currentItem()) {
        showSelectionActions();
    } else {
        showNoSelectionActions();
    }
}

void VaultManagerWindow::onCreateVault() {
    bool ok;
    QString vaultName = QInputDialog::getText(this, "Create Vault",
        "Enter a name for the new vault:", QLineEdit::Normal,
        "My Vault", &ok);
    
    if (!ok || vaultName.isEmpty()) {
        return;
    }
    
    QString defaultPath = m_vaultManager->getDefaultVaultPath();
    QString dirPath = QFileInfo(defaultPath).absolutePath();
    
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Create Vault File",
        dirPath + "/" + vaultName.toLower().replace(" ", "_") + ".vault",
        "Vault Files (*.vault);;All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    if (!fileName.endsWith(".vault")) {
        fileName += ".vault";
    }
    
    VaultInfo newVault(vaultName, fileName, QDateTime::currentDateTime());
    m_vaultManager->addVault(newVault);
    refreshVaultList();
    
    openVaultAtPath(fileName);
}

void VaultManagerWindow::onOpenExisting() {
    QString defaultPath = m_vaultManager->getDefaultVaultPath();
    QString dirPath = QFileInfo(defaultPath).absolutePath();
    
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open Vault File",
        dirPath,
        "Vault Files (*.vault);;All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    bool inList = false;
    QList<VaultInfo> vaults = m_vaultManager->getRecentVaults();
    QString vaultName;
    
    for (const VaultInfo &vault : vaults) {
        if (vault.path() == fileName) {
            inList = true;
            vaultName = vault.name();
            break;
        }
    }
    
    if (!inList) {
        bool ok;
        vaultName = QInputDialog::getText(this, "Vault Name",
            "Enter a name for this vault:", QLineEdit::Normal,
            QFileInfo(fileName).baseName(), &ok);
        
        if (!ok || vaultName.isEmpty()) {
            vaultName = QFileInfo(fileName).baseName();
        }
        
        VaultInfo newVault(vaultName, fileName, QDateTime::currentDateTime());
        m_vaultManager->addVault(newVault);
        refreshVaultList();
    }
    
    openVaultAtPath(fileName);
}

void VaultManagerWindow::onOpenSelected() {
    QListWidgetItem *currentItem = m_vaultList->currentItem();
    if (!currentItem) {
        return;
    }
    
    QString path = currentItem->data(Qt::UserRole).toString();
    
    if (!QFileInfo::exists(path)) {
        QMessageBox::warning(this, "Vault Not Found",
            "The vault file no longer exists at this location.");
        m_vaultManager->removeVault(path);
        refreshVaultList();
        showNoSelectionActions();
        return;
    }
    
    openVaultAtPath(path);
}

void VaultManagerWindow::onVaultDoubleClicked(QListWidgetItem *item) {
    if (item) {
        onOpenSelected();
    }
}

void VaultManagerWindow::onRenameVault() {
    QListWidgetItem *currentItem = m_vaultList->currentItem();
    if (!currentItem) {
        return;
    }
    
    QString path = currentItem->data(Qt::UserRole).toString();
    QString oldName = currentItem->data(Qt::UserRole + 1).toString();
    
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Vault",
        "Enter new name for the vault:", QLineEdit::Normal,
        oldName, &ok);
    
    if (!ok || newName.isEmpty() || newName == oldName) {
        return;
    }
    
    VaultInfo vault(newName, path, QDateTime::currentDateTime());
    m_vaultManager->addVault(vault);
    refreshVaultList();
    
    // Reselect the renamed vault
    for (int i = 0; i < m_vaultList->count(); ++i) {
        if (m_vaultList->item(i)->data(Qt::UserRole).toString() == path) {
            m_vaultList->setCurrentRow(i);
            break;
        }
    }
}

void VaultManagerWindow::onDeleteVault() {
    QListWidgetItem *currentItem = m_vaultList->currentItem();
    if (!currentItem) {
        return;
    }
    
    QString path = currentItem->data(Qt::UserRole).toString();
    QString name = currentItem->data(Qt::UserRole + 1).toString();
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Delete Vault");
    msgBox.setText(QString("Delete vault '%1'?").arg(name));
    msgBox.setInformativeText("This will permanently delete the vault file and all passwords inside it. This action cannot be undone!");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    
    QAbstractButton *yesButton = msgBox.button(QMessageBox::Yes);
    yesButton->setText("Delete Permanently");
    yesButton->setStyleSheet("QPushButton { background-color: #dc3545; color: white; }");
    
    if (msgBox.exec() == QMessageBox::Yes) {
        if (QFile::remove(path)) {
            m_vaultManager->removeVault(path);
            refreshVaultList();
            showNoSelectionActions();
            QMessageBox::information(this, "Vault Deleted", 
                "The vault has been permanently deleted.");
        } else {
            QMessageBox::critical(this, "Error", 
                "Failed to delete the vault file. Check file permissions.");
        }
    }
}

void VaultManagerWindow::onOpenSettings() {
    SettingsDialog dialog(m_appSettings, nullptr, this);
    dialog.exec();
    // Theme changes are applied immediately in the dialog
}

void VaultManagerWindow::onThemeChanged() {
    // Theme is applied globally, window will update automatically
}

void VaultManagerWindow::openVaultAtPath(const QString &path) {
    m_vaultManager->updateLastAccessed(path);
    
    LoginWindow *loginWindow = new LoginWindow(path);
    loginWindow->setAttribute(Qt::WA_DeleteOnClose);
    
    // Hide vault manager when opening vault
    hide();
    
    // Show vault manager when login window closes
    connect(loginWindow, &QObject::destroyed, this, &VaultManagerWindow::showAndRefresh);
    
    loginWindow->show();
}

void VaultManagerWindow::showAndRefresh() {
    refreshVaultList();
    show();
    raise();
    activateWindow();
}