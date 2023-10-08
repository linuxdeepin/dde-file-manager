// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSAVERSETTINGPLUGIN_H
#define SCREENSAVERSETTINGPLUGIN_H

#include "screensaver/screensaverwindow.h"

#include <interface/moduleinterface.h>
#include <interface/frameproxyinterface.h>

#include <QObject>

namespace dfm_wallpapersetting {

class ScreensaverSettingPlugin : public QObject, public DCC_NAMESPACE::ModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ModuleInterface_iid FILE "api.json")
    Q_INTERFACES(DCC_NAMESPACE::ModuleInterface)
public:
    explicit ScreensaverSettingPlugin(QObject *parent = nullptr);
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
    bool enabled() const override;
private:
    void initSearchData() override;
signals:

public slots:
    void active() override;
private:
    ScreensaverProvider *provider = nullptr;
};

}

#endif // SCREENSAVERSETTINGPLUGIN_H
