// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/define.h"
#include "tag/tagutil.h"
#include "tagmanagerdaemon.h"
#include "shutil/dsqlitehandle.h"
#include "dbusadaptor/tagmanagerdaemon_adaptor.h"

#include <QDebug>


static constexpr const char *ObjectPath{"/com/deepin/filemanager/daemon/TagManagerDaemon"};

std::atomic<int> counter{ 0 };


TagManagerDaemon::TagManagerDaemon(QObject *const parent)
    : QObject{ parent },
      adaptor{ new TagManagerDaemonAdaptor{ this } },
m_anything_monitor{ new DAnythingMonitorFilter{this} }
{

    this->init_connection();

    if (!QDBusConnection::systemBus().registerObject(ObjectPath, this)) {
        qFatal("Failed to register object."); //###: log!
    }
}

QDBusVariant TagManagerDaemon::disposeClientData(const QMap<QString, QVariant> &filesAndTags, const unsigned long long &type)
{
    QDBusVariant dbusVar{};

    if (!filesAndTags.isEmpty()) {
        QMap<QString, QVariant>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QVariant>::const_iterator cend{ filesAndTags.cend() };
        QMap<QString, QList<QString>> filesAndTagsName{};

        for (; cbeg != cend; ++cbeg) {
            QString key{ Tag::escaping_en_skim(cbeg.key()) };
            QList<QString> values{};

            auto lst = cbeg.value().toStringList();
            std::transform(lst.begin(), lst.end(), std::back_inserter(values), [](const QString & qstr) {
                return Tag::escaping_en_skim(qstr);
            });

            filesAndTagsName[key] = values;
        }

        QVariant var{ DSqliteHandle::instance()->disposeClientData(filesAndTagsName, type) };
        dbusVar.setVariant(var);
    }

    return dbusVar;
}

void TagManagerDaemon::onAddNewTags(const QVariant &new_tags)noexcept
{
    emit addNewTags(QDBusVariant{new_tags});
}

void TagManagerDaemon::onDeleteTags(const QVariant &be_deleted_tags)noexcept
{
    emit deleteTags(QDBusVariant{be_deleted_tags});
}

void TagManagerDaemon::onFileWereTagged(const QVariantMap &files_were_tagged)noexcept
{
    emit filesWereTagged(files_were_tagged);
}

void TagManagerDaemon::onUntagFiles(const QVariantMap &tag_beg_removed_files)noexcept
{
    emit untagFiles(tag_beg_removed_files);
}

void TagManagerDaemon::onChangeTagColor(const QVariantMap &old_and_new_color)noexcept
{
    emit changeTagColor(old_and_new_color);
}

void TagManagerDaemon::onChangeTagName(const QVariantMap &old_and_new_name)noexcept
{
    emit changeTagName(old_and_new_name);
}


void TagManagerDaemon::init_connection()noexcept
{
    if (adaptor) {
        QObject::connect(DSqliteHandle::instance(), &DSqliteHandle::addNewTags, this, &TagManagerDaemon::onAddNewTags);
        QObject::connect(DSqliteHandle::instance(), &DSqliteHandle::deleteTags, this, &TagManagerDaemon::onDeleteTags);
        QObject::connect(DSqliteHandle::instance(), &DSqliteHandle::changeTagColor, this, &TagManagerDaemon::onChangeTagColor);
        QObject::connect(DSqliteHandle::instance(), &DSqliteHandle::changeTagName, this, &TagManagerDaemon::onChangeTagName);
        QObject::connect(DSqliteHandle::instance(), &DSqliteHandle::filesWereTagged, this, &TagManagerDaemon::onFileWereTagged);
        QObject::connect(DSqliteHandle::instance(), &DSqliteHandle::untagFiles, this, &TagManagerDaemon::onUntagFiles);
    }
}
