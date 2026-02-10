#include "theme_manager.h"

#include <QApplication>
#include <QPalette>
#include <QWidget>

bool ThemeManager::isSystemDark()
{
    if (!qApp) {
        return true;
    }

    const QColor windowColor = qApp->palette().color(QPalette::Window);
    return windowColor.lightness() < 128;
}

ThemeTokens ThemeManager::currentTokens()
{
    ThemeTokens t;

    if (isSystemDark()) {
        t.background = QColor("#0E1116");
        t.surface = QColor("#161B22");
        t.surfaceElevated = QColor("#1C232D");
        t.border = QColor("#303842");
        t.textPrimary = QColor("#E6EDF3");
        t.textSecondary = QColor("#9BA7B4");
        t.accent = QColor("#FF6A3D");
        t.accentAlt = QColor("#3CCBDA");
        t.success = QColor("#37C871");
        t.danger = QColor("#E5534B");
    } else {
        t.background = QColor("#F3F6FA");
        t.surface = QColor("#FFFFFF");
        t.surfaceElevated = QColor("#F8FAFC");
        t.border = QColor("#D0D7DE");
        t.textPrimary = QColor("#1F2328");
        t.textSecondary = QColor("#5F6B76");
        t.accent = QColor("#D84B2A");
        t.accentAlt = QColor("#007EA7");
        t.success = QColor("#1F9D55");
        t.danger = QColor("#C0392B");
    }

    t.radiusLarge = 16;
    t.radiusMedium = 12;
    t.radiusSmall = 8;
    return t;
}

void ThemeManager::apply(QWidget *root)
{
    if (!root) {
        return;
    }

    root->setStyleSheet(buildStyleSheet(currentTokens()));
}

QColor ThemeManager::overlayBackgroundColor(bool running, bool darkTheme)
{
    if (running) {
        return darkTheme ? QColor(12, 46, 30, 215) : QColor(184, 246, 214, 220);
    }

    return darkTheme ? QColor(30, 35, 42, 205) : QColor(233, 237, 243, 220);
}

QColor ThemeManager::overlayTextColor(bool running, bool darkTheme)
{
    if (running) {
        return darkTheme ? QColor("#7FFFB1") : QColor("#0F5A34");
    }

    return darkTheme ? QColor("#E6EDF3") : QColor("#243241");
}

QColor ThemeManager::overlayBorderColor(bool running, bool darkTheme)
{
    if (running) {
        return darkTheme ? QColor("#5EE9A5") : QColor("#12824A");
    }

    return darkTheme ? QColor("#516275") : QColor("#9DA9B5");
}

QString ThemeManager::buildStyleSheet(const ThemeTokens& t)
{
    const QString accentSoft = t.accentAlt.name();
    const QString accent = t.accent.name();
    const QString success = t.success.name();
    const QString danger = t.danger.name();

    return QString(
        "QMainWindow, QWidget#centralSurface {"
        "  background: %1;"
        "  color: %2;"
        "  font-family: 'Bahnschrift SemiBold', 'Microsoft YaHei UI';"
        "  font-size: 13px;"
        "}"

        "QFrame#heroCard, QFrame#panelCard {"
        "  background: %3;"
        "  border: 1px solid %4;"
        "  border-radius: %5px;"
        "}"

        "QLabel#heroTitle {"
        "  color: %2;"
        "  font-size: 22px;"
        "  font-weight: 700;"
        "}"

        "QLabel#heroSubtitle {"
        "  color: %6;"
        "  font-size: 12px;"
        "}"

        "QLabel#statusBadge {"
        "  background: %7;"
        "  color: %2;"
        "  border-radius: %8px;"
        "  border: 1px solid %4;"
        "  font-size: 13px;"
        "  font-weight: 700;"
        "  padding: 5px 12px;"
        "}"

        "QLabel#statusBadge[running=\"true\"] {"
        "  background: %9;"
        "  color: #FFFFFF;"
        "  border: 1px solid %9;"
        "}"

        "QPushButton#primaryToggleButton {"
        "  border: 0;"
        "  border-radius: %8px;"
        "  background: %10;"
        "  color: #FFFFFF;"
        "  font-size: 16px;"
        "  font-weight: 700;"
        "  padding: 10px 18px;"
        "}"

        "QPushButton#primaryToggleButton:hover {"
        "  background: %11;"
        "}"

        "QPushButton#primaryToggleButton[running=\"true\"] {"
        "  background: %12;"
        "}"

        "QPushButton#primaryToggleButton[running=\"true\"]:hover {"
        "  background: %13;"
        "}"

        "QPushButton#secondaryButton, QPushButton#changeHotkeyButton, QPushButton#presetButton {"
        "  background: %7;"
        "  border: 1px solid %4;"
        "  border-radius: %14px;"
        "  color: %2;"
        "  padding: 6px 10px;"
        "}"

        "QPushButton#secondaryButton:hover, QPushButton#changeHotkeyButton:hover, QPushButton#presetButton:hover {"
        "  border: 1px solid %15;"
        "}"

        "QPushButton#presetButton:checked {"
        "  background: %15;"
        "  color: #FFFFFF;"
        "  border: 1px solid %15;"
        "}"

        "QLineEdit#hotkeyValue, QSpinBox {"
        "  background: %16;"
        "  color: %2;"
        "  border: 1px solid %4;"
        "  border-radius: %14px;"
        "  padding: 4px 8px;"
        "}"

        "QLineEdit#hotkeyValue[capturing=\"true\"] {"
        "  border: 1px solid %17;"
        "  background: %18;"
        "}"

        "QLabel#sectionTitle {"
        "  color: %2;"
        "  font-size: 15px;"
        "  font-weight: 700;"
        "}"

        "QLabel#sectionHint, QLabel#runtimeSummary {"
        "  color: %6;"
        "  font-size: 12px;"
        "}"

        "QRadioButton, QCheckBox, QLabel#bodyText {"
        "  color: %2;"
        "}"

        "QProgressBar {"
        "  background: %7;"
        "  border: 1px solid %4;"
        "  border-radius: %14px;"
        "  text-align: center;"
        "  color: %2;"
        "}"

        "QProgressBar::chunk {"
        "  background: %15;"
        "  border-radius: %14px;"
        "}"

        "QSlider::groove:horizontal {"
        "  border: 1px solid %4;"
        "  background: %7;"
        "  height: 6px;"
        "  border-radius: 3px;"
        "}"

        "QSlider::sub-page:horizontal {"
        "  background: %15;"
        "  border-radius: 3px;"
        "}"

        "QSlider::handle:horizontal {"
        "  background: %17;"
        "  width: 14px;"
        "  margin: -5px 0;"
        "  border-radius: 7px;"
        "}"
    )
        .arg(t.background.name())
        .arg(t.textPrimary.name())
        .arg(t.surface.name())
        .arg(t.border.name())
        .arg(t.radiusLarge)
        .arg(t.textSecondary.name())
        .arg(t.surfaceElevated.name())
        .arg(t.radiusMedium)
        .arg(success)
        .arg(accent)
        .arg(QColor(accent).lighter(110).name())
        .arg(danger)
        .arg(QColor(danger).lighter(110).name())
        .arg(t.radiusSmall)
        .arg(accentSoft)
        .arg(t.surfaceElevated.name())
        .arg(accent.name())
        .arg(QColor(accent).lighter(170).name());
}
