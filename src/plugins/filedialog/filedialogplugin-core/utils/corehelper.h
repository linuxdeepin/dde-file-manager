/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef COREHELPER_H
#define COREHELPER_H

#include "filedialogplugin_core_global.h"

#include "dfm-base/utils/windowutils.h"

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
    static QString findExtensioName(const QString &fileName, const QStringList &newNameFilters, QMimeDatabase *db);
    static void urlTransform(QList<QUrl> *urls);
};

}

#endif   // COREHELPER_H
