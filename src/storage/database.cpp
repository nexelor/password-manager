#include "database.h"
#include "../crypto/encryption.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QVariant>

Database::Database() {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

Database::~Database() {
    close();
}

bool Database::open(const QString &path) {
    QString dbPath = path;
    if (dbPath.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        dbPath = dataDir + "/passwords.db";
    }

    m_db.setDatabaseName(dbPath);
    
    if (!m_db.open()) {
        qDebug() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    return createTables();
}

void Database::close() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool Database::isOpen() const {
    return m_db.isOpen();
}

bool Database::createTables() {
    QSqlQuery query(m_db);

    // User table
    if (!query.exec("CREATE TABLE IF NOT EXISTS user ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "master_password_hash TEXT NOT NULL, "
                   "salt BLOB NOT NULL)")) {
        qDebug() << "Failed to create user table:" << query.lastError().text();
        return false;
    }

    // Passwords table
    if (!query.exec("CREATE TABLE IF NOT EXISTS passwords ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "title_encrypted BLOB NOT NULL, "
                   "username_encrypted BLOB NOT NULL, "
                   "password_encrypted BLOB NOT NULL, "
                   "url_encrypted BLOB, "
                   "notes_encrypted BLOB, "
                   "created_at DATETIME NOT NULL, "
                   "modified_at DATETIME NOT NULL)")) {
        qDebug() << "Failed to create passwords table:" << query.lastError().text();
        return false;
    }

    // Settings table - stores vault-specific settings
    if (!query.exec("CREATE TABLE IF NOT EXISTS vault_settings ("
                   "key TEXT PRIMARY KEY, "
                   "value TEXT NOT NULL, "
                   "type TEXT NOT NULL)")) {
        qDebug() << "Failed to create vault_settings table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Database tables created/verified successfully";
    return true;
}

bool Database::createUser(const QString &masterPasswordHash, 
                         const QByteArray &salt) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO user (master_password_hash, salt) VALUES (?, ?)");
    query.addBindValue(masterPasswordHash);
    query.addBindValue(salt);
    
    return query.exec();
}

bool Database::verifyUser(const QString &masterPasswordHash) {
    QSqlQuery query(m_db);
    query.prepare("SELECT master_password_hash FROM user WHERE id = 1");
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    return query.value(0).toString() == masterPasswordHash;
}

QByteArray Database::getUserSalt() {
    QSqlQuery query(m_db);
    query.prepare("SELECT salt FROM user WHERE id = 1");
    
    if (!query.exec() || !query.next()) {
        return QByteArray();
    }
    
    return query.value(0).toByteArray();
}

bool Database::addEntry(const PasswordEntry &entry, const QByteArray &masterKey) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO passwords (title_encrypted, username_encrypted, "
                 "password_encrypted, url_encrypted, notes_encrypted, "
                 "created_at, modified_at) VALUES (?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(Encryption::encrypt(entry.title().toUtf8(), masterKey));
    query.addBindValue(Encryption::encrypt(entry.username().toUtf8(), masterKey));
    query.addBindValue(Encryption::encrypt(entry.password().toUtf8(), masterKey));
    query.addBindValue(Encryption::encrypt(entry.url().toUtf8(), masterKey));
    query.addBindValue(Encryption::encrypt(entry.notes().toUtf8(), masterKey));
    query.addBindValue(entry.created());
    query.addBindValue(entry.modified());
    
    return query.exec();
}

bool Database::updateEntry(const PasswordEntry &entry, const QByteArray &masterKey) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE passwords SET title_encrypted = ?, username_encrypted = ?, "
                 "password_encrypted = ?, url_encrypted = ?, notes_encrypted = ?, "
                 "modified_at = ? WHERE id = ?");
    
    query.addBindValue(Encryption::encrypt(entry.title().toUtf8(), masterKey));
    query.addBindValue(Encryption::encrypt(entry.username().toUtf8(), masterKey));
    query.addBindValue(Encryption::encrypt(entry.password().toUtf8(), masterKey));
    query.addBindValue(Encryption::encrypt(entry.url().toUtf8(), masterKey));
    query.addBindValue(Encryption::encrypt(entry.notes().toUtf8(), masterKey));
    query.addBindValue(QDateTime::currentDateTime());
    query.addBindValue(entry.id());
    
    return query.exec();
}

bool Database::deleteEntry(int id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM passwords WHERE id = ?");
    query.addBindValue(id);
    
    return query.exec();
}

QList<PasswordEntry> Database::getAllEntries(const QByteArray &masterKey) {
    QList<PasswordEntry> entries;
    QSqlQuery query(m_db);
    
    if (!query.exec("SELECT id, title_encrypted, username_encrypted, "
                   "password_encrypted, url_encrypted, notes_encrypted, "
                   "created_at, modified_at FROM passwords")) {
        return entries;
    }
    
    while (query.next()) {
        PasswordEntry entry;
        entry.setId(query.value(0).toInt());
        entry.setTitle(QString::fromUtf8(
            Encryption::decrypt(query.value(1).toByteArray(), masterKey)));
        entry.setUsername(QString::fromUtf8(
            Encryption::decrypt(query.value(2).toByteArray(), masterKey)));
        entry.setPassword(QString::fromUtf8(
            Encryption::decrypt(query.value(3).toByteArray(), masterKey)));
        entry.setUrl(QString::fromUtf8(
            Encryption::decrypt(query.value(4).toByteArray(), masterKey)));
        entry.setNotes(QString::fromUtf8(
            Encryption::decrypt(query.value(5).toByteArray(), masterKey)));
        
        entries.append(entry);
    }
    
    return entries;
}

PasswordEntry Database::getEntry(int id, const QByteArray &masterKey) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id, title_encrypted, username_encrypted, "
                 "password_encrypted, url_encrypted, notes_encrypted, "
                 "created_at, modified_at FROM passwords WHERE id = ?");
    query.addBindValue(id);
    
    PasswordEntry entry;
    if (!query.exec() || !query.next()) {
        return entry;
    }
    
    entry.setId(query.value(0).toInt());
    entry.setTitle(QString::fromUtf8(
        Encryption::decrypt(query.value(1).toByteArray(), masterKey)));
    entry.setUsername(QString::fromUtf8(
        Encryption::decrypt(query.value(2).toByteArray(), masterKey)));
    entry.setPassword(QString::fromUtf8(
        Encryption::decrypt(query.value(3).toByteArray(), masterKey)));
    entry.setUrl(QString::fromUtf8(
        Encryption::decrypt(query.value(4).toByteArray(), masterKey)));
    entry.setNotes(QString::fromUtf8(
        Encryption::decrypt(query.value(5).toByteArray(), masterKey)));
    
    return entry;
}

// ========== Vault Settings Methods ==========

bool Database::setSetting(const QString &key, const QVariant &value) {
    if (!m_db.isOpen()) {
        qWarning() << "Database not open, cannot save setting:" << key;
        return false;
    }

    QSqlQuery query(m_db);
    
    // Store the type information so we can restore it correctly
    QString typeStr = QString(value.typeName());
    QString valueStr = value.toString();
    
    // Use INSERT OR REPLACE (SQLite specific) to handle both insert and update
    query.prepare("INSERT OR REPLACE INTO vault_settings (key, value, type) VALUES (?, ?, ?)");
    query.addBindValue(key);
    query.addBindValue(valueStr);
    query.addBindValue(typeStr);
    
    bool success = query.exec();
    if (!success) {
        qWarning() << "Failed to save setting" << key << ":" << query.lastError().text();
    } else {
        qDebug() << "Saved vault setting:" << key << "=" << valueStr << "(" << typeStr << ")";
    }
    
    return success;
}

QVariant Database::getSetting(const QString &key, const QVariant &defaultValue) const {
    if (!m_db.isOpen()) {
        qWarning() << "Database not open, returning default for:" << key;
        return defaultValue;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT value, type FROM vault_settings WHERE key = ?");
    query.addBindValue(key);
    
    if (!query.exec() || !query.next()) {
        return defaultValue;
    }
    
    QString valueStr = query.value(0).toString();
    QString typeStr = query.value(1).toString();
    
    // Convert string back to appropriate type
    QVariant result;
    
    if (typeStr == "bool") {
        result = QVariant(valueStr == "true" || valueStr == "1");
    } else if (typeStr == "int") {
        result = QVariant(valueStr.toInt());
    } else if (typeStr == "double") {
        result = QVariant(valueStr.toDouble());
    } else if (typeStr == "QString") {
        result = QVariant(valueStr);
    } else {
        // Default to string
        result = QVariant(valueStr);
    }
    
    qDebug() << "Loaded vault setting:" << key << "=" << result << "(" << typeStr << ")";
    return result;
}

bool Database::hasSetting(const QString &key) const {
    if (!m_db.isOpen()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM vault_settings WHERE key = ?");
    query.addBindValue(key);
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    return query.value(0).toInt() > 0;
}

bool Database::removeSetting(const QString &key) {
    if (!m_db.isOpen()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM vault_settings WHERE key = ?");
    query.addBindValue(key);
    
    bool success = query.exec();
    if (success) {
        qDebug() << "Removed vault setting:" << key;
    }
    
    return success;
}

QStringList Database::getAllSettingKeys() const {
    QStringList keys;
    
    if (!m_db.isOpen()) {
        return keys;
    }

    QSqlQuery query(m_db);
    if (!query.exec("SELECT key FROM vault_settings")) {
        return keys;
    }
    
    while (query.next()) {
        keys.append(query.value(0).toString());
    }
    
    return keys;
}