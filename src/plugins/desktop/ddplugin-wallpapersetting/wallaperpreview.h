// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WALLAPERPREVIEW_H
#define WALLAPERPREVIEW_H

#include "ddplugin_wallpapersetting_global.h"
#include "backgroundpreview.h"

#include <dfm-base/interfaces/screen/abstractscreen.h>

#ifdef COMPILE_ON_V2X
#    include "dbus/appearance_interface.h"
using BackgroudInter = org::deepin::dde::Appearance1;
#else
#    include <com_deepin_wm.h>
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
    bool isVisible() const
    {
        return visible;
    }
    void pullImageSettings();
    void setWallpaper(const QString &screen, const QString &image);
    inline QMap<QString, QString> wallpaper() const
    {
        return wallpapers;
    }
    inline PreviewWidgetPtr widget(const QString &screen) const
    {
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

#endif   // WALLAPERPREVIEW_H
