/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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

#include "dfmextmenuplugin.h"

BEGEN_DFMEXT_NAMESPACE
class DFMExtMenuPluginPrivate
{
public:
    DFMExtMenuPlugin::InitializeFunc initializeFunc;
    DFMExtMenuPlugin::BuildNormalMenuFunc buildNormalMeunFunc;
    DFMExtMenuPlugin::BuildEmptyAreaMenuFunc buildEmptyAreaMenuFunc;
};
END_DFMEXT_NAMESPACE

USING_DFMEXT_NAMESPACE

DFMExtMenuPlugin::DFMExtMenuPlugin()
    : d(new DFMExtMenuPluginPrivate)
{

}

DFMExtMenuPlugin::~DFMExtMenuPlugin()
{
    delete d;
}

void DFMExtMenuPlugin::initialize(DFMExtMenuProxy *proxy)
{
    if (d->initializeFunc)
        d->initializeFunc(proxy);
}

void DFMExtMenuPlugin::registerInitialize(const DFMExtMenuPlugin::InitializeFunc &func)
{
    d->initializeFunc = func;
}

bool DFMExtMenuPlugin::buildNormalMenu(DFMExtMenu *main,
                                       const std::string &currentUrl,
                                       const std::string &focusUrl,
                                       const std::list<std::string> &urlList,
                                       bool onDesktop)
{
    if (d->buildNormalMeunFunc)
        return d->buildNormalMeunFunc(main, currentUrl, focusUrl, urlList, onDesktop);

    return false;
}

void DFMExtMenuPlugin::registerBuildNormalMenu(const DFMExtMenuPlugin::BuildNormalMenuFunc &func)
{
    d->buildNormalMeunFunc = func;
}

bool DFMExtMenuPlugin::buildEmptyAreaMenu(DFMExtMenu *main, const std::string &currentUrl, bool onDesktop)
{
    if (d->buildEmptyAreaMenuFunc)
        return d->buildEmptyAreaMenuFunc(main, currentUrl, onDesktop);

    return false;
}

void DFMExtMenuPlugin::registerBuildEmptyAreaMenu(const DFMExtMenuPlugin::BuildEmptyAreaMenuFunc &func)
{
    d->buildEmptyAreaMenuFunc = func;
}
