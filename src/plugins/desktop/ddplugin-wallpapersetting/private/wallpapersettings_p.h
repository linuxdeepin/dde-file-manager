/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef WALLPAPERSETTINGS_P_H
#define WALLPAPERSETTINGS_P_H

#include "wallpapersettings.h"
#include "wallpaperlist.h"
#include "loadinglabel.h"
#include "wallaperpreview.h"
#include "dbus/screensaver_interface.h"

#include <com_deepin_wm.h>
#include <com_deepin_daemon_appearance.h>
#include <com_deepin_sessionmanager.h>

#include <DIconButton>
#include <DRegionMonitor>
#include <DButtonBox>

#include <QObject>
#include <QHBoxLayout>
#include <QCheckBox>

class QLabel;
DWIDGET_USE_NAMESPACE

namespace ddplugin_wallpapersetting {
using WMInter = com::deepin::wm;
using AppearanceIfs = com::deepin::daemon::Appearance;
using SessionIfs = com::deepin::SessionManager;
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
    void initSreenSaver();
    void initPreivew();
    QList<QPair<QString, bool> > processListReply(const QString &reply);
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
    WMInter *wmInter = nullptr;
    AppearanceIfs *appearanceIfs = nullptr;
    SessionIfs *sessionIfs = nullptr;
    ScreenSaverIfs *screenSaverIfs = nullptr;
    DRegionMonitor *regionMonitor = nullptr;
public:
    DButtonBox *switchModeControl = nullptr;
    LoadingLabel *loadingLabel = nullptr;
    QTimer reloadTimer;
public:
    DIconButton *closeButton  = nullptr;
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

#endif // WALLPAPERSETTINGS_P_H
