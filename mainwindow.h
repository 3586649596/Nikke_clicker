#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QCheckBox>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

#include "keyboardhook.h"
#include "mouseclicker.h"
#include "overlaywidget.h"
#include "settingsmanager.h"
#include "updatechecker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class UiPreset {
        Custom = -1,
        Stable = 0,
        Balanced = 1,
        Aggressive = 2
    };
    Q_ENUM(UiPreset)

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void checkForUpdates();

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onStartStopClicked();
    void onClickIntervalChanged(int value);
    void onPressDownDurationChanged(int value);
    void onRandomDelayChanged(int value);
    void onClickModeChanged();
    void onChangeHotkeyClicked();
    void onHotkeyPressed(int vkCode, const QString& name);
    void onClickerStatusChanged(bool running);
    void onHookInstalled();
    void onHookFailed(const QString& error);
    void onShowOverlayChanged(bool checked);

    void onCheckUpdateClicked();
    void onUpdateAvailable(const QString& version, const QString& url, const QString& notes);
    void onNoUpdateAvailable();
    void onCheckUpdateFailed(const QString& error);
    void onDownloadProgress(int percent);
    void onDownloadFinished(const QString& filePath);
    void onDownloadFailed(const QString& error);

private:
    void setupUi();
    void buildHeroSection(QVBoxLayout *mainLayout);
    void buildControlSection(QVBoxLayout *mainLayout);
    void buildUpdateSection(QVBoxLayout *mainLayout);

    void connectSignals();
    void initKeyboardHook();
    void updateStatusDisplay();
    void updateDynamicStateStyles();
    void playStatusAnimation();
    void refreshRuntimeSummary();

    void loadSettings();
    void saveSettings();

    void applyPreset(UiPreset preset, bool persistPreset);
    void syncPresetSelectionWithValues();
    UiPreset currentPresetFromValues() const;
    QString presetToLabel(UiPreset preset) const;

    static void repolish(QWidget *widget);

    Ui::MainWindow *ui;

    MouseClicker *m_clicker;
    KeyboardHook *m_keyboardHook;
    OverlayWidget *m_overlay;
    SettingsManager *m_settings;
    int m_toggleHotkey;
    bool m_isCapturingHotkey;
    bool m_hookReady;

    QLabel *m_statusLabel;
    QLabel *m_statusBadge;
    QLabel *m_runtimeSummaryLabel;

    QLineEdit *m_hotkeyEdit;
    QPushButton *m_changeHotkeyBtn;

    QButtonGroup *m_presetButtonGroup;
    QPushButton *m_presetStableBtn;
    QPushButton *m_presetBalancedBtn;
    QPushButton *m_presetAggressiveBtn;

    QSlider *m_clickIntervalSlider;
    QSpinBox *m_clickIntervalSpinBox;
    QSlider *m_pressDownSlider;
    QSpinBox *m_pressDownSpinBox;
    QSlider *m_randomDelaySlider;
    QSpinBox *m_randomDelaySpinBox;

    QRadioButton *m_sendInputRadio;
    QRadioButton *m_postMessageRadio;

    QPushButton *m_startStopBtn;
    QCheckBox *m_showOverlayCheckBox;

    UpdateChecker *m_updateChecker;
    QPushButton *m_checkUpdateBtn;
    QProgressBar *m_updateProgressBar;
    QLabel *m_updateStatusLabel;
    QString m_pendingUpdatePath;

    QGraphicsOpacityEffect *m_statusFadeEffect;
    QPropertyAnimation *m_statusFadeAnimation;
    bool m_themeApplyInProgress;
};

#endif // MAINWINDOW_H
