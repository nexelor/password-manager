#include "vaultinfo.h"

VaultInfo::VaultInfo()
    : m_lastAccessed(QDateTime::currentDateTime()) {}

VaultInfo::VaultInfo(const QString &name, const QString &path, const QDateTime &lastAccessed)
    : m_name(name), m_path(path), m_lastAccessed(lastAccessed) {}
