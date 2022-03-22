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
#ifndef TAGDBUSHELPER_H
#define TAGDBUSHELPER_H

#include "dfmplugin_tag_global.h"

#include <QSharedPointer>
#include <QDBusVariant>

DPTAG_BEGIN_NAMESPACE

class TagDBusInterface;
class TagDBusHelper : public QObject
{
    Q_OBJECT
public:
    explicit TagDBusHelper(QObject *parent = nullptr);

    QVariant sendDataToDBus(const QVariantMap &filesAndTags, TagActionType type);

public slots:
    void onAddNewTags(const QDBusVariant &newTags);
    void onChangeTagColor(const QVariantMap &oldAndNewColor);
    void onChangeTagName(const QVariantMap &oldAndNewName);
    void onDeleteTags(const QDBusVariant &deletedTags);
    void onFilesWereTagged(const QVariantMap &taggedFiles);
    void onUntagFiles(const QVariantMap &untaggedFiles);

signals:
    void addNewTags(const QStringList &tags);
    void tagsDeleted(const QStringList &tags);
    void tagColorChanged(const QMap<QString, QString> &oldAndNew);
    void tagNameChanged(const QMap<QString, QString> &oldAndNew);
    void filesTagged(const QMap<QString, QList<QString>> &fileAndTags);
    void filesUntagged(const QMap<QString, QList<QString>> &fileAndTags);

private:
    void initializeConnect();

    QSharedPointer<TagDBusInterface> dbusInterface;
};

DPTAG_END_NAMESPACE

#endif   // TAGDBUSHELPER_H
