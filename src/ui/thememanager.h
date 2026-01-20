#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include "../models/settings.h"

class ThemeManager : public QObject {
    Q_OBJECT

public:
    static ThemeManager* instance();
    
    void applyTheme(AppSettings::Theme theme);
    QString getCurrentStyleSheet() const;
    
signals:
    void themeChanged();

private:
    ThemeManager();
    QString loadStyleSheet(const QString &fileName) const;
    QString m_currentStyleSheet;
    
    static ThemeManager *s_instance;
};

#endif