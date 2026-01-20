#include "database.h"
#include "../crypto/encryption.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

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

    if (!query.exec("CREATE TABLE IF NOT EXISTS user ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "master_password_hash TEXT NOT NULL, "
                   "salt BLOB NOT NULL)")) {
        qDebug() << "Failed to create user table:" << query.lastError().text();
        return false;
    }

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
