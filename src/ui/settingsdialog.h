#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCloseEvent>
#include "../models/settings.h"

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(AppSettings *appSettings, 
                           VaultSettings *vaultSettings = nullptr,
                           QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;
    void reject() override;

private slots:
    void onCategoryChanged(int index);
    void onApplySettings();
    void onResetToDefaults();
    void onSelectBackupLocation();
    void onTestSync();
    void onConnectSyncAccount();
    void onSettingChanged();

private:
    AppSettings *m_appSettings;
    VaultSettings *m_vaultSettings;
    bool m_hasVault;
    bool m_hasUnsavedChanges;
    
    QListWidget *m_categoryList;
    QStackedWidget *m_contentStack;
    QPushButton *m_applyButton;
    
    // General Settings Widgets
    QComboBox *m_themeCombo;
    QComboBox *m_languageCombo;
    QCheckBox *m_minimizeToTrayCheck;
    QCheckBox *m_startOnBootCheck;
    
    // Security Settings Widgets
    QSpinBox *m_autoLockTimeoutSpin;
    QCheckBox *m_clearClipboardCheck;
    QSpinBox *m_clipboardClearTimeSpin;
    QCheckBox *m_requirePasswordOnWakeCheck;
    QSpinBox *m_passwordStrengthSpin;
    
    // Backup Settings Widgets
    QCheckBox *m_autoBackupCheck;
    QComboBox *m_backupFrequencyCombo;
    QLineEdit *m_backupLocationEdit;
    QPushButton *m_selectBackupButton;
    QSpinBox *m_maxBackupCountSpin;
    
    // Sync Settings Widgets
    QCheckBox *m_syncEnabledCheck;
    QLineEdit *m_syncEmailEdit;
    QComboBox *m_syncOptionCombo;
    QCheckBox *m_autoSyncCheck;
    QPushButton *m_connectAccountButton;
    QPushButton *m_testSyncButton;
    QLabel *m_syncStatusLabel;
    
    // Vault Settings Widgets
    QCheckBox *m_showPasswordStrengthCheck;
    QCheckBox *m_requirePasswordConfirmationCheck;
    QSpinBox *m_defaultPasswordLengthSpin;
    
    // About Info
    QLabel *m_versionLabel;
    QLabel *m_buildDateLabel;
    QLabel *m_qtVersionLabel;
    
    void setupUi();
    void createGeneralPage();
    void createSecurityPage();
    void createBackupPage();
    void createSyncPage();
    void createVaultPage();
    void createAboutPage();
    
    void loadSettings();
    void saveSettings();
    void enableVaultSettings(bool enable);
    void setUnsavedChanges(bool hasChanges);
    void connectSettingSignals();
    bool hasChanges() const;
};

#endif