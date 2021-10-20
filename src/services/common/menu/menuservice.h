/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
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
#include "dfm-base/base/abstractfilemenu.h"
#include "menufactory.h"
#include <QUrl>
#include <functional>
#include <QMenu>
#include <dfm-framework/service/pluginservicecontext.h>

DSC_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

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

    template<class T>
    static bool regClass(const QString &name, QString *errorString = nullptr)
    {
        return MenuFactory::regClass<T>(name, errorString);
    }

    QMenu *createMenu(AbstractFileMenu::MenuMode mode,
                      const QUrl &rootUrl,
                      const QUrl &foucsUrl,
                      const QList<QUrl> selected);

private:
    explicit MenuService(QObject *parent = nullptr);
    virtual ~MenuService() override;
};

DSC_END_NAMESPACE
#endif // MENUSERVICE_H
