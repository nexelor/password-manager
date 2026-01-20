#include "thememanager.h"
#include <QFile>
#include <QApplication>
#include <QPalette>

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager::ThemeManager() : QObject(nullptr) {}

ThemeManager* ThemeManager::instance() {
    if (!s_instance) {
        s_instance = new ThemeManager();
    }
    return s_instance;
}

void ThemeManager::applyTheme(AppSettings::Theme theme) {
    QString styleSheet;
    
    switch (theme) {
        case AppSettings::Dark:
            styleSheet = loadStyleSheet(":/src/styles/theme-dark.qss");
            break;
        case AppSettings::Light:
            styleSheet = loadStyleSheet(":/src/styles/theme-light.qss");
            break;
        case AppSettings::System:
            // Use system palette to determine theme
            QPalette palette = QApplication::palette();
            bool isDark = palette.color(QPalette::Window).lightness() < 128;
            styleSheet = isDark ? loadStyleSheet(":/src/styles/theme-dark.qss") 
                                : loadStyleSheet(":/src/styles/theme-light.qss");
            break;
    }
    
    m_currentStyleSheet = styleSheet;
    qApp->setStyleSheet(styleSheet);
    emit themeChanged();
}

QString ThemeManager::getCurrentStyleSheet() const {
    return m_currentStyleSheet;
}

QString ThemeManager::loadStyleSheet(const QString &fileName) const {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("Could not open stylesheet: %s", qPrintable(fileName));
        return QString();
    }
    return QLatin1String(file.readAll());
}