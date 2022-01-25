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
#ifndef WALLPAPERSETTINGS_H
#define WALLPAPERSETTINGS_H

#include "dfm_desktop_service_global.h"

#include <DBlurEffectWidget>

DSB_D_BEGIN_NAMESPACE
class WallpaperSettingsPrivate;
class WallpaperSettings : public DTK_WIDGET_NAMESPACE::DBlurEffectWidget
{
    Q_OBJECT
    friend class WallpaperSettingsPrivate;
    Q_DISABLE_COPY(WallpaperSettings)
public:
    enum class Mode {
        WallpaperMode,
        ScreenSaverMode
    };
    explicit WallpaperSettings(const QString &screenName, Mode mode = Mode::WallpaperMode, QWidget *parent = nullptr);
    ~WallpaperSettings();
    void switchMode(Mode mode);
    QString wallpaperSlideShow() const;
    void setWallpaperSlideShow(const QString &period);
    static QVector<int> availableScreenSaverTime();
    static QStringList availableWallpaperSlide();
    void adjustGeometry();
    void refreshList();
    QPair<QString,QString> currentWallpaper() const;
public:
    void onGeometryChanged();
signals:
    void backgroundChanged();
    void quit();
protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    bool eventFilter(QObject *object, QEvent *event) override;
    void showLoading();
    void closeLoading();
    void loadWallpaper();
    void loadScreenSaver();
    void applyToDesktop();
    void applyToGreeter();
private:
    void init();
private:
    WallpaperSettingsPrivate *d;
};

DSB_D_END_NAMESPACE

#endif // WALLPAPERSETTINGS_H
