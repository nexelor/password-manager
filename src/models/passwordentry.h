#ifndef PASSWORDENTRY_H
#define PASSWORDENTRY_H

#include <QString>
#include <QDateTime>

class PasswordEntry {
public:
    PasswordEntry();
    PasswordEntry(int id, const QString &title, const QString &username,
                  const QString &password, const QString &url,
                  const QString &notes, const QDateTime &created,
                  const QDateTime &modified);

    int id() const { return m_id; }
    QString title() const { return m_title; }
    QString username() const { return m_username; }
    QString password() const { return m_password; }
    QString url() const { return m_url; }
    QString notes() const { return m_notes; }
    QDateTime created() const { return m_created; }
    QDateTime modified() const { return m_modified; }

    void setId(int id) { m_id = id; }
    void setTitle(const QString &title) { m_title = title; }
    void setUsername(const QString &username) { m_username = username; }
    void setPassword(const QString &password) { m_password = password; }
    void setUrl(const QString &url) { m_url = url; }
    void setNotes(const QString &notes) { m_notes = notes; }
    void setModified(const QDateTime &modified) { m_modified = modified; }

private:
    int m_id;
    QString m_title;
    QString m_username;
    QString m_password;
    QString m_url;
    QString m_notes;
    QDateTime m_created;
    QDateTime m_modified;
};

#endif
