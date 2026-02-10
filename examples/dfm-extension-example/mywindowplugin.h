// SPDX-FileCopyrightText: 2023 - 2026 ~ 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MYWINDOWPLUGIN_H
#define MYWINDOWPLUGIN_H

#include <dfm-extension/window/dfmextwindowplugin.h>

namespace Exapmle {

class MyWindowPlugin : public DFMEXT::DFMExtWindowPlugin
{
public:
    MyWindowPlugin();

    void windowOpened(std::uint64_t winId) DFM_FAKE_OVERRIDE;
    void windowClosed(std::uint64_t winId) DFM_FAKE_OVERRIDE;
    void firstWindowOpened(std::uint64_t winId) DFM_FAKE_OVERRIDE;
    void lastWindowClosed(std::uint64_t winId) DFM_FAKE_OVERRIDE;
    void windowUrlChanged(std::uint64_t winId, const std::string &urlString) DFM_FAKE_OVERRIDE;
};

}   // namespace Exapmle

#endif   // MYWINDOWPLUGIN_H
