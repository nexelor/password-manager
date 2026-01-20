#ifndef VAULTMANAGER_H
#define VAULTMANAGER_H

#include <QList>
#include <QString>
#include "../models/vaultinfo.h"

class VaultManager {
public:
    VaultManager();

    QList<VaultInfo> getRecentVaults() const;
    void addVault(const VaultInfo &vault);
    void removeVault(const QString &path);
    void updateLastAccessed(const QString &path);
    QString getDefaultVaultPath() const;

private:
    void loadVaults();
    void saveVaults();

    QList<VaultInfo> m_vaults;
};

#endif
