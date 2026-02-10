// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/lifecycle/plugin.h>

DPF_USE_NAMESPACE

/*!
 * \brief Plugin::initialize This function is multi-threaded execution
 * and can be used internally for some thread-safe functions, class operations
 */
void Plugin::initialize()
{
    qCDebug(logDPF) << "Plugin::initialize: default implementation called";
}

/*!
 * \brief Plugin::stop
 */
void Plugin::stop()
{
    qCDebug(logDPF) << "Plugin::stop: default implementation called";
}
