/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef FRAME_H
#define FRAME_H

#include <dblureffectwidget.h>
#include <dimagebutton.h>
#include <dregionmonitor.h>

DWIDGET_BEGIN_NAMESPACE
class DSegmentedControl;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QCheckBox;
class QHBoxLayout;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class WallpaperList;
class WallpaperListView;
class ComDeepinDaemonAppearanceInterface;
class ComDeepinScreenSaverInterface;
class DeepinWM;
class BackgroundHelper;
class Frame : public DBlurEffectWidget
{
    Q_OBJECT

public:
    enum Mode {
        WallpaperMode,
        ScreenSaverMode
    };

    Frame(QFrame *parent = 0);
    ~Frame();

    void show();
    void hide();

    QString desktopBackground() const;

signals:
    void aboutHide();
    void done();

public slots:
    void handleNeedCloseButton(QString path, QPoint pos);

protected:
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void keyPressEvent(QKeyEvent *);
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

private:
#if !defined(DISABLE_SCREENSAVER) || !defined(DISABLE_WALLPAPER_CAROUSEL)
    void adjustModeSwitcherPoint();
    DSegmentedControl *m_switchModeControl;
#endif

#ifndef DISABLE_SCREENSAVER
    void setMode(int mode);
    void reLayoutTools();

    Mode m_mode = WallpaperMode;
    QHBoxLayout *m_toolLayout;
    QLabel *m_waitControlLabel;
    DSegmentedControl *m_waitControl;
    QCheckBox *m_lockScreenBox;
#else
    const Mode m_mode = WallpaperMode;
#endif
    WallpaperList *m_wallpaperList = NULL;
    QString m_desktopWallpaper;
    QString m_lockWallpaper;
    DImageButton * m_closeButton = NULL;

#ifndef DISABLE_WALLPAPER_CAROUSEL
    QHBoxLayout *m_wallpaperCarouselLayout;
    QCheckBox *m_wallpaperCarouselCheckBox;
    DSegmentedControl *m_wallpaperCarouselControl;
#endif

    ComDeepinDaemonAppearanceInterface * m_dbusAppearance = NULL;
#ifndef DISABLE_SCREENSAVER
    ComDeepinScreenSaverInterface *m_dbusScreenSaver = nullptr;
#endif
    DeepinWM * m_dbusDeepinWM = NULL;
    DRegionMonitor * m_mouseArea = NULL;

    QString m_formerWallpaper;
    QMap<QString, bool> m_deletableInfo;

    BackgroundHelper *m_backgroundHelper = nullptr;

    void initUI();
    void initSize();
    void initListView();
    void refreshList();
    void onItemPressed(const QString &data);
    void onItemButtonClicked(const QString &buttonID);
    QStringList processListReply(const QString &reply);
};

#endif // FRAME_H
