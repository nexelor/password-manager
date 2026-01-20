#ifndef VAULTINFO_H
#define VAULTINFO_H

#include <QString>
#include <QDateTime>

class VaultInfo {
public:
    VaultInfo();
    VaultInfo(const QString &name, const QString &path, const QDateTime &lastAccessed);

    QString name() const { return m_name; }
    QString path() const { return m_path; }
    QDateTime lastAccessed() const { return m_lastAccessed; }

    void setName(const QString &name) { m_name = name; }
    void setPath(const QString &path) { m_path = path; }
    void setLastAccessed(const QDateTime &lastAccessed) { m_lastAccessed = lastAccessed; }

private:
    QString m_name;
    QString m_path;
    QDateTime m_lastAccessed;
};

#endif
