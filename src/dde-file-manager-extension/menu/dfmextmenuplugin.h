// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    using InitializeFunc = std::function<void(DFMEXT::DFMExtMenuProxy *proxy)>;
    using BuildNormalMenuFunc = std::function<bool(DFMExtMenu *,
                                                   const std::string &,
                                                   const std::string &,
                                                   const std::list<std::string> &, bool)>;
    using BuildEmptyAreaMenuFunc = std::function<bool(DFMEXT::DFMExtMenu *, const std::string &, bool)>;

public:
    DFMExtMenuPlugin();
    ~DFMExtMenuPlugin();

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

#endif   // DFMEXTMENUPLUGIN_H
