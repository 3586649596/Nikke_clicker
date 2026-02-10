#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QColor>
#include <QString>

class QWidget;

struct ThemeTokens
{
    QColor background;
    QColor surface;
    QColor surfaceElevated;
    QColor border;
    QColor textPrimary;
    QColor textSecondary;
    QColor accent;
    QColor accentAlt;
    QColor success;
    QColor danger;

    int radiusLarge;
    int radiusMedium;
    int radiusSmall;
};

class ThemeManager
{
public:
    static bool isSystemDark();
    static ThemeTokens currentTokens();
    static void apply(QWidget *root);

    static QColor overlayBackgroundColor(bool running, bool darkTheme);
    static QColor overlayTextColor(bool running, bool darkTheme);
    static QColor overlayBorderColor(bool running, bool darkTheme);

private:
    static QString buildStyleSheet(const ThemeTokens& tokens);
};

#endif // THEME_MANAGER_H
