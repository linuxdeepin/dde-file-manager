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
#ifndef WALLAPERPREVIEW_H
#define WALLAPERPREVIEW_H

#include "ddplugin_wallpapersetting_global.h"
#include "backgroundpreview.h"

#include "interfaces/screen/abstractscreen.h"

#ifdef COMPILE_ON_V23
#include "dbus/appearance_interface.h"
using BackgroudInter = org::deepin::dde::Appearance1;
#else
#include <com_deepin_wm.h>
using BackgroudInter = com::deepin::wm;
#endif

#include <QObject>
#include <QMap>

namespace ddplugin_wallpapersetting {

class WallaperPreview : public QObject
{
    Q_OBJECT
public:
    explicit WallaperPreview(QObject *parent = nullptr);
    ~WallaperPreview();
    void init();
    void setVisible(bool visible);
    bool isVisible() const {
        return visible;
    }
    void pullImageSettings();
    void setWallpaper(const QString &screen, const QString &image);
    inline QMap<QString, QString> wallpaper() const {
        return wallpapers;
    }
    inline PreviewWidgetPtr widget(const QString &screen) const {
        return previewWidgets.value(screen);
    }
signals:

public slots:
    void buildWidgets();
    void updateGeometry();
    void updateWallpaper();
protected:
    PreviewWidgetPtr createWidget(DFMBASE_NAMESPACE::ScreenPointer);
    QString getBackground(const QString &screen);
private:
    bool visible = false;
    BackgroudInter *inter = nullptr;
    QMap<QString, QString> wallpapers;
    QMap<QString, PreviewWidgetPtr> previewWidgets;
};

}

#endif // WALLAPERPREVIEW_H
