// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFMEXTWINDOWPLUGIN_H
#define DFMEXTWINDOWPLUGIN_H

#include <dfm-extension/dfm-extension-global.h>

#include <functional>
#include <string>
#include <cstdint>

BEGEN_DFMEXT_NAMESPACE

class DFMExtWindowPluginPrivate;
class DFMExtWindowPlugin
{
    DFM_DISABLE_COPY(DFMExtWindowPlugin)

public:
    using WindowFunc = std::function<void(std::uint64_t)>;
    using WindowUrlFunc = std::function<void(std::uint64_t, const std::string &)>;

public:
    DFMExtWindowPlugin();
    ~DFMExtWindowPlugin();

    DFM_FAKE_VIRTUAL void windowOpened(std::uint64_t winId);
    DFM_FAKE_VIRTUAL void windowClosed(std::uint64_t winId);
    DFM_FAKE_VIRTUAL void firstWindowOpened(std::uint64_t winId);
    DFM_FAKE_VIRTUAL void lastWindowClosed(std::uint64_t winId);
    DFM_FAKE_VIRTUAL void windowUrlChanged(std::uint64_t winId, const std::string &urlString);

public:
    void registerWindowOpened(const WindowFunc &func);
    void registerWindowClosed(const WindowFunc &func);
    void registerFirstWindowOpened(const WindowFunc &func);
    void registerLastWindowClosed(const WindowFunc &func);
    void registerWindowUrlChanged(const WindowUrlFunc &func);

private:
    DFMExtWindowPluginPrivate *d;
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTWINDOWPLUGIN_H
