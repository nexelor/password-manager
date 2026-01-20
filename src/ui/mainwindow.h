#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include "../storage/database.h"
#include "../models/passwordentry.h"
#include "../models/settings.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(Database *database, const QByteArray &masterKey, 
               const QString &vaultPath, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onAddPassword();
    void onEditPassword();
    void onDeletePassword();
    void onSearchTextChanged(const QString &text);
    void onTableDoubleClicked(int row, int column);
    void onCopyUsername();
    void onCopyPassword();
    void onOpenSettings();
    void onShowAbout();
    void onShowContextMenu(const QPoint &pos);
    void onAutoLock();
    void onThemeChanged();

private:
    Database *m_database;
    QByteArray m_masterKey;
    QString m_vaultPath;
    
    QTableWidget *m_tableWidget;
    QLineEdit *m_searchBox;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    
    QList<PasswordEntry> m_allEntries;
    
    AppSettings *m_appSettings;
    VaultSettings *m_vaultSettings;
    
    QTimer *m_clipboardTimer;
    QTimer *m_autoLockTimer;
    
    void setupUi();
    void loadPasswords();
    void filterPasswords(const QString &searchText);
    void updateTable(const QList<PasswordEntry> &entries);
    void setupAutoLock();
    void resetAutoLockTimer();
    void startClipboardTimer();
    void applySettings();
};

#endif