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
#ifndef TAGDATAMANAGERDBUS_H
#define TAGDATAMANAGERDBUS_H

#include <QMap>
#include <QList>
#include <QObject>
#include <QDBusVariant>

class TagManagerDaemonAdaptor;
class TagDataManagerDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.daemon.TagManagerDaemon")
public:
    explicit TagDataManagerDBus(QObject *const parent);
    virtual ~TagDataManagerDBus() = default;
    TagDataManagerDBus(const TagDataManagerDBus &other) = delete;
    TagDataManagerDBus &operator=(const TagDataManagerDBus &other) = delete;

    Q_INVOKABLE QDBusVariant disposeClientData(const QMap<QString, QVariant> &filesAndTags, const unsigned long long &type);

signals:
    void addNewTags(const QDBusVariant &new_tags);
    void deleteTags(const QDBusVariant &be_deleted_tags);
    void changeTagColor(const QVariantMap &old_and_new_color);
    void changeTagName(const QVariantMap &old_and_new_name);
    void filesWereTagged(const QVariantMap &files_were_tagged);
    void untagFiles(const QVariantMap &tag_beg_removed_files);

private slots:
    void onAddNewTags(const QVariant &new_tags) noexcept;
    void onDeleteTags(const QVariant &be_deleted_tags) noexcept;
    void onFileWereTagged(const QVariantMap &files_were_tagged) noexcept;
    void onUntagFiles(const QVariantMap &tag_beg_removed_files) noexcept;
    void onChangeTagColor(const QVariantMap &old_and_new_color) noexcept;
    void onChangeTagName(const QVariantMap &old_and_new_name) noexcept;

private:
    void init_connection() noexcept;

    TagManagerDaemonAdaptor *adaptor { nullptr };
};

#endif   // TAGDATAMANAGERDBUS_H
