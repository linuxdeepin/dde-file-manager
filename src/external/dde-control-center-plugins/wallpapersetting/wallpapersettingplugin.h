// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WALLPAPERSETTINGPLUGIN_H
#define WALLPAPERSETTINGPLUGIN_H

#include "wallpaper/wallpaperwindow.h"

#include <interface/moduleinterface.h>
#include <interface/frameproxyinterface.h>

#include <QObject>

namespace dfm_wallpapersetting {
class WallpaperProvider;
class WallpaperSettingPlugin : public QObject, public DCC_NAMESPACE::ModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ModuleInterface_iid FILE "api.json")
    Q_INTERFACES(DCC_NAMESPACE::ModuleInterface)
public:
    explicit WallpaperSettingPlugin(QObject *parent = nullptr);
    ~WallpaperSettingPlugin();
    void preInitialize(bool sync = false,
                               DCC_NAMESPACE::FrameProxyInterface::PushType
                               = DCC_NAMESPACE::FrameProxyInterface::PushType::Normal) override;
    void initialize() override;
    const QString name() const override;
    const QString displayName() const override;
    QIcon icon() const override;
    QString translationPath() const override;
    QString path() const override;
    QString follow() const override;
    void addChildPageTrans() const override;
    QStringList availPage() const override;
    void showPage(const QString &pageName) override;
private:
    void initSearchData() override;
signals:

public slots:
    void active() override;
private:
    WallpaperProvider *provider = nullptr;
};

}

#endif // WALLPAPERSETTINGPLUGIN_H
