// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREHELPER_H
#define COREHELPER_H

#include "dfmplugin_core_global.h"

#include <QObject>
#include <QVariant>

DPCORE_BEGIN_NAMESPACE

class CoreHelper
{
public:
    static void cd(quint64 windowId, const QUrl &url);
    static void openNewWindow(const QUrl &url, const QVariant &opt = QVariant());
};

DPCORE_END_NAMESPACE

#endif   // COREHELPER_H
