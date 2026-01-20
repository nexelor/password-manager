#include "vaultmanager.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

VaultManager::VaultManager() {
    loadVaults();
}

QString VaultManager::getDefaultVaultPath() const {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    return dataDir + "/default.vault";
}

void VaultManager::loadVaults() {
    QSettings settings;
    int size = settings.beginReadArray("vaults");
    
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        VaultInfo vault;
        vault.setName(settings.value("name").toString());
        vault.setPath(settings.value("path").toString());
        vault.setLastAccessed(settings.value("lastAccessed").toDateTime());
        
        // Only add if file still exists
        if (QFileInfo::exists(vault.path())) {
            m_vaults.append(vault);
        }
    }
    
    settings.endArray();
}

void VaultManager::saveVaults() {
    QSettings settings;
    settings.beginWriteArray("vaults");
    
    for (int i = 0; i < m_vaults.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("name", m_vaults[i].name());
        settings.setValue("path", m_vaults[i].path());
        settings.setValue("lastAccessed", m_vaults[i].lastAccessed());
    }
    
    settings.endArray();
}

QList<VaultInfo> VaultManager::getRecentVaults() const {
    return m_vaults;
}

void VaultManager::addVault(const VaultInfo &vault) {
    // Check if vault already exists
    for (int i = 0; i < m_vaults.size(); ++i) {
        if (m_vaults[i].path() == vault.path()) {
            // Update existing vault
            m_vaults[i].setName(vault.name());
            m_vaults[i].setLastAccessed(vault.lastAccessed());
            saveVaults();
            return;
        }
    }
    
    // Add new vault
    m_vaults.append(vault);
    saveVaults();
}

void VaultManager::removeVault(const QString &path) {
    for (int i = 0; i < m_vaults.size(); ++i) {
        if (m_vaults[i].path() == path) {
            m_vaults.removeAt(i);
            saveVaults();
            return;
        }
    }
}

void VaultManager::updateLastAccessed(const QString &path) {
    for (int i = 0; i < m_vaults.size(); ++i) {
        if (m_vaults[i].path() == path) {
            m_vaults[i].setLastAccessed(QDateTime::currentDateTime());
            saveVaults();
            return;
        }
    }
}
