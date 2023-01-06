// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
