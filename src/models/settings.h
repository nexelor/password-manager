#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QSettings>

class AppSettings {
public:
    enum Theme {
        Dark,
        Light,
        System
    };
    
    enum Language {
        English,
        French,
        Spanish,
        German
    };
    
    AppSettings();
    
    // General Settings
    Theme theme() const { return m_theme; }
    void setTheme(Theme theme);
    
    Language language() const { return m_language; }
    void setLanguage(Language language);
    
    bool minimizeToTray() const { return m_minimizeToTray; }
    void setMinimizeToTray(bool enable);
    
    bool startOnBoot() const { return m_startOnBoot; }
    void setStartOnBoot(bool enable);
    
    int autoLockTimeout() const { return m_autoLockTimeout; }
    void setAutoLockTimeout(int minutes);
    
    bool clearClipboardAfterCopy() const { return m_clearClipboardAfterCopy; }
    void setClearClipboardAfterCopy(bool enable);
    
    int clipboardClearTime() const { return m_clipboardClearTime; }
    void setClipboardClearTime(int seconds);
    
    // Security Settings
    bool requireMasterPasswordOnWake() const { return m_requireMasterPasswordOnWake; }
    void setRequireMasterPasswordOnWake(bool enable);
    
    int passwordStrengthMinimum() const { return m_passwordStrengthMinimum; }
    void setPasswordStrengthMinimum(int strength);
    
    // Application Info
    static QString version() { return "1.0.0"; }
    static QString buildDate() { return __DATE__; }
    static QString qtVersion();
    
    void load();
    void save();
    
private:
    Theme m_theme;
    Language m_language;
    bool m_minimizeToTray;
    bool m_startOnBoot;
    int m_autoLockTimeout;
    bool m_clearClipboardAfterCopy;
    int m_clipboardClearTime;
    bool m_requireMasterPasswordOnWake;
    int m_passwordStrengthMinimum;
    
    QSettings m_settings;
};

class VaultSettings {
public:
    enum BackupFrequency {
        Never,
        Daily,
        Weekly,
        Monthly
    };
    
    enum SyncOption {
        PasswordsOnly,
        SettingsOnly,
        Everything
    };
    
    VaultSettings(const QString &vaultPath);
    
    // Backup Settings
    bool autoBackupEnabled() const { return m_autoBackupEnabled; }
    void setAutoBackupEnabled(bool enable);
    
    BackupFrequency backupFrequency() const { return m_backupFrequency; }
    void setBackupFrequency(BackupFrequency frequency);
    
    QString backupLocation() const { return m_backupLocation; }
    void setBackupLocation(const QString &location);
    
    int maxBackupCount() const { return m_maxBackupCount; }
    void setMaxBackupCount(int count);
    
    // Sync Settings
    bool syncEnabled() const { return m_syncEnabled; }
    void setSyncEnabled(bool enable);
    
    QString syncAccountEmail() const { return m_syncAccountEmail; }
    void setSyncAccountEmail(const QString &email);
    
    SyncOption syncOption() const { return m_syncOption; }
    void setSyncOption(SyncOption option);
    
    bool autoSyncEnabled() const { return m_autoSyncEnabled; }
    void setAutoSyncEnabled(bool enable);
    
    // Vault Specific Settings
    bool showPasswordStrength() const { return m_showPasswordStrength; }
    void setShowPasswordStrength(bool show);
    
    bool requirePasswordConfirmation() const { return m_requirePasswordConfirmation; }
    void setRequirePasswordConfirmation(bool require);
    
    int defaultPasswordLength() const { return m_defaultPasswordLength; }
    void setDefaultPasswordLength(int length);
    
    void load();
    void save();
    
private:
    QString m_vaultPath;
    bool m_autoBackupEnabled;
    BackupFrequency m_backupFrequency;
    QString m_backupLocation;
    int m_maxBackupCount;
    bool m_syncEnabled;
    QString m_syncAccountEmail;
    SyncOption m_syncOption;
    bool m_autoSyncEnabled;
    bool m_showPasswordStrength;
    bool m_requirePasswordConfirmation;
    int m_defaultPasswordLength;
    
    QSettings m_settings;
    QString getSettingsPath() const;
};

#endif