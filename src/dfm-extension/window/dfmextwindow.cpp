// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmextwindow.h"

#include <cassert>

BEGEN_DFMEXT_NAMESPACE

DFMExtWindow::~DFMExtWindow()
{
    if (d)
        delete d;
}

void DFMExtWindow::cd(const std::string &urlString)
{
}

std::string DFMExtWindow::currentUrlString() const
{
    return {};
}

uint64_t DFMExtWindow::internalWinId() const
{
    return {};
}

DFMExtWindow::DFMExtWindow(DFMExtWindowPrivate *d_ptr)
    : d(d_ptr)
{
    assert(d);
}

END_DFMEXT_NAMESPACE
