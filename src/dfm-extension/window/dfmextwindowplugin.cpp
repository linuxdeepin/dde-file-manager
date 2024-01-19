// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-extension/window/dfmextwindowplugin.h>

BEGEN_DFMEXT_NAMESPACE

class DFMExtWindowPluginPrivate
{
public:
    DFMExtWindowPlugin::WindowFunc windowOpenedFunc;
    DFMExtWindowPlugin::WindowFunc windowClosedFunc;
    DFMExtWindowPlugin::WindowFunc firstWindowOpenedFunc;
    DFMExtWindowPlugin::WindowFunc lastWindowClosedFunc;
    DFMExtWindowPlugin::WindowUrlFunc windowUrlChangedFunc;
};

DFMExtWindowPlugin::DFMExtWindowPlugin()
    : d(new DFMExtWindowPluginPrivate)
{
}

DFMExtWindowPlugin::~DFMExtWindowPlugin()
{
    delete d;
}

void DFMExtWindowPlugin::windowOpened(uint64_t winId)
{
    if (d->windowOpenedFunc)
        d->windowOpenedFunc(winId);
}

void DFMExtWindowPlugin::windowClosed(uint64_t winId)
{
    if (d->windowClosedFunc)
        d->windowClosedFunc(winId);
}

void DFMExtWindowPlugin::firstWindowOpened(uint64_t winId)
{
    if (d->firstWindowOpenedFunc)
        d->firstWindowOpenedFunc(winId);
}

void DFMExtWindowPlugin::lastWindowClosed(uint64_t winId)
{
    if (d->lastWindowClosedFunc)
        d->lastWindowClosedFunc(winId);
}

void DFMExtWindowPlugin::windowUrlChanged(uint64_t winId, const std::string &urlString)
{
    if (d->windowUrlChangedFunc)
        d->windowUrlChangedFunc(winId, urlString);
}

void DFMExtWindowPlugin::registerWindowOpened(const WindowFunc &func)
{
    d->windowOpenedFunc = func;
}

void DFMExtWindowPlugin::registerWindowClosed(const WindowFunc &func)
{
    d->windowClosedFunc = func;
}

void DFMExtWindowPlugin::registerFirstWindowOpened(const WindowFunc &func)
{
    d->firstWindowOpenedFunc = func;
}

void DFMExtWindowPlugin::registerLastWindowClosed(const WindowFunc &func)
{
    d->lastWindowClosedFunc = func;
}

void DFMExtWindowPlugin::registerWindowUrlChanged(const WindowUrlFunc &func)
{
    d->windowUrlChangedFunc = func;
}

END_DFMEXT_NAMESPACE
