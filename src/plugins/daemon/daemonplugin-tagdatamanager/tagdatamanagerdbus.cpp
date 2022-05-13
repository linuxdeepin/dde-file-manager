/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#include "tagdatamanagerdbus.h"
#include "dbusadaptor/tagdaemon_adaptor.h"
#include "util.h"
#include "dsqlitehandle.h"

static constexpr const char *kTagDaemonObjPath { "/com/deepin/filemanager/daemon/TagManagerDaemon" };

TagDataManagerDBus::TagDataManagerDBus(QObject *const parent)
    : QObject { parent },
      adaptor { new TagManagerDaemonAdaptor { this } }
{
    init_connection();

    if (!QDBusConnection::systemBus().registerObject(kTagDaemonObjPath, this)) {
        qFatal("Failed to register object.");   //###: log!
    }
}

QDBusVariant TagDataManagerDBus::disposeClientData(const QMap<QString, QVariant> &filesAndTags, const unsigned long long &type)
{
    QDBusVariant dbusVar {};

    if (!filesAndTags.isEmpty()) {
        QMap<QString, QVariant>::const_iterator cbeg { filesAndTags.cbegin() };
        QMap<QString, QVariant>::const_iterator cend { filesAndTags.cend() };
        QMap<QString, QList<QString>> filesAndTagsName {};

        for (; cbeg != cend; ++cbeg) {
            QString key { DAEMONPTAGDM_NAMESPACE::escapingEnSkim(cbeg.key()) };
            QList<QString> values {};

            auto lst = cbeg.value().toStringList();
            std::transform(lst.begin(), lst.end(), std::back_inserter(values), [](const QString &qstr) {
                return DAEMONPTAGDM_NAMESPACE::escapingEnSkim(qstr);
            });

            filesAndTagsName[key] = values;
        }

        QVariant var { DSqliteHandle::instance()->disposeClientData(filesAndTagsName, type) };
        dbusVar.setVariant(var);
    }

    return dbusVar;
}

void TagDataManagerDBus::onAddNewTags(const QVariant &new_tags) noexcept
{
    emit addNewTags(QDBusVariant { new_tags });
}

void TagDataManagerDBus::onDeleteTags(const QVariant &be_deleted_tags) noexcept
{
    emit deleteTags(QDBusVariant { be_deleted_tags });
}

void TagDataManagerDBus::onFileWereTagged(const QVariantMap &files_were_tagged) noexcept
{
    emit filesWereTagged(files_were_tagged);
}

void TagDataManagerDBus::onUntagFiles(const QVariantMap &tag_beg_removed_files) noexcept
{
    emit untagFiles(tag_beg_removed_files);
}

void TagDataManagerDBus::onChangeTagColor(const QVariantMap &old_and_new_color) noexcept
{
    emit changeTagColor(old_and_new_color);
}

void TagDataManagerDBus::onChangeTagName(const QVariantMap &old_and_new_name) noexcept
{
    emit changeTagName(old_and_new_name);
}

void TagDataManagerDBus::init_connection() noexcept
{
    if (adaptor) {
        QObject::connect(DSqliteHandle::instance(), &DSqliteHandle::addNewTags, this, &TagDataManagerDBus::onAddNewTags);
        QObject::connect(DSqliteHandle::instance(), &DSqliteHandle::deleteTags, this, &TagDataManagerDBus::onDeleteTags);
        QObject::connect(DSqliteHandle::instance(), &DSqliteHandle::changeTagColor, this, &TagDataManagerDBus::onChangeTagColor);
        QObject::connect(DSqliteHandle::instance(), &DSqliteHandle::changeTagName, this, &TagDataManagerDBus::onChangeTagName);
        QObject::connect(DSqliteHandle::instance(), &DSqliteHandle::filesWereTagged, this, &TagDataManagerDBus::onFileWereTagged);
        QObject::connect(DSqliteHandle::instance(), &DSqliteHandle::untagFiles, this, &TagDataManagerDBus::onUntagFiles);
    }
}
