#include "settings.h"
#include "../storage/database.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>

// ========== AppSettings Implementation ==========
// App settings are stored in the system-appropriate location using QSettings
// On Linux: ~/.config/LocalFirst/PasswordManager.conf
// On Windows: Registry or .ini file in AppData
// On macOS: ~/Library/Preferences/com.LocalFirst.PasswordManager.plist

AppSettings::AppSettings()
    : m_theme(Dark),
      m_language(English),
      m_minimizeToTray(false),
      m_startOnBoot(false),
      m_autoLockTimeout(15),
      m_clearClipboardAfterCopy(true),
      m_clipboardClearTime(30),
      m_requireMasterPasswordOnWake(true),
      m_passwordStrengthMinimum(3),
      m_settings("LocalFirst", "PasswordManager") {
    load();
}

void AppSettings::load() {
    // Load general settings
    m_theme = static_cast<Theme>(m_settings.value("app/theme", Dark).toInt());
    m_language = static_cast<Language>(m_settings.value("app/language", English).toInt());
    m_minimizeToTray = m_settings.value("app/minimizeToTray", false).toBool();
    m_startOnBoot = m_settings.value("app/startOnBoot", false).toBool();
    
    // Load security settings
    m_autoLockTimeout = m_settings.value("security/autoLockTimeout", 15).toInt();
    m_clearClipboardAfterCopy = m_settings.value("security/clearClipboardAfterCopy", true).toBool();
    m_clipboardClearTime = m_settings.value("security/clipboardClearTime", 30).toInt();
    m_requireMasterPasswordOnWake = m_settings.value("security/requireMasterPasswordOnWake", true).toBool();
    m_passwordStrengthMinimum = m_settings.value("security/passwordStrengthMinimum", 3).toInt();
    
    qDebug() << "AppSettings loaded from:" << m_settings.fileName();
}

void AppSettings::save() {
    // Save general settings
    m_settings.setValue("app/theme", static_cast<int>(m_theme));
    m_settings.setValue("app/language", static_cast<int>(m_language));
    m_settings.setValue("app/minimizeToTray", m_minimizeToTray);
    m_settings.setValue("app/startOnBoot", m_startOnBoot);
    
    // Save security settings
    m_settings.setValue("security/autoLockTimeout", m_autoLockTimeout);
    m_settings.setValue("security/clearClipboardAfterCopy", m_clearClipboardAfterCopy);
    m_settings.setValue("security/clipboardClearTime", m_clipboardClearTime);
    m_settings.setValue("security/requireMasterPasswordOnWake", m_requireMasterPasswordOnWake);
    m_settings.setValue("security/passwordStrengthMinimum", m_passwordStrengthMinimum);
    
    // Force write to disk
    m_settings.sync();
    
    qDebug() << "AppSettings saved to:" << m_settings.fileName();
}

void AppSettings::setTheme(Theme theme) {
    if (m_theme != theme) {
        m_theme = theme;
        save();
    }
}

void AppSettings::setLanguage(Language language) {
    if (m_language != language) {
        m_language = language;
        save();
    }
}

void AppSettings::setMinimizeToTray(bool enable) {
    if (m_minimizeToTray != enable) {
        m_minimizeToTray = enable;
        save();
    }
}

void AppSettings::setStartOnBoot(bool enable) {
    if (m_startOnBoot != enable) {
        m_startOnBoot = enable;
        save();
    }
}

void AppSettings::setAutoLockTimeout(int minutes) {
    if (m_autoLockTimeout != minutes) {
        m_autoLockTimeout = minutes;
        save();
    }
}

void AppSettings::setClearClipboardAfterCopy(bool enable) {
    if (m_clearClipboardAfterCopy != enable) {
        m_clearClipboardAfterCopy = enable;
        save();
    }
}

void AppSettings::setClipboardClearTime(int seconds) {
    if (m_clipboardClearTime != seconds) {
        m_clipboardClearTime = seconds;
        save();
    }
}

void AppSettings::setRequireMasterPasswordOnWake(bool enable) {
    if (m_requireMasterPasswordOnWake != enable) {
        m_requireMasterPasswordOnWake = enable;
        save();
    }
}

void AppSettings::setPasswordStrengthMinimum(int strength) {
    if (m_passwordStrengthMinimum != strength) {
        m_passwordStrengthMinimum = strength;
        save();
    }
}

QString AppSettings::qtVersion() {
    return qVersion();
}

// ========== VaultSettings Implementation ==========
// Vault settings are now stored INSIDE the vault database in the vault_settings table
// This ensures perfect portability - the vault file is completely self-contained

VaultSettings::VaultSettings(Database *database)
    : m_database(database),
      m_autoBackupEnabled(false),
      m_backupFrequency(Weekly),
      m_maxBackupCount(10),
      m_syncEnabled(false),
      m_syncOption(Everything),
      m_autoSyncEnabled(false),
      m_showPasswordStrength(true),
      m_requirePasswordConfirmation(true),
      m_defaultPasswordLength(16) {
    
    // Set default backup location (will be overridden if saved in DB)
    m_backupLocation = "backups";
    
    load();
}

void VaultSettings::load() {
    if (!m_database || !m_database->isOpen()) {
        qWarning() << "Cannot load VaultSettings: database not available";
        return;
    }

    // Load backup settings
    m_autoBackupEnabled = m_database->getSetting("backup.autoBackupEnabled", false).toBool();
    m_backupFrequency = static_cast<BackupFrequency>(
        m_database->getSetting("backup.backupFrequency", Weekly).toInt());
    m_backupLocation = m_database->getSetting("backup.backupLocation", "backups").toString();
    m_maxBackupCount = m_database->getSetting("backup.maxBackupCount", 10).toInt();
    
    // Load sync settings
    m_syncEnabled = m_database->getSetting("sync.syncEnabled", false).toBool();
    m_syncAccountEmail = m_database->getSetting("sync.syncAccountEmail", "").toString();
    m_syncOption = static_cast<SyncOption>(
        m_database->getSetting("sync.syncOption", Everything).toInt());
    m_autoSyncEnabled = m_database->getSetting("sync.autoSyncEnabled", false).toBool();
    
    // Load vault-specific settings
    m_showPasswordStrength = m_database->getSetting("vault.showPasswordStrength", true).toBool();
    m_requirePasswordConfirmation = m_database->getSetting("vault.requirePasswordConfirmation", true).toBool();
    m_defaultPasswordLength = m_database->getSetting("vault.defaultPasswordLength", 16).toInt();
    
    qDebug() << "VaultSettings loaded from database";
}

void VaultSettings::save() {
    if (!m_database || !m_database->isOpen()) {
        qWarning() << "Cannot save VaultSettings: database not available";
        return;
    }

    // Save backup settings
    m_database->setSetting("backup.autoBackupEnabled", m_autoBackupEnabled);
    m_database->setSetting("backup.backupFrequency", static_cast<int>(m_backupFrequency));
    m_database->setSetting("backup.backupLocation", m_backupLocation);
    m_database->setSetting("backup.maxBackupCount", m_maxBackupCount);
    
    // Save sync settings
    m_database->setSetting("sync.syncEnabled", m_syncEnabled);
    m_database->setSetting("sync.syncAccountEmail", m_syncAccountEmail);
    m_database->setSetting("sync.syncOption", static_cast<int>(m_syncOption));
    m_database->setSetting("sync.autoSyncEnabled", m_autoSyncEnabled);
    
    // Save vault-specific settings
    m_database->setSetting("vault.showPasswordStrength", m_showPasswordStrength);
    m_database->setSetting("vault.requirePasswordConfirmation", m_requirePasswordConfirmation);
    m_database->setSetting("vault.defaultPasswordLength", m_defaultPasswordLength);
    
    qDebug() << "VaultSettings saved to database";
}

void VaultSettings::setAutoBackupEnabled(bool enable) {
    if (m_autoBackupEnabled != enable) {
        m_autoBackupEnabled = enable;
        save();
    }
}

void VaultSettings::setBackupFrequency(BackupFrequency frequency) {
    if (m_backupFrequency != frequency) {
        m_backupFrequency = frequency;
        save();
    }
}

void VaultSettings::setBackupLocation(const QString &location) {
    if (m_backupLocation != location) {
        m_backupLocation = location;
        save();
    }
}

void VaultSettings::setMaxBackupCount(int count) {
    if (m_maxBackupCount != count) {
        m_maxBackupCount = count;
        save();
    }
}

void VaultSettings::setSyncEnabled(bool enable) {
    if (m_syncEnabled != enable) {
        m_syncEnabled = enable;
        save();
    }
}

void VaultSettings::setSyncAccountEmail(const QString &email) {
    if (m_syncAccountEmail != email) {
        m_syncAccountEmail = email;
        save();
    }
}

void VaultSettings::setSyncOption(SyncOption option) {
    if (m_syncOption != option) {
        m_syncOption = option;
        save();
    }
}

void VaultSettings::setAutoSyncEnabled(bool enable) {
    if (m_autoSyncEnabled != enable) {
        m_autoSyncEnabled = enable;
        save();
    }
}

void VaultSettings::setShowPasswordStrength(bool show) {
    if (m_showPasswordStrength != show) {
        m_showPasswordStrength = show;
        save();
    }
}

void VaultSettings::setRequirePasswordConfirmation(bool require) {
    if (m_requirePasswordConfirmation != require) {
        m_requirePasswordConfirmation = require;
        save();
    }
}

void VaultSettings::setDefaultPasswordLength(int length) {
    if (m_defaultPasswordLength != length) {
        m_defaultPasswordLength = length;
        save();
    }
}