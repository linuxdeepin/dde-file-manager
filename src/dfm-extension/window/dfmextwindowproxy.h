// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTWINDOWPROXY_H
#define DFMEXTWINDOWPROXY_H

#include <dfm-extension/dfm-extension-global.h>

#include <string>
#include <vector>
#include <cstdint>

BEGEN_DFMEXT_NAMESPACE

// TODO: impl me

class DFMExtWindow;
class DFMExtWindowProxyPrivate;
class DFMExtWindowProxy
{
    friend class DFMExtWindowProxyPrivate;

public:
    ~DFMExtWindowProxy();

    DFMExtWindow *createWindow(const std::string &urlString);
    void showWindow(std::uint64_t winId);
    std::vector<std::uint64_t> windowIdList();

protected:
    explicit DFMExtWindowProxy(DFMExtWindowProxyPrivate *d_ptr);
    DFMExtWindowProxyPrivate *d;
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTWINDOWPROXY_H
