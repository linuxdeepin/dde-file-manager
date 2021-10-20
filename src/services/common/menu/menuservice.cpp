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
#include "menuservice.h"

DSC_BEGIN_NAMESPACE

QMenu *MenuService::createMenu(AbstractFileMenu::MenuMode mode,
                               const QUrl &rootUrl,
                               const QUrl &foucsUrl,
                               const QList<QUrl> selected)
{
    if (!rootUrl.isValid())
        return nullptr;

    auto topClass = MenuFactory::create(rootUrl.scheme());
    if (!topClass)
        return nullptr;
    return topClass->build(mode, rootUrl, foucsUrl, selected);;
}

MenuService::MenuService(QObject *parent)
    : dpf::PluginService (parent)
    , dpf::AutoServiceRegister<MenuService>()
{

}

MenuService::~MenuService()
{

}

DSC_END_NAMESPACE
