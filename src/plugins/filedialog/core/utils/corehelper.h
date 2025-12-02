// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREHELPER_H
#define COREHELPER_H

#include "filedialogplugin_core_global.h"

#include <dfm-base/utils/windowutils.h>

#include <QMimeDatabase>
#include <QWidget>

#include <functional>

namespace filedialog_core {

class CoreHelper
{
public:
    static void delayInvokeProxy(std::function<void()> func, quint64 winID, QObject *parent);
    static bool askHiddenFile(QWidget *parent);
    static bool askReplaceFile(QString fileName, QWidget *parent);
    static QStringList stripFilters(const QStringList &filters);
    static QString findExtensionName(const QString &fileName, const QStringList &newNameFilters, QMimeDatabase *db);
};

}

#endif   // COREHELPER_H
