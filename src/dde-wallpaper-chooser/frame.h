// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FRAME_H
#define FRAME_H

#include "view/backgroundmanager.h"
#include <DBlurEffectWidget>
#include <DRegionMonitor>
#include <com_deepin_sessionmanager.h>

DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DButtonBox;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QCheckBox;
class QHBoxLayout;
class QAbstractButton;
class QLabel;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE
class WaitItem;
class WallpaperList;
class WallpaperListView;
class ComDeepinDaemonAppearanceInterface;
class ComDeepinScreenSaverInterface;
class DeepinWM;
class BackgroundHelper;
class CheckBox;
class Frame : public DBlurEffectWidget
{
    Q_OBJECT

public:
    enum Mode {
        WallpaperMode,
        ScreenSaverMode
    };

    Frame(QString screenName, Mode mode = WallpaperMode, QWidget *parent = nullptr);
    ~Frame() override;

    void show();
    void hide();
    QPair<QString,QString> desktopBackground() const;
signals:
    void backgroundChanged();
    void done();

public slots:
    void handleNeedCloseButton(QString path, QPoint pos);
    void onRest();
protected:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    bool event(QEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void setMode(Mode mode);
#if !defined(DISABLE_SCREENSAVER) || !defined(DISABLE_WALLPAPER_CAROUSEL)
    void adjustModeSwitcherPoint();
    DButtonBox *m_switchModeControl;
#endif
    com::deepin::SessionManager *m_sessionManagerInter;

#ifndef DISABLE_SCREENSAVER
    void setMode(QAbstractButton *toggledBtn, bool on);
    void reLayoutTools();

    Mode m_mode;
    QHBoxLayout *m_toolLayout;
    QLabel *m_waitControlLabel;
    DButtonBox *m_waitControl = nullptr;
    QCheckBox *m_lockScreenBox;
#else
    const Mode m_mode = WallpaperMode;
#endif
    WallpaperList *m_wallpaperList = nullptr;
    QString m_actualEffectivedWallpaper;
    QString m_currentSelectedWallpaper;
    QStringList m_needDeleteList;
    DIconButton *m_closeButton = nullptr;
#ifndef DISABLE_WALLPAPER_CAROUSEL
    QHBoxLayout *m_wallpaperCarouselLayout;
    CheckBox *m_wallpaperCarouselCheckBox;
    DButtonBox *m_wallpaperCarouselControl;
#endif

    WMInter *m_dbusWmInter = nullptr;
    ComDeepinDaemonAppearanceInterface * m_dbusAppearance = nullptr;
#ifndef DISABLE_SCREENSAVER
    ComDeepinScreenSaverInterface *m_dbusScreenSaver = nullptr;
#endif
    DeepinWM * m_dbusDeepinWM = nullptr;
    DRegionMonitor * m_mouseArea = nullptr;

    QString m_formerWallpaper;
    QMap<QString, bool> m_deletableInfo;
    BackgroundManager *m_backgroundManager = nullptr;

    void loading();
    void initUI();
    void initSize();
    void refreshList();
    void onItemPressed(const QString &data);
    void onItemButtonClicked(const QString &buttonID);
    QStringList processListReply(const QString &reply);

    QString getWallpaperSlideShow();
    void setWallpaperSlideShow(QString slideShow);
    void applyToDesktop();
    void applyToGreeter();

    QString m_screenName;

    WaitItem* m_itemwait = nullptr;
    QTimer m_loadTimer;
};

#endif // FRAME_H
