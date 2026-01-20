#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QString>
#include <QList>
#include "../models/passwordentry.h"

class Database {
public:
    Database();
    ~Database();

    bool open(const QString &path);
    void close();
    bool isOpen() const;

    bool createUser(const QString &masterPasswordHash, const QByteArray &salt);
    bool verifyUser(const QString &masterPasswordHash);
    QByteArray getUserSalt();

    bool addEntry(const PasswordEntry &entry, const QByteArray &masterKey);
    bool updateEntry(const PasswordEntry &entry, const QByteArray &masterKey);
    bool deleteEntry(int id);
    QList<PasswordEntry> getAllEntries(const QByteArray &masterKey);
    PasswordEntry getEntry(int id, const QByteArray &masterKey);

private:
    QSqlDatabase m_db;
    bool createTables();
};

#endif
