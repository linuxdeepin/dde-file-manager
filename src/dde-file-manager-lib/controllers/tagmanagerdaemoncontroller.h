/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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

#ifndef TAGMANAGERDAEMONCONTROLLER_H
#define TAGMANAGERDAEMONCONTROLLER_H

#include <memory>


#include "tag/tagutil.h"
#include "./interface/tagmanagerdaemon_interface.h"

#include <QObject>
#include <QVariantMap>

class TagManagerDaemonController final : public QObject
{
    Q_OBJECT
public:
    virtual ~TagManagerDaemonController()=default;
    explicit TagManagerDaemonController(QObject* const parent = nullptr);
    TagManagerDaemonController(const TagManagerDaemonController& other)=delete;
    TagManagerDaemonController& operator=(const TagManagerDaemonController& other)=delete;

    QVariant disposeClientData(const QVariantMap& filesAndTags, Tag::ActionType type);

    static TagManagerDaemonController* instance();

signals:
    void addNewTags(const QVariant& new_tags);
    void changeTagColor(const QVariantMap& old_and_new_color);
    void changeTagName(const QVariantMap& old_and_new_name);
    void deleteTags(const QVariant& be_deleted_tags);
    void filesWereTagged(const QVariantMap& files_were_tagged);
    void untagFiles(const QVariantMap& tag_be_removed_files);


private slots:
    void onAddNewTags(const QDBusVariant &new_tags);
    void onChangeTagColor(const QVariantMap& old_and_new_color);
    void onChangeTagName(const QVariantMap& old_and_new_name);
    void onDeleteTags(const QDBusVariant& be_deleted_tags);
    void onFilesWereTagged(const QVariantMap& files_were_tagged);
    void onUntagFiles(const QVariantMap& tag_be_removed_files);

private:

    void init_connect()noexcept;

    std::unique_ptr<TagManagerDaemonInterface> m_daemonInterface{ nullptr };
};

#endif // TAGMANAGERDAEMONCONTROLLER_H
