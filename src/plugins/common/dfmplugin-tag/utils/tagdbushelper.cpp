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
#include "tagdbushelper.h"

#include "interfaces/tagdbusinterface.h"

using namespace dfmplugin_tag;

TagDBusHelper::TagDBusHelper(QObject *parent)
    : QObject(parent)
{
    dbusInterface.reset(new TagDBusInterface(this));
    initializeConnect();
}

QVariant TagDBusHelper::sendDataToDBus(const QVariantMap &filesAndTags, TagActionType type)
{
    if (dbusInterface) {
        QDBusVariant dbusVariant = dbusInterface->disposeClientData(filesAndTags, static_cast<unsigned long long>(type));
        QVariant variant = dbusVariant.variant();
        QDBusArgument argument = variant.value<QDBusArgument>();
        QDBusArgument::ElementType currentType = argument.currentType();

        QMap<QString, QVariant> variantMap {};

        if (currentType == QDBusArgument::ElementType::MapType) {
            argument >> variantMap;
            variant.setValue(variantMap);
        }

        return variant;
    }

    return QVariant();
}

void TagDBusHelper::onAddNewTags(const QDBusVariant &newTags)
{
    QStringList tags = newTags.variant().toStringList();
    emit addNewTags(tags);
}

void TagDBusHelper::onChangeTagColor(const QVariantMap &oldAndNewColor)
{
    QMap<QString, QString> tagAndNewColorName {};

    QMap<QString, QVariant>::const_iterator it = oldAndNewColor.begin();
    while (it != oldAndNewColor.end()) {
        tagAndNewColorName[it.key()] = it.value().toString();
        ++it;
    }

    emit tagColorChanged(tagAndNewColorName);
}

void TagDBusHelper::onChangeTagName(const QVariantMap &oldAndNewName)
{
    QMap<QString, QString> oldAndNewTagName {};

    QMap<QString, QVariant>::const_iterator it = oldAndNewName.begin();
    while (it != oldAndNewName.end()) {
        oldAndNewTagName[it.key()] = it.value().toString();
        ++it;
    }

    emit tagNameChanged(oldAndNewTagName);
}

void TagDBusHelper::onDeleteTags(const QDBusVariant &deletedTags)
{
    QStringList tags = deletedTags.variant().toStringList();
    emit tagsDeleted(tags);
}

void TagDBusHelper::onFilesWereTagged(const QVariantMap &taggedFiles)
{
    QMap<QString, QList<QString>> fileAndTags {};

    QMap<QString, QVariant>::const_iterator it = taggedFiles.begin();
    while (it != taggedFiles.end()) {
        fileAndTags[it.key()] = it.value().toStringList();
        ++it;
    }

    emit filesTagged(fileAndTags);
}

void TagDBusHelper::onUntagFiles(const QVariantMap &untaggedFiles)
{
    QMap<QString, QList<QString>> fileAndTags {};

    QMap<QString, QVariant>::const_iterator it = untaggedFiles.begin();
    while (it != untaggedFiles.end()) {
        fileAndTags[it.key()] = it.value().toStringList();
        ++it;
    }

    emit filesUntagged(fileAndTags);
}

void TagDBusHelper::initializeConnect()
{
    if (dbusInterface) {
        QObject::connect(dbusInterface.get(), &TagDBusInterface::addNewTags, this, &TagDBusHelper::onAddNewTags);
        QObject::connect(dbusInterface.get(), &TagDBusInterface::deleteTags, this, &TagDBusHelper::onDeleteTags);
        QObject::connect(dbusInterface.get(), &TagDBusInterface::changeTagColor, this, &TagDBusHelper::onChangeTagColor);
        QObject::connect(dbusInterface.get(), &TagDBusInterface::changeTagName, this, &TagDBusHelper::onChangeTagName);
        QObject::connect(dbusInterface.get(), &TagDBusInterface::filesWereTagged, this, &TagDBusHelper::onFilesWereTagged);
        QObject::connect(dbusInterface.get(), &TagDBusInterface::untagFiles, this, &TagDBusHelper::onUntagFiles);
    }
}
