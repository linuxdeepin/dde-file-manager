/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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
#include "tagdbusinterface.h"

using namespace dfmplugin_tag;

namespace DaemonServiceIFace {
static constexpr char kInterfaceService[] { "com.deepin.filemanager.daemon" };
static constexpr char kInterfacePath[] { "/com/deepin/filemanager/daemon/TagManagerDaemon" };
static constexpr char kInterfaceName[] { "com.deepin.filemanager.daemon.TagManagerDaemon" };

static constexpr char kFuncDisposeData[] { "disposeClientData" };
}

TagDBusInterface::TagDBusInterface(QObject *parent)
    : QDBusAbstractInterface(DaemonServiceIFace::kInterfaceService,
                             DaemonServiceIFace::kInterfacePath,
                             DaemonServiceIFace::kInterfaceName,
                             QDBusConnection::systemBus(), parent)
{
}

QDBusPendingReply<QDBusVariant> TagDBusInterface::disposeClientData(const QVariantMap &filesAndTags, qulonglong type)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(filesAndTags) << QVariant::fromValue(type);

    return asyncCallWithArgumentList(DaemonServiceIFace::kFuncDisposeData, argumentList);
}
