/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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

#include "dfm-base/base/menufactory.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/widgets/action/actiondatacontainer.h"
#include "dfm-base/dfm_actiontype_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/interfaces/abstractmenu.h"
#include "dfm-base/interfaces/abstractmenuscene.h"
#include "dfm-base/interfaces/abstractscenecreator.h"

#include <dfm-framework/service/pluginservicecontext.h>

#include <QUrl>
#include <QMenu>

#include <functional>

DSC_BEGIN_NAMESPACE

using ActionCreateCb = std::function<QString(bool isNormal, const QUrl &currentUrl, const QUrl &focusFile, const QList<QUrl> &selected)>;
using ActionClickedCb = std::function<void(bool isNormal, const QUrl &currentUrl, const QUrl &focusFile, const QList<QUrl> &selected)>;

struct ActionInfo
{
    DFMBASE_NAMESPACE::ExtensionType type;
    ActionCreateCb createCb { nullptr };
    ActionClickedCb clickedCb { nullptr };
};

class MenuServicePrivate;
class MenuService final : public dpf::PluginService, dpf::AutoServiceRegister<MenuService>
{
    Q_OBJECT
    Q_DISABLE_COPY(MenuService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    Q_DECLARE_FLAGS(ExtensionFlags, DFMBASE_NAMESPACE::ExtensionType)

    static QString name()
    {
        return "org.deepin.service.MenuService";
    }

    static MenuService *service();

    template<class T>
    static bool regClass(const QString &name, QString *errorString = nullptr)
    {
        return DFMBASE_NAMESPACE::MenuFactory::regClass<T>(name, errorString);
    }

    QMenu *createMenu(QWidget *parent,
                      const QString &scene,
                      DFMBASE_NAMESPACE::AbstractMenu::MenuMode mode,
                      const QUrl &rootUrl,
                      const QUrl &focusUrl,
                      const QList<QUrl> selected,
                      bool onDesktop = false,
                      ExtensionFlags flags = DFMBASE_NAMESPACE::ExtensionType::kAllExtensionAction,
                      QVariant customData = QVariant());

    // TODO(Lee):移植最新的menu后删除

    static void regAction(ActionInfo &info);

    // TODO(lee) delete old interface when new menu frame be used
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
