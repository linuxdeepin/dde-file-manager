// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WALLPAPERSETTINGS_H
#define WALLPAPERSETTINGS_H

#include "ddplugin_wallpapersetting_global.h"

#include <DBlurEffectWidget>

namespace ddplugin_wallpapersetting {
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
    ~WallpaperSettings() override;
    void switchMode(Mode mode);
    QString wallpaperSlideShow() const;
    void setWallpaperSlideShow(const QString &period);
    static QVector<int> availableScreenSaverTime();
    static QStringList availableWallpaperSlide();
    void adjustGeometry();
    Q_INVOKABLE void refreshList();
    QPair<QString, QString> currentWallpaper() const;

public:
    void onGeometryChanged();
signals:
    void backgroundChanged();
    void quit();

protected:
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
    bool isWallpaperLocked() const;

private:
    void init();

private:
    WallpaperSettingsPrivate *d;
};

}

#endif   // WALLPAPERSETTINGS_H
