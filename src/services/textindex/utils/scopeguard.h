// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCOPEGUARD_H
#define SCOPEGUARD_H

#include "service_textindex_global.h"

#include <functional>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class ScopeGuard
{
public:
    explicit ScopeGuard(std::function<void()> onExit)
        : m_exitFunc(std::move(onExit)), m_dismissed(false) { }

    ~ScopeGuard()
    {
        if (!m_dismissed && m_exitFunc)
            m_exitFunc();
    }

    void dismiss() { m_dismissed = true; }

private:
    std::function<void()> m_exitFunc;
    bool m_dismissed;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // SCOPEGUARD_H
