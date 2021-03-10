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

#include <mutex>

#include "tagmanagerdaemoncontroller.h"

#include <QDBusVariant>

static constexpr const  char *const service{ "com.deepin.filemanager.daemon" };
static constexpr const char *const path{ "/com/deepin/filemanager/daemon/TagManagerDaemon" };

std::atomic<bool> flagForLockingBackend{ false };

TagManagerDaemonController::TagManagerDaemonController(QObject *const parent)
    : QObject{ parent }
    , m_daemonInterface{ nullptr}
{
    m_daemonInterface = std::unique_ptr<TagManagerDaemonInterface> { new TagManagerDaemonInterface {
            service,
            path,
            QDBusConnection::systemBus()
        }
    };

    // blumia: since it's associated with context menu so we need to set a shorter timeout time.
    m_daemonInterface->setTimeout(3000);

    this->init_connect();
}

TagManagerDaemonController *TagManagerDaemonController::instance()
{
    static TagManagerDaemonController *the_instance{ new TagManagerDaemonController };
    return the_instance;
}

void TagManagerDaemonController::onAddNewTags(const QDBusVariant &new_tags)
{
    emit addNewTags(new_tags.variant());
}

void TagManagerDaemonController::onChangeTagColor(const QVariantMap &old_and_new_color)
{
    emit changeTagColor(old_and_new_color);
}

void TagManagerDaemonController::onChangeTagName(const QVariantMap &old_and_new_name)
{
    emit changeTagName(old_and_new_name);
}

void TagManagerDaemonController::onDeleteTags(const QDBusVariant &be_deleted_tags)
{
    emit deleteTags(be_deleted_tags.variant());
}

void TagManagerDaemonController::onFilesWereTagged(const QVariantMap &files_were_tagged)
{
    emit filesWereTagged(files_were_tagged);
}

void TagManagerDaemonController::onUntagFiles(const QVariantMap &tag_be_removed_files)
{
    emit untagFiles(tag_be_removed_files);
}

void TagManagerDaemonController::init_connect()noexcept
{
    if (m_daemonInterface) {
        QObject::connect(m_daemonInterface.get(), &TagManagerDaemonInterface::addNewTags, this, &TagManagerDaemonController::onAddNewTags);
        QObject::connect(m_daemonInterface.get(), &TagManagerDaemonInterface::deleteTags, this, &TagManagerDaemonController::onDeleteTags);
        QObject::connect(m_daemonInterface.get(), &TagManagerDaemonInterface::changeTagColor, this, &TagManagerDaemonController::onChangeTagColor);
        QObject::connect(m_daemonInterface.get(), &TagManagerDaemonInterface::changeTagName, this, &TagManagerDaemonController::onChangeTagName);
        QObject::connect(m_daemonInterface.get(), &TagManagerDaemonInterface::filesWereTagged, this, &TagManagerDaemonController::onFilesWereTagged);
        QObject::connect(m_daemonInterface.get(), &TagManagerDaemonInterface::untagFiles, this, &TagManagerDaemonController::onUntagFiles);
    }
}

QVariant TagManagerDaemonController::disposeClientData(const QVariantMap &filesAndTags, Tag::ActionType type)
{
    QDBusVariant var{ m_daemonInterface->disposeClientData(filesAndTags, static_cast<unsigned long long>(type)) };
    QVariant variant{ var.variant() };
    QDBusArgument argument{ variant.value<QDBusArgument>() };
    QDBusArgument::ElementType current_type{ argument.currentType() };
    QMap<QString, QVariant> var_map{};

    if (current_type == QDBusArgument::ElementType::MapType) {
        argument >> var_map;
        variant.setValue(var_map);
    }

    return variant;
}
