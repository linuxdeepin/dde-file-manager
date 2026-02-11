// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WALLPAPERSETTINGS_P_H
#define WALLPAPERSETTINGS_P_H

#include "wallpapersettings.h"
#include "wallpaperlist.h"
#include "loadinglabel.h"
#include "wallaperpreview.h"
#include "dbus/screensaver_interface.h"
#include "dbus/dbussessionmanager.h"

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <com_deepin_wm.h>
#endif
#ifdef COMPILE_ON_V2X
#    include "dbus/appearance_interface.h"
#    define APPEARANCE_NAME org::deepin::dde::Appearance1
#else
#    include <com_deepin_daemon_appearance.h>
#    define APPEARANCE_NAME com::deepin::daemon::Appearance
#endif

#include <DIconButton>
#include <DRegionMonitor>
#include <DButtonBox>

#include <QObject>
#include <QHBoxLayout>
#include <QCheckBox>

class QLabel;
DWIDGET_USE_NAMESPACE

namespace ddplugin_wallpapersetting {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
using WMInter = com::deepin::wm;
#endif
using AppearanceIfs = APPEARANCE_NAME;
using SessionIfs = DBusSessionManager;
using ScreenSaverIfs = com::deepin::ScreenSaver;

class WallpaperSettingsPrivate : public QObject
{
    Q_OBJECT
public:
    explicit WallpaperSettingsPrivate(WallpaperSettings *parent);
    void propertyForWayland();
    void initUI();
    void relaylout();
    void adjustModeSwitcher();
    static QString timeFormat(int second);
public slots:
    void carouselTurn(bool checked);
    void switchCarousel(QAbstractButton *toggledBtn, bool state);
    void switchWaitTime(QAbstractButton *toggledBtn, bool state);
    void setMode(QAbstractButton *toggledBtn, bool on);
    void onListBackgroundReply(QDBusPendingCallWatcher *watch);
    void onItemPressed(const QString &itemData);
    void onItemButtonClicked(WallpaperItem *item, const QString &id);
    void onItemTab(WallpaperItem *item);
    void onItemBacktab(WallpaperItem *item);
    void handleNeedCloseButton(const QString &itemData, const QPoint &pos);
    void onCloseButtonClicked();
    void onMousePressed(const QPoint &pos, int button);
    void onScreenChanged();

protected:
    void initCloseButton();
    void initCarousel();
    void initScreenSaver();
    void initPreivew();
    QList<QPair<QString, bool>> processListReply(const QString &reply);
    bool eventFilter(QObject *watched, QEvent *event);

public:
    static const int kHeaderSwitcherHeight;
    static const int kFrameHeight;
    static const int kListHeight;

public:
    WallpaperSettings::Mode mode;
    QString screenName;
    QString actualEffectivedWallpaper;
    QString currentSelectedWallpaper;
    WallpaperList *wallpaperList = nullptr;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    WMInter *wmInter = nullptr;
#endif
    AppearanceIfs *appearanceIfs = nullptr;
    SessionIfs *sessionIfs = nullptr;
    ScreenSaverIfs *screenSaverIfs = nullptr;
    DRegionMonitor *regionMonitor = nullptr;

public:
    DButtonBox *switchModeControl = nullptr;
    LoadingLabel *loadingLabel = nullptr;
    QTimer reloadTimer;

public:
    DIconButton *closeButton = nullptr;
    QHBoxLayout *carouselLayout = nullptr;
    QCheckBox *carouselCheckBox = nullptr;
    DButtonBox *carouselControl = nullptr;
    WallaperPreview *wallpaperPrview = nullptr;
    QStringList needDelWallpaper;

public:
    QHBoxLayout *toolLayout = nullptr;
    DButtonBox *waitControl = nullptr;
    QCheckBox *lockScreenBox = nullptr;
    QLabel *waitControlLabel = nullptr;

public:
    WallpaperSettings *q;
};

}

#endif   // WALLPAPERSETTINGS_P_H
