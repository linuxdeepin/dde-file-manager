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
#ifndef TAGDBUSINTERFACE_H
#define TAGDBUSINTERFACE_H

#include "dfmplugin_tag_global.h"

#include <QtDBus/QtDBus>

namespace dfmplugin_tag {

class TagDBusInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit TagDBusInterface(QObject *parent = nullptr);

public Q_SLOTS:
    QDBusPendingReply<QDBusVariant> disposeClientData(const QVariantMap &filesAndTags, qulonglong type);

Q_SIGNALS:
    void addNewTags(const QDBusVariant &new_tags);
    void changeTagColor(const QVariantMap &old_and_new_color);
    void changeTagName(const QVariantMap &old_and_new_name);
    void deleteTags(const QDBusVariant &be_deleted_tags);
    void filesWereTagged(const QVariantMap &tagged_files);
    void untagFiles(const QVariantMap &file_be_removed_tags);
};

}

#endif   // TAGDBUSINTERFACE_H
