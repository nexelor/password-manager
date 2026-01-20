#ifndef VAULTMANAGERWINDOW_H
#define VAULTMANAGERWINDOW_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include "../storage/vaultmanager.h"

class VaultManagerWindow : public QWidget {
    Q_OBJECT

public:
    explicit VaultManagerWindow(QWidget *parent = nullptr);
    ~VaultManagerWindow();

private slots:
    void onCreateVault();
    void onOpenExisting();
    void onOpenSelected();
    void onRenameVault();
    void onDeleteVault();
    void onVaultSelectionChanged();
    void onVaultDoubleClicked(QListWidgetItem *item);

private:
    VaultManager *m_vaultManager;
    
    QListWidget *m_vaultList;
    QStackedWidget *m_actionStack;
    
    // No selection actions
    QPushButton *m_createButton;
    QPushButton *m_openExistingButton;
    
    // Selection actions
    QPushButton *m_openButton;
    QPushButton *m_renameButton;
    QPushButton *m_deleteButton;
    
    QLabel *m_titleLabel;
    QLabel *m_infoLabel;
    
    void setupUi();
    void refreshVaultList();
    void openVaultAtPath(const QString &path);
    void showNoSelectionActions();
    void showSelectionActions();
};

#endif