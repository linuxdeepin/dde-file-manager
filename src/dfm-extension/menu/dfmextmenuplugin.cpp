// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-extension/menu/dfmextmenuplugin.h>

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
                                       const std::string &currentPath,
                                       const std::string &focusPath,
                                       const std::list<std::string> &pathList,
                                       bool onDesktop)
{
    if (d->buildNormalMeunFunc)
        return d->buildNormalMeunFunc(main, currentPath, focusPath, pathList, onDesktop);

    return false;
}

void DFMExtMenuPlugin::registerBuildNormalMenu(const DFMExtMenuPlugin::BuildNormalMenuFunc &func)
{
    d->buildNormalMeunFunc = func;
}

bool DFMExtMenuPlugin::buildEmptyAreaMenu(DFMExtMenu *main, const std::string &currentPath, bool onDesktop)
{
    if (d->buildEmptyAreaMenuFunc)
        return d->buildEmptyAreaMenuFunc(main, currentPath, onDesktop);

    return false;
}

void DFMExtMenuPlugin::registerBuildEmptyAreaMenu(const DFMExtMenuPlugin::BuildEmptyAreaMenuFunc &func)
{
    d->buildEmptyAreaMenuFunc = func;
}
