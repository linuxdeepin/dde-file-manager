/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef EXTENDMENUPLUGIN_H
#define EXTENDMENUPLUGIN_H

#include "dfmplugin_extend_menu_global.h"

#include <services/common/dfm_common_service_global.h>

#include <dfm-framework/framework.h>

namespace dfm_service_common {
class MenuService;
}

DPEXTENDMENU_BEGIN_NAMESPACE

class ExtendMenuPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "extendmenu.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;

private:
    void regDefaultScene();
    void bindScene(const QString &parentScene);

private:
    dfm_service_common::MenuService *menuServer = nullptr;
};

DPEXTENDMENU_END_NAMESPACE

#endif   // EXTENDMENUPLUGIN_H
