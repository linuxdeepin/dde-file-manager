/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "dbusadaptor/tagmanagerdaemon_adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class TagManagerDaemonAdaptor
 */

TagManagerDaemonAdaptor::TagManagerDaemonAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

TagManagerDaemonAdaptor::~TagManagerDaemonAdaptor()
{
    // destructor
}

QDBusVariant TagManagerDaemonAdaptor::disposeClientData(const QVariantMap &filesAndTags, qulonglong type)
{
    // handle method call com.deepin.filemanager.daemon.TagManagerDaemon.disposeClientData
    QDBusVariant result;
    QMetaObject::invokeMethod(parent(), "disposeClientData", Q_RETURN_ARG(QDBusVariant, result), Q_ARG(QVariantMap, filesAndTags), Q_ARG(qulonglong, type));
    return result;
}

