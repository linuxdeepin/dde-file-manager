// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmextwindowproxy.h"

#include <cassert>

BEGEN_DFMEXT_NAMESPACE

DFMExtWindowProxy::~DFMExtWindowProxy()
{
    delete d;
}

DFMExtWindow *DFMExtWindowProxy::createWindow(const std::string &urlString)
{
    return {};
}

void DFMExtWindowProxy::showWindow(std::uint64_t winId)
{
}

std::vector<uint64_t> DFMExtWindowProxy::windowIdList()
{
    return {};
}

DFMExtWindowProxy::DFMExtWindowProxy(DFMExtWindowProxyPrivate *d_ptr)
    : d(d_ptr)
{
    assert(d);
}

END_DFMEXT_NAMESPACE
