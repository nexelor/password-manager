#include "passwordentry.h"

PasswordEntry::PasswordEntry()
    : m_id(-1), m_created(QDateTime::currentDateTime()),
      m_modified(QDateTime::currentDateTime()) {}

PasswordEntry::PasswordEntry(int id, const QString &title,
                             const QString &username, const QString &password,
                             const QString &url, const QString &notes,
                             const QDateTime &created, const QDateTime &modified)
    : m_id(id), m_title(title), m_username(username), m_password(password),
      m_url(url), m_notes(notes), m_created(created), m_modified(modified) {}
