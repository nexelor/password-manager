#include "settings.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

// AppSettings Implementation
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
    m_theme = static_cast<Theme>(m_settings.value("general/theme", Dark).toInt());
    m_language = static_cast<Language>(m_settings.value("general/language", English).toInt());
    m_minimizeToTray = m_settings.value("general/minimizeToTray", false).toBool();
    m_startOnBoot = m_settings.value("general/startOnBoot", false).toBool();
    m_autoLockTimeout = m_settings.value("security/autoLockTimeout", 15).toInt();
    m_clearClipboardAfterCopy = m_settings.value("security/clearClipboardAfterCopy", true).toBool();
    m_clipboardClearTime = m_settings.value("security/clipboardClearTime", 30).toInt();
    m_requireMasterPasswordOnWake = m_settings.value("security/requireMasterPasswordOnWake", true).toBool();
    m_passwordStrengthMinimum = m_settings.value("security/passwordStrengthMinimum", 3).toInt();
}

void AppSettings::save() {
    m_settings.setValue("general/theme", static_cast<int>(m_theme));
    m_settings.setValue("general/language", static_cast<int>(m_language));
    m_settings.setValue("general/minimizeToTray", m_minimizeToTray);
    m_settings.setValue("general/startOnBoot", m_startOnBoot);
    m_settings.setValue("security/autoLockTimeout", m_autoLockTimeout);
    m_settings.setValue("security/clearClipboardAfterCopy", m_clearClipboardAfterCopy);
    m_settings.setValue("security/clipboardClearTime", m_clipboardClearTime);
    m_settings.setValue("security/requireMasterPasswordOnWake", m_requireMasterPasswordOnWake);
    m_settings.setValue("security/passwordStrengthMinimum", m_passwordStrengthMinimum);
    m_settings.sync();
}

void AppSettings::setTheme(Theme theme) {
    m_theme = theme;
    save();
}

void AppSettings::setLanguage(Language language) {
    m_language = language;
    save();
}

void AppSettings::setMinimizeToTray(bool enable) {
    m_minimizeToTray = enable;
    save();
}

void AppSettings::setStartOnBoot(bool enable) {
    m_startOnBoot = enable;
    save();
}

void AppSettings::setAutoLockTimeout(int minutes) {
    m_autoLockTimeout = minutes;
    save();
}

void AppSettings::setClearClipboardAfterCopy(bool enable) {
    m_clearClipboardAfterCopy = enable;
    save();
}

void AppSettings::setClipboardClearTime(int seconds) {
    m_clipboardClearTime = seconds;
    save();
}

void AppSettings::setRequireMasterPasswordOnWake(bool enable) {
    m_requireMasterPasswordOnWake = enable;
    save();
}

void AppSettings::setPasswordStrengthMinimum(int strength) {
    m_passwordStrengthMinimum = strength;
    save();
}

QString AppSettings::qtVersion() {
    return qVersion();
}

// VaultSettings Implementation
VaultSettings::VaultSettings(const QString &vaultPath)
    : m_vaultPath(vaultPath),
      m_autoBackupEnabled(false),
      m_backupFrequency(Weekly),
      m_maxBackupCount(10),
      m_syncEnabled(false),
      m_syncOption(Everything),
      m_autoSyncEnabled(false),
      m_showPasswordStrength(true),
      m_requirePasswordConfirmation(true),
      m_defaultPasswordLength(16),
      m_settings(getSettingsPath(), QSettings::IniFormat) {
    
    QString dataDir = QFileInfo(vaultPath).absolutePath();
    m_backupLocation = dataDir + "/backups";
    
    load();
}

QString VaultSettings::getSettingsPath() const {
    QString baseName = QFileInfo(m_vaultPath).completeBaseName();
    QString dirPath = QFileInfo(m_vaultPath).absolutePath();
    return dirPath + "/" + baseName + ".settings";
}

void VaultSettings::load() {
    m_autoBackupEnabled = m_settings.value("backup/autoBackupEnabled", false).toBool();
    m_backupFrequency = static_cast<BackupFrequency>(
        m_settings.value("backup/backupFrequency", Weekly).toInt());
    m_backupLocation = m_settings.value("backup/backupLocation", m_backupLocation).toString();
    m_maxBackupCount = m_settings.value("backup/maxBackupCount", 10).toInt();
    
    m_syncEnabled = m_settings.value("sync/syncEnabled", false).toBool();
    m_syncAccountEmail = m_settings.value("sync/syncAccountEmail", "").toString();
    m_syncOption = static_cast<SyncOption>(
        m_settings.value("sync/syncOption", Everything).toInt());
    m_autoSyncEnabled = m_settings.value("sync/autoSyncEnabled", false).toBool();
    
    m_showPasswordStrength = m_settings.value("vault/showPasswordStrength", true).toBool();
    m_requirePasswordConfirmation = m_settings.value("vault/requirePasswordConfirmation", true).toBool();
    m_defaultPasswordLength = m_settings.value("vault/defaultPasswordLength", 16).toInt();
}

void VaultSettings::save() {
    m_settings.setValue("backup/autoBackupEnabled", m_autoBackupEnabled);
    m_settings.setValue("backup/backupFrequency", static_cast<int>(m_backupFrequency));
    m_settings.setValue("backup/backupLocation", m_backupLocation);
    m_settings.setValue("backup/maxBackupCount", m_maxBackupCount);
    
    m_settings.setValue("sync/syncEnabled", m_syncEnabled);
    m_settings.setValue("sync/syncAccountEmail", m_syncAccountEmail);
    m_settings.setValue("sync/syncOption", static_cast<int>(m_syncOption));
    m_settings.setValue("sync/autoSyncEnabled", m_autoSyncEnabled);
    
    m_settings.setValue("vault/showPasswordStrength", m_showPasswordStrength);
    m_settings.setValue("vault/requirePasswordConfirmation", m_requirePasswordConfirmation);
    m_settings.setValue("vault/defaultPasswordLength", m_defaultPasswordLength);
    
    m_settings.sync();
}

void VaultSettings::setAutoBackupEnabled(bool enable) {
    m_autoBackupEnabled = enable;
    save();
}

void VaultSettings::setBackupFrequency(BackupFrequency frequency) {
    m_backupFrequency = frequency;
    save();
}

void VaultSettings::setBackupLocation(const QString &location) {
    m_backupLocation = location;
    save();
}

void VaultSettings::setMaxBackupCount(int count) {
    m_maxBackupCount = count;
    save();
}

void VaultSettings::setSyncEnabled(bool enable) {
    m_syncEnabled = enable;
    save();
}

void VaultSettings::setSyncAccountEmail(const QString &email) {
    m_syncAccountEmail = email;
    save();
}

void VaultSettings::setSyncOption(SyncOption option) {
    m_syncOption = option;
    save();
}

void VaultSettings::setAutoSyncEnabled(bool enable) {
    m_autoSyncEnabled = enable;
    save();
}

void VaultSettings::setShowPasswordStrength(bool show) {
    m_showPasswordStrength = show;
    save();
}

void VaultSettings::setRequirePasswordConfirmation(bool require) {
    m_requirePasswordConfirmation = require;
    save();
}

void VaultSettings::setDefaultPasswordLength(int length) {
    m_defaultPasswordLength = length;
    save();
}