#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <QString>
#include <QByteArray>

class Encryption {
public:
    static bool initialize();
    static QByteArray deriveMasterKey(const QString &masterPassword, 
                                      const QByteArray &salt);
    static QByteArray generateSalt();
    static QByteArray encrypt(const QByteArray &data, const QByteArray &key);
    static QByteArray decrypt(const QByteArray &encryptedData, 
                             const QByteArray &key);
    static QString hashPassword(const QString &password);
    static bool verifyPassword(const QString &password, const QString &hash);
};

#endif
