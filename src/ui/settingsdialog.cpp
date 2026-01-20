#include "settingsdialog.h"
#include "thememanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QCloseEvent>

SettingsDialog::SettingsDialog(AppSettings *appSettings, 
                               VaultSettings *vaultSettings,
                               QWidget *parent)
    : QDialog(parent), 
      m_appSettings(appSettings),
      m_vaultSettings(vaultSettings),
      m_hasVault(vaultSettings != nullptr),
      m_hasUnsavedChanges(false) {
    setupUi();
    loadSettings();
    connectSettingSignals();
    setUnsavedChanges(false); // Initialize with no changes
}

void SettingsDialog::setupUi() {
    setWindowTitle("Settings");
    resize(800, 600);
    setMinimumSize(700, 500);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Left sidebar with categories
    m_categoryList = new QListWidget(this);
    m_categoryList->setObjectName("categoryList");
    m_categoryList->setMaximumWidth(200);
    m_categoryList->addItem("General");
    m_categoryList->addItem("Security");
    m_categoryList->addItem("Backup");
    m_categoryList->addItem("Sync");
    m_categoryList->addItem("Vault");
    m_categoryList->addItem("About");
    
    // Right content area
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setObjectName("rightPanel");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(20, 20, 20, 20);
    
    m_contentStack = new QStackedWidget(this);
    
    createGeneralPage();
    createSecurityPage();
    createBackupPage();
    createSyncPage();
    createVaultPage();
    createAboutPage();
    
    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    m_applyButton = buttonBox->addButton("Apply", QDialogButtonBox::ApplyRole);
    QPushButton *okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    QPushButton *cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    QPushButton *resetButton = buttonBox->addButton("Reset to Defaults", QDialogButtonBox::ResetRole);
    
    m_applyButton->setEnabled(false); // Initially disabled
    
    rightLayout->addWidget(m_contentStack);
    rightLayout->addWidget(buttonBox);
    
    mainLayout->addWidget(m_categoryList);
    mainLayout->addWidget(rightPanel, 1);
    
    // Connections
    connect(m_categoryList, &QListWidget::currentRowChanged, 
            this, &SettingsDialog::onCategoryChanged);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplySettings);
    connect(okButton, &QPushButton::clicked, [this]() {
        if (m_hasUnsavedChanges) {
            onApplySettings();
        }
        accept();
    });
    connect(cancelButton, &QPushButton::clicked, this, &SettingsDialog::reject);
    connect(resetButton, &QPushButton::clicked, this, &SettingsDialog::onResetToDefaults);
    
    m_categoryList->setCurrentRow(0);
    
    // Enable/disable vault-specific settings
    enableVaultSettings(m_hasVault);
}

void SettingsDialog::createGeneralPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    
    QLabel *titleLabel = new QLabel("General Settings");
    titleLabel->setObjectName("pageTitle");
    
    QGroupBox *appearanceGroup = new QGroupBox("Appearance");
    QFormLayout *appearanceLayout = new QFormLayout(appearanceGroup);
    
    m_themeCombo = new QComboBox();
    m_themeCombo->addItems({"Dark", "Light", "System"});
    
    m_languageCombo = new QComboBox();
    m_languageCombo->addItems({"English", "French", "Spanish", "German"});
    
    appearanceLayout->addRow("Theme:", m_themeCombo);
    appearanceLayout->addRow("Language:", m_languageCombo);
    
    QGroupBox *behaviorGroup = new QGroupBox("Behavior");
    QVBoxLayout *behaviorLayout = new QVBoxLayout(behaviorGroup);
    
    m_minimizeToTrayCheck = new QCheckBox("Minimize to system tray");
    m_startOnBootCheck = new QCheckBox("Start on system boot");
    
    behaviorLayout->addWidget(m_minimizeToTrayCheck);
    behaviorLayout->addWidget(m_startOnBootCheck);
    
    layout->addWidget(titleLabel);
    layout->addWidget(appearanceGroup);
    layout->addWidget(behaviorGroup);
    layout->addStretch();
    
    m_contentStack->addWidget(page);
}

void SettingsDialog::createSecurityPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    
    QLabel *titleLabel = new QLabel("Security Settings");
    titleLabel->setObjectName("pageTitle");
    
    QGroupBox *lockGroup = new QGroupBox("Auto-Lock");
    QFormLayout *lockLayout = new QFormLayout(lockGroup);
    
    m_autoLockTimeoutSpin = new QSpinBox();
    m_autoLockTimeoutSpin->setRange(1, 120);
    m_autoLockTimeoutSpin->setSuffix(" minutes");
    
    m_requirePasswordOnWakeCheck = new QCheckBox("Require password after system wake");
    
    lockLayout->addRow("Auto-lock timeout:", m_autoLockTimeoutSpin);
    lockLayout->addRow("", m_requirePasswordOnWakeCheck);
    
    QGroupBox *clipboardGroup = new QGroupBox("Clipboard");
    QFormLayout *clipboardLayout = new QFormLayout(clipboardGroup);
    
    m_clearClipboardCheck = new QCheckBox("Clear clipboard after copy");
    m_clipboardClearTimeSpin = new QSpinBox();
    m_clipboardClearTimeSpin->setRange(5, 300);
    m_clipboardClearTimeSpin->setSuffix(" seconds");
    
    clipboardLayout->addRow("", m_clearClipboardCheck);
    clipboardLayout->addRow("Clear after:", m_clipboardClearTimeSpin);
    
    QGroupBox *passwordGroup = new QGroupBox("Password Strength");
    QFormLayout *passwordLayout = new QFormLayout(passwordGroup);
    
    m_passwordStrengthSpin = new QSpinBox();
    m_passwordStrengthSpin->setRange(1, 5);
    
    passwordLayout->addRow("Minimum strength:", m_passwordStrengthSpin);
    
    layout->addWidget(titleLabel);
    layout->addWidget(lockGroup);
    layout->addWidget(clipboardGroup);
    layout->addWidget(passwordGroup);
    layout->addStretch();
    
    connect(m_clearClipboardCheck, &QCheckBox::toggled, 
            m_clipboardClearTimeSpin, &QWidget::setEnabled);
    
    m_contentStack->addWidget(page);
}

void SettingsDialog::createBackupPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    
    QLabel *titleLabel = new QLabel("Backup Settings");
    titleLabel->setObjectName("pageTitle");
    
    QLabel *infoLabel = new QLabel("These settings are vault-specific");
    infoLabel->setObjectName("infoLabel");
    
    QGroupBox *backupGroup = new QGroupBox("Automatic Backup");
    QFormLayout *backupLayout = new QFormLayout(backupGroup);
    
    m_autoBackupCheck = new QCheckBox("Enable automatic backups");
    
    m_backupFrequencyCombo = new QComboBox();
    m_backupFrequencyCombo->addItems({"Never", "Daily", "Weekly", "Monthly"});
    
    QHBoxLayout *locationLayout = new QHBoxLayout();
    m_backupLocationEdit = new QLineEdit();
    m_backupLocationEdit->setReadOnly(true);
    m_selectBackupButton = new QPushButton("Browse...");
    locationLayout->addWidget(m_backupLocationEdit);
    locationLayout->addWidget(m_selectBackupButton);
    
    m_maxBackupCountSpin = new QSpinBox();
    m_maxBackupCountSpin->setRange(1, 100);
    
    backupLayout->addRow("", m_autoBackupCheck);
    backupLayout->addRow("Frequency:", m_backupFrequencyCombo);
    backupLayout->addRow("Location:", locationLayout);
    backupLayout->addRow("Keep backups:", m_maxBackupCountSpin);
    
    layout->addWidget(titleLabel);
    layout->addWidget(infoLabel);
    layout->addWidget(backupGroup);
    layout->addStretch();
    
    connect(m_selectBackupButton, &QPushButton::clicked, 
            this, &SettingsDialog::onSelectBackupLocation);
    connect(m_autoBackupCheck, &QCheckBox::toggled, [this](bool checked) {
        m_backupFrequencyCombo->setEnabled(checked);
        m_backupLocationEdit->setEnabled(checked);
        m_selectBackupButton->setEnabled(checked);
        m_maxBackupCountSpin->setEnabled(checked);
    });
    
    m_contentStack->addWidget(page);
}

void SettingsDialog::createSyncPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    
    QLabel *titleLabel = new QLabel("Sync Settings");
    titleLabel->setObjectName("pageTitle");
    
    QLabel *infoLabel = new QLabel("These settings are vault-specific. Sync feature coming soon!");
    infoLabel->setObjectName("infoLabel");
    
    QGroupBox *accountGroup = new QGroupBox("Sync Account");
    QFormLayout *accountLayout = new QFormLayout(accountGroup);
    
    m_syncEnabledCheck = new QCheckBox("Enable sync");
    
    m_syncEmailEdit = new QLineEdit();
    m_syncEmailEdit->setPlaceholderText("your@email.com");
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_connectAccountButton = new QPushButton("Connect Account");
    m_testSyncButton = new QPushButton("Test Connection");
    buttonLayout->addWidget(m_connectAccountButton);
    buttonLayout->addWidget(m_testSyncButton);
    buttonLayout->addStretch();
    
    m_syncStatusLabel = new QLabel("Status: Not connected");
    m_syncStatusLabel->setObjectName("syncStatusLabel");
    
    accountLayout->addRow("", m_syncEnabledCheck);
    accountLayout->addRow("Email:", m_syncEmailEdit);
    accountLayout->addRow("", buttonLayout);
    accountLayout->addRow("", m_syncStatusLabel);
    
    QGroupBox *optionsGroup = new QGroupBox("Sync Options");
    QFormLayout *optionsLayout = new QFormLayout(optionsGroup);
    
    m_syncOptionCombo = new QComboBox();
    m_syncOptionCombo->addItems({"Passwords Only", "Settings Only", "Everything"});
    
    m_autoSyncCheck = new QCheckBox("Automatically sync changes");
    
    optionsLayout->addRow("What to sync:", m_syncOptionCombo);
    optionsLayout->addRow("", m_autoSyncCheck);
    
    layout->addWidget(titleLabel);
    layout->addWidget(infoLabel);
    layout->addWidget(accountGroup);
    layout->addWidget(optionsGroup);
    layout->addStretch();
    
    connect(m_connectAccountButton, &QPushButton::clicked, 
            this, &SettingsDialog::onConnectSyncAccount);
    connect(m_testSyncButton, &QPushButton::clicked, 
            this, &SettingsDialog::onTestSync);
    connect(m_syncEnabledCheck, &QCheckBox::toggled, [this](bool checked) {
        m_syncEmailEdit->setEnabled(checked);
        m_connectAccountButton->setEnabled(checked);
        m_testSyncButton->setEnabled(checked);
        m_syncOptionCombo->setEnabled(checked);
        m_autoSyncCheck->setEnabled(checked);
    });
    
    m_contentStack->addWidget(page);
}

void SettingsDialog::createVaultPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    
    QLabel *titleLabel = new QLabel("Vault Settings");
    titleLabel->setObjectName("pageTitle");
    
    QLabel *infoLabel = new QLabel("These settings are vault-specific");
    infoLabel->setObjectName("infoLabel");
    
    QGroupBox *passwordGroup = new QGroupBox("Password Generation");
    QFormLayout *passwordLayout = new QFormLayout(passwordGroup);
    
    m_defaultPasswordLengthSpin = new QSpinBox();
    m_defaultPasswordLengthSpin->setRange(8, 128);
    
    m_showPasswordStrengthCheck = new QCheckBox("Show password strength indicator");
    m_requirePasswordConfirmationCheck = new QCheckBox("Require password confirmation when generating");
    
    passwordLayout->addRow("Default length:", m_defaultPasswordLengthSpin);
    passwordLayout->addRow("", m_showPasswordStrengthCheck);
    passwordLayout->addRow("", m_requirePasswordConfirmationCheck);
    
    layout->addWidget(titleLabel);
    layout->addWidget(infoLabel);
    layout->addWidget(passwordGroup);
    layout->addStretch();
    
    m_contentStack->addWidget(page);
}

void SettingsDialog::createAboutPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    
    QLabel *titleLabel = new QLabel("Password Manager");
    titleLabel->setObjectName("aboutTitle");
    
    m_versionLabel = new QLabel();
    m_buildDateLabel = new QLabel();
    m_qtVersionLabel = new QLabel();
    
    m_versionLabel->setObjectName("aboutInfo");
    m_buildDateLabel->setObjectName("aboutInfo");
    m_qtVersionLabel->setObjectName("aboutInfo");
    
    QLabel *licenseLabel = new QLabel("Licensed under MIT License");
    licenseLabel->setObjectName("aboutInfo");
    
    QLabel *copyrightLabel = new QLabel("Copyright © 2025 nexelor");
    copyrightLabel->setObjectName("aboutInfo");
    
    QLabel *descLabel = new QLabel("A secure, local-first password manager");
    descLabel->setObjectName("aboutDesc");
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignCenter);
    
    layout->addSpacing(40);
    layout->addWidget(titleLabel, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(m_versionLabel, 0, Qt::AlignCenter);
    layout->addWidget(m_buildDateLabel, 0, Qt::AlignCenter);
    layout->addWidget(m_qtVersionLabel, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(descLabel, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(licenseLabel, 0, Qt::AlignCenter);
    layout->addWidget(copyrightLabel, 0, Qt::AlignCenter);
    layout->addStretch();
    
    m_contentStack->addWidget(page);
}

void SettingsDialog::loadSettings() {
    // Load app settings
    m_themeCombo->setCurrentIndex(static_cast<int>(m_appSettings->theme()));
    m_languageCombo->setCurrentIndex(static_cast<int>(m_appSettings->language()));
    m_minimizeToTrayCheck->setChecked(m_appSettings->minimizeToTray());
    m_startOnBootCheck->setChecked(m_appSettings->startOnBoot());
    
    m_autoLockTimeoutSpin->setValue(m_appSettings->autoLockTimeout());
    m_clearClipboardCheck->setChecked(m_appSettings->clearClipboardAfterCopy());
    m_clipboardClearTimeSpin->setValue(m_appSettings->clipboardClearTime());
    m_clipboardClearTimeSpin->setEnabled(m_clearClipboardCheck->isChecked());
    m_requirePasswordOnWakeCheck->setChecked(m_appSettings->requireMasterPasswordOnWake());
    m_passwordStrengthSpin->setValue(m_appSettings->passwordStrengthMinimum());
    
    // Load vault settings if available
    if (m_vaultSettings) {
        m_autoBackupCheck->setChecked(m_vaultSettings->autoBackupEnabled());
        m_backupFrequencyCombo->setCurrentIndex(static_cast<int>(m_vaultSettings->backupFrequency()));
        m_backupLocationEdit->setText(m_vaultSettings->backupLocation());
        m_maxBackupCountSpin->setValue(m_vaultSettings->maxBackupCount());
        
        bool backupEnabled = m_autoBackupCheck->isChecked();
        m_backupFrequencyCombo->setEnabled(backupEnabled);
        m_backupLocationEdit->setEnabled(backupEnabled);
        m_selectBackupButton->setEnabled(backupEnabled);
        m_maxBackupCountSpin->setEnabled(backupEnabled);
        
        m_syncEnabledCheck->setChecked(m_vaultSettings->syncEnabled());
        m_syncEmailEdit->setText(m_vaultSettings->syncAccountEmail());
        m_syncOptionCombo->setCurrentIndex(static_cast<int>(m_vaultSettings->syncOption()));
        m_autoSyncCheck->setChecked(m_vaultSettings->autoSyncEnabled());
        
        bool syncEnabled = m_syncEnabledCheck->isChecked();
        m_syncEmailEdit->setEnabled(syncEnabled);
        m_connectAccountButton->setEnabled(syncEnabled);
        m_testSyncButton->setEnabled(syncEnabled);
        m_syncOptionCombo->setEnabled(syncEnabled);
        m_autoSyncCheck->setEnabled(syncEnabled);
        
        m_showPasswordStrengthCheck->setChecked(m_vaultSettings->showPasswordStrength());
        m_requirePasswordConfirmationCheck->setChecked(m_vaultSettings->requirePasswordConfirmation());
        m_defaultPasswordLengthSpin->setValue(m_vaultSettings->defaultPasswordLength());
    }
    
    // Load about info
    m_versionLabel->setText(QString("Version %1").arg(AppSettings::version()));
    m_buildDateLabel->setText(QString("Built on %1").arg(AppSettings::buildDate()));
    m_qtVersionLabel->setText(QString("Qt %1").arg(AppSettings::qtVersion()));
}

void SettingsDialog::saveSettings() {
    // Save app settings
    m_appSettings->setTheme(static_cast<AppSettings::Theme>(m_themeCombo->currentIndex()));
    m_appSettings->setLanguage(static_cast<AppSettings::Language>(m_languageCombo->currentIndex()));
    m_appSettings->setMinimizeToTray(m_minimizeToTrayCheck->isChecked());
    m_appSettings->setStartOnBoot(m_startOnBootCheck->isChecked());
    
    m_appSettings->setAutoLockTimeout(m_autoLockTimeoutSpin->value());
    m_appSettings->setClearClipboardAfterCopy(m_clearClipboardCheck->isChecked());
    m_appSettings->setClipboardClearTime(m_clipboardClearTimeSpin->value());
    m_appSettings->setRequireMasterPasswordOnWake(m_requirePasswordOnWakeCheck->isChecked());
    m_appSettings->setPasswordStrengthMinimum(m_passwordStrengthSpin->value());
    
    // Apply theme immediately
    ThemeManager::instance()->applyTheme(m_appSettings->theme());
    
    // Save vault settings if available
    if (m_vaultSettings) {
        m_vaultSettings->setAutoBackupEnabled(m_autoBackupCheck->isChecked());
        m_vaultSettings->setBackupFrequency(
            static_cast<VaultSettings::BackupFrequency>(m_backupFrequencyCombo->currentIndex()));
        m_vaultSettings->setBackupLocation(m_backupLocationEdit->text());
        m_vaultSettings->setMaxBackupCount(m_maxBackupCountSpin->value());
        
        m_vaultSettings->setSyncEnabled(m_syncEnabledCheck->isChecked());
        m_vaultSettings->setSyncAccountEmail(m_syncEmailEdit->text());
        m_vaultSettings->setSyncOption(
            static_cast<VaultSettings::SyncOption>(m_syncOptionCombo->currentIndex()));
        m_vaultSettings->setAutoSyncEnabled(m_autoSyncCheck->isChecked());
        
        m_vaultSettings->setShowPasswordStrength(m_showPasswordStrengthCheck->isChecked());
        m_vaultSettings->setRequirePasswordConfirmation(m_requirePasswordConfirmationCheck->isChecked());
        m_vaultSettings->setDefaultPasswordLength(m_defaultPasswordLengthSpin->value());
    }
}

void SettingsDialog::onCategoryChanged(int index) {
    m_contentStack->setCurrentIndex(index);
}

void SettingsDialog::onApplySettings() {
    saveSettings();
    setUnsavedChanges(false);
    QMessageBox::information(this, "Settings Saved", 
        "Your settings have been saved successfully.");
}

void SettingsDialog::onResetToDefaults() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, 
        "Reset Settings",
        "Are you sure you want to reset all settings to their default values?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Reset app settings to defaults
        m_themeCombo->setCurrentIndex(0);
        m_languageCombo->setCurrentIndex(0);
        m_minimizeToTrayCheck->setChecked(false);
        m_startOnBootCheck->setChecked(false);
        m_autoLockTimeoutSpin->setValue(15);
        m_clearClipboardCheck->setChecked(true);
        m_clipboardClearTimeSpin->setValue(30);
        m_requirePasswordOnWakeCheck->setChecked(true);
        m_passwordStrengthSpin->setValue(3);
        
        if (m_vaultSettings) {
            m_autoBackupCheck->setChecked(false);
            m_backupFrequencyCombo->setCurrentIndex(2);
            m_maxBackupCountSpin->setValue(10);
            m_syncEnabledCheck->setChecked(false);
            m_syncOptionCombo->setCurrentIndex(2);
            m_autoSyncCheck->setChecked(false);
            m_showPasswordStrengthCheck->setChecked(true);
            m_requirePasswordConfirmationCheck->setChecked(true);
            m_defaultPasswordLengthSpin->setValue(16);
        }
        
        setUnsavedChanges(true);
    }
}

void SettingsDialog::onSelectBackupLocation() {
    QString dir = QFileDialog::getExistingDirectory(this, 
        "Select Backup Location",
        m_backupLocationEdit->text());
    
    if (!dir.isEmpty()) {
        m_backupLocationEdit->setText(dir);
    }
}

void SettingsDialog::onTestSync() {
    QMessageBox::information(this, "Sync Test",
        "Sync feature is not yet implemented. Coming soon!");
}

void SettingsDialog::onConnectSyncAccount() {
    QMessageBox::information(this, "Connect Account",
        "Account connection feature is not yet implemented. Coming soon!");
}

void SettingsDialog::enableVaultSettings(bool enable) {
    // Disable vault-specific categories if no vault is open
    for (int i = 2; i <= 4; ++i) { // Backup, Sync, Vault pages
        QListWidgetItem *item = m_categoryList->item(i);
        if (!enable) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            item->setToolTip("Open a vault to access these settings");
        } else {
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
            item->setToolTip("");
        }
    }
}

void SettingsDialog::setUnsavedChanges(bool hasChanges) {
    m_hasUnsavedChanges = hasChanges;
    m_applyButton->setEnabled(hasChanges);
}

void SettingsDialog::onSettingChanged() {
    setUnsavedChanges(true);
}

void SettingsDialog::connectSettingSignals() {
    // General Settings
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &SettingsDialog::onSettingChanged);
    connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &SettingsDialog::onSettingChanged);
    connect(m_minimizeToTrayCheck, &QCheckBox::toggled, 
            this, &SettingsDialog::onSettingChanged);
    connect(m_startOnBootCheck, &QCheckBox::toggled, 
            this, &SettingsDialog::onSettingChanged);
    
    // Security Settings
    connect(m_autoLockTimeoutSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &SettingsDialog::onSettingChanged);
    connect(m_clearClipboardCheck, &QCheckBox::toggled, 
            this, &SettingsDialog::onSettingChanged);
    connect(m_clipboardClearTimeSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &SettingsDialog::onSettingChanged);
    connect(m_requirePasswordOnWakeCheck, &QCheckBox::toggled, 
            this, &SettingsDialog::onSettingChanged);
    connect(m_passwordStrengthSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &SettingsDialog::onSettingChanged);
    
    if (m_vaultSettings) {
        // Backup Settings
        connect(m_autoBackupCheck, &QCheckBox::toggled, 
                this, &SettingsDialog::onSettingChanged);
        connect(m_backupFrequencyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                this, &SettingsDialog::onSettingChanged);
        connect(m_backupLocationEdit, &QLineEdit::textChanged, 
                this, &SettingsDialog::onSettingChanged);
        connect(m_maxBackupCountSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
                this, &SettingsDialog::onSettingChanged);
        
        // Sync Settings
        connect(m_syncEnabledCheck, &QCheckBox::toggled, 
                this, &SettingsDialog::onSettingChanged);
        connect(m_syncEmailEdit, &QLineEdit::textChanged, 
                this, &SettingsDialog::onSettingChanged);
        connect(m_syncOptionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                this, &SettingsDialog::onSettingChanged);
        connect(m_autoSyncCheck, &QCheckBox::toggled, 
                this, &SettingsDialog::onSettingChanged);
        
        // Vault Settings
        connect(m_showPasswordStrengthCheck, &QCheckBox::toggled, 
                this, &SettingsDialog::onSettingChanged);
        connect(m_requirePasswordConfirmationCheck, &QCheckBox::toggled, 
                this, &SettingsDialog::onSettingChanged);
        connect(m_defaultPasswordLengthSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
                this, &SettingsDialog::onSettingChanged);
    }
}

void SettingsDialog::closeEvent(QCloseEvent *event) {
    if (m_hasUnsavedChanges) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Unsaved Changes",
            "You have unsaved changes. Do you want to save them before closing?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (reply == QMessageBox::Save) {
            saveSettings();
            setUnsavedChanges(false);
            event->accept();
        } else if (reply == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void SettingsDialog::reject() {
    if (m_hasUnsavedChanges) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Unsaved Changes",
            "You have unsaved changes. Do you want to save them before closing?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (reply == QMessageBox::Save) {
            saveSettings();
            setUnsavedChanges(false);
            QDialog::reject();
        } else if (reply == QMessageBox::Discard) {
            QDialog::reject();
        }
        // If Cancel, do nothing (dialog stays open)
    } else {
        QDialog::reject();
    }
}