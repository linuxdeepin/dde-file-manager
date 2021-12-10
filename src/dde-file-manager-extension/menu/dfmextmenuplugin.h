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
#ifndef DFMEXTMENUPLUGIN_H
#define DFMEXTMENUPLUGIN_H

#include "dfm-extension-global.h"

#include <string>
#include <list>
#include <functional>

BEGEN_DFMEXT_NAMESPACE
class DFMExtMenu;
class DFMExtMenuProxy;
class DFMExtMenuPluginPrivate;
class DFMExtMenuPlugin
{
    DFM_DISABLE_COPY(DFMExtMenuPlugin)
public:
    using InitializeFunc = std::function<void (DFMEXT::DFMExtMenuProxy *proxy)>;
    using BuildNormalMenuFunc = std::function<bool (DFMExtMenu *, 
                                const std::string &,
                                const std::string &,
                                const std::list<std::string> &, bool)>;
    using BuildEmptyAreaMenuFunc = std::function<bool (DFMEXT::DFMExtMenu *, const std::string &, bool)>;

public:
    DFMExtMenuPlugin();
    virtual ~DFMExtMenuPlugin();

    DFM_FAKE_VIRTUAL void initialize(DFMEXT::DFMExtMenuProxy *proxy);
    DFM_FAKE_VIRTUAL bool buildNormalMenu(DFMEXT::DFMExtMenu *main,
                                          const std::string &currentUrl,
                                          const std::string &focusUrl,
                                          const std::list<std::string> &urlList,
                                          bool onDesktop);
    DFM_FAKE_VIRTUAL bool buildEmptyAreaMenu(DFMEXT::DFMExtMenu *main, const std::string &currentUrl, bool onDesktop);

public:
    void registerInitialize(const InitializeFunc &func);
    void registerBuildNormalMenu(const BuildNormalMenuFunc &func);
    void registerBuildEmptyAreaMenu(const BuildEmptyAreaMenuFunc &func);

private:
    DFMExtMenuPluginPrivate *d;
};

END_DFMEXT_NAMESPACE

#endif // DFMEXTMENUPLUGIN_H
