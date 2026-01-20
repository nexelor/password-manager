#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include "../storage/database.h"
#include "../models/passwordentry.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(Database *database, const QByteArray &masterKey, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onAddPassword();
    void onEditPassword();
    void onDeletePassword();
    void onSearchTextChanged(const QString &text);
    void onTableDoubleClicked(int row, int column);
    void onCopyUsername();
    void onCopyPassword();

private:
    Database *m_database;
    QByteArray m_masterKey;
    
    QTableWidget *m_tableWidget;
    QLineEdit *m_searchBox;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    
    QList<PasswordEntry> m_allEntries;
    
    void setupUi();
    void loadStyleSheet();
    void loadPasswords();
    void filterPasswords(const QString &searchText);
    void updateTable(const QList<PasswordEntry> &entries);
};

#endif