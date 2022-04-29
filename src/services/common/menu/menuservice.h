/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef MENUSERVICE_H
#define MENUSERVICE_H

#include "dfm_common_service_global.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/interfaces/abstractmenuscene.h"
#include <dfm-base/interfaces/abstractscenecreator.h>

#include <dfm-framework/service/pluginservicecontext.h>

#include <QUrl>
#include <QMenu>

#include <functional>

DSC_BEGIN_NAMESPACE

class MenuServicePrivate;
class MenuService final : public dpf::PluginService, dpf::AutoServiceRegister<MenuService>
{
    Q_OBJECT
    Q_DISABLE_COPY(MenuService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.MenuService";
    }

    static MenuService *service();
    bool contains(const QString &name) const;
    bool registerScene(const QString &name, DFMBASE_NAMESPACE::AbstractSceneCreator *creator);
    DFMBASE_NAMESPACE::AbstractSceneCreator *unregisterScene(const QString &name);
    bool bind(const QString &name, const QString &parent);
    void unBind(const QString &name, const QString &parent = QString());
    DFMBASE_NAMESPACE::AbstractMenuScene *createScene(const QString &name) const;
signals:
    void sceneAdded(const QString &scene);
    void sceneRemoved(const QString &scene);
private:
    explicit MenuService(QObject *parent = nullptr);
    virtual ~MenuService() override;

private:
    MenuServicePrivate *d { nullptr };
};

DSC_END_NAMESPACE
#endif   // MENUSERVICE_H
