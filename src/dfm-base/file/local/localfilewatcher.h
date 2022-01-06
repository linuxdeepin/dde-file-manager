/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef LOCALFILEWATCHER_H
#define LOCALFILEWATCHER_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"

#include <dfm-io/core/dfileinfo.h>

#include <QObject>

class QUrl;
DFMBASE_BEGIN_NAMESPACE
class LocalFileWatcher : public AbstractFileWatcher
{
    Q_OBJECT
public:
    explicit LocalFileWatcher() = delete;
    explicit LocalFileWatcher(const QUrl &url, QObject *parent = nullptr);
    ~LocalFileWatcher() override;

    //debug function
    static QStringList getMonitorFiles();
};
DFMBASE_END_NAMESPACE

#endif   // LOCALFILEWATCHER_H
