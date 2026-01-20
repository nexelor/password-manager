#include "mainwindow.h"
#include "passworddialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QFile>

MainWindow::MainWindow(Database *database, const QByteArray &masterKey, QWidget *parent)
    : QMainWindow(parent), m_database(database), m_masterKey(masterKey) {
    setAttribute(Qt::WA_DeleteOnClose);
    setupUi();
    loadStyleSheet();
    loadPasswords();
}

MainWindow::~MainWindow() {
    if (m_database) {
        delete m_database;
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Clear sensitive data from memory
    m_masterKey.fill(0);
    m_allEntries.clear();
    QMainWindow::closeEvent(event);
}

void MainWindow::loadStyleSheet() {
    QFile styleFile(":/src/styles/mainwindow.qss");
    if (!styleFile.open(QFile::ReadOnly)) {
        qWarning("Could not open main window stylesheet");
        return;
    }
    QString styleSheet = QLatin1String(styleFile.readAll());
    setStyleSheet(styleSheet);
}

void MainWindow::setupUi() {
    setWindowTitle("Password Manager");
    resize(900, 600);
    setMinimumSize(600, 400);
    
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Search bar
    QHBoxLayout *searchLayout = new QHBoxLayout();
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Search passwords...");
    searchLayout->addWidget(m_searchBox);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_addButton = new QPushButton("Add Password", this);
    m_addButton->setObjectName("addButton");
    m_editButton = new QPushButton("Edit", this);
    m_deleteButton = new QPushButton("Delete", this);
    
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addStretch();
    
    // Table
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(4);
    m_tableWidget->setHorizontalHeaderLabels({"Title", "Username", "URL", "Modified"});
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Make table columns resize properly
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    
    mainLayout->addLayout(searchLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(m_tableWidget);
    
    // Menu bar
    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *fileMenu = menuBar->addMenu("File");
    QAction *exitAction = fileMenu->addAction("Exit");
    
    setMenuBar(menuBar);
    
    // Connections
    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::onAddPassword);
    connect(m_editButton, &QPushButton::clicked, this, &MainWindow::onEditPassword);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeletePassword);
    connect(m_searchBox, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_tableWidget, &QTableWidget::cellDoubleClicked, this, &MainWindow::onTableDoubleClicked);
    connect(m_tableWidget, &QTableWidget::itemSelectionChanged, [this]() {
        bool hasSelection = !m_tableWidget->selectedItems().isEmpty();
        m_editButton->setEnabled(hasSelection);
        m_deleteButton->setEnabled(hasSelection);
    });
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
}

void MainWindow::loadPasswords() {
    m_allEntries = m_database->getAllEntries(m_masterKey);
    updateTable(m_allEntries);
}

void MainWindow::updateTable(const QList<PasswordEntry> &entries) {
    m_tableWidget->setRowCount(entries.size());
    
    for (int i = 0; i < entries.size(); ++i) {
        const PasswordEntry &entry = entries[i];
        
        m_tableWidget->setItem(i, 0, new QTableWidgetItem(entry.title()));
        m_tableWidget->setItem(i, 1, new QTableWidgetItem(entry.username()));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(entry.url()));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(
            entry.modified().toString("yyyy-MM-dd HH:mm")));
        
        m_tableWidget->item(i, 0)->setData(Qt::UserRole, entry.id());
    }
}

void MainWindow::onAddPassword() {
    PasswordDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        PasswordEntry entry = dialog.getPasswordEntry();
        if (m_database->addEntry(entry, m_masterKey)) {
            loadPasswords();
        } else {
            QMessageBox::critical(this, "Error", "Failed to add password.");
        }
    }
}

void MainWindow::onEditPassword() {
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) return;
    
    int entryId = m_tableWidget->item(currentRow, 0)->data(Qt::UserRole).toInt();
    PasswordEntry entry = m_database->getEntry(entryId, m_masterKey);
    
    PasswordDialog dialog(entry, this);
    if (dialog.exec() == QDialog::Accepted) {
        PasswordEntry updatedEntry = dialog.getPasswordEntry();
        updatedEntry.setId(entryId);
        
        if (m_database->updateEntry(updatedEntry, m_masterKey)) {
            loadPasswords();
        } else {
            QMessageBox::critical(this, "Error", "Failed to update password.");
        }
    }
}

void MainWindow::onDeletePassword() {
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) return;
    
    QString title = m_tableWidget->item(currentRow, 0)->text();
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete",
        QString("Are you sure you want to delete '%1'?").arg(title),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        int entryId = m_tableWidget->item(currentRow, 0)->data(Qt::UserRole).toInt();
        if (m_database->deleteEntry(entryId)) {
            loadPasswords();
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete password.");
        }
    }
}

void MainWindow::onSearchTextChanged(const QString &text) {
    filterPasswords(text);
}

void MainWindow::filterPasswords(const QString &searchText) {
    if (searchText.isEmpty()) {
        updateTable(m_allEntries);
        return;
    }
    
    QList<PasswordEntry> filtered;
    for (const PasswordEntry &entry : m_allEntries) {
        if (entry.title().contains(searchText, Qt::CaseInsensitive) ||
            entry.username().contains(searchText, Qt::CaseInsensitive) ||
            entry.url().contains(searchText, Qt::CaseInsensitive)) {
            filtered.append(entry);
        }
    }
    
    updateTable(filtered);
}

void MainWindow::onTableDoubleClicked(int row, int column) {
    if (column == 1) {
        onCopyUsername();
    } else {
        onEditPassword();
    }
}

void MainWindow::onCopyUsername() {
    int currentRow = m_tableWidget->currentRow();
    if (currentRow >= 0) {
        QString username = m_tableWidget->item(currentRow, 1)->text();
        QApplication::clipboard()->setText(username);
    }
}

void MainWindow::onCopyPassword() {
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) return;
    
    int entryId = m_tableWidget->item(currentRow, 0)->data(Qt::UserRole).toInt();
    PasswordEntry entry = m_database->getEntry(entryId, m_masterKey);
    QApplication::clipboard()->setText(entry.password());
}