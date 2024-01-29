// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTWINDOW_H
#define DFMEXTWINDOW_H

#include <dfm-extension/dfm-extension-global.h>

#include <string>
#include <cstdint>

BEGEN_DFMEXT_NAMESPACE

class DFMExtWindowPrivate;   // TODO: impl me

class DFMExtWindow
{
    friend class DFMExtWindowPrivate;

public:
    ~DFMExtWindow();

    void cd(const std::string &urlString);
    std::string currentUrlString() const;
    std::uint64_t internalWinId() const;

protected:
    explicit DFMExtWindow(DFMExtWindowPrivate *d_ptr);
    DFMExtWindowPrivate *d;
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTWINDOW_H
