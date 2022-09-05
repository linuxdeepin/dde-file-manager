// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGMANAGERDAEMON_H
#define TAGMANAGERDAEMON_H

#include <memory>

#include <QMap>
#include <QList>
#include <QObject>


#include "deviceinfo/udisklistener.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "shutil/danythingmonitorfilter.h"

class TagManagerDaemonAdaptor;
class TagManagerDaemon : public QObject
{
    Q_OBJECT
public:
    explicit TagManagerDaemon(QObject *const parent);
    virtual ~TagManagerDaemon() = default;
    TagManagerDaemon(const TagManagerDaemon &other) = delete;
    TagManagerDaemon &operator=(const TagManagerDaemon &other) = delete;

    Q_INVOKABLE QDBusVariant disposeClientData(const QMap<QString, QVariant> &filesAndTags, const unsigned long long &type);

signals:
    void addNewTags(const QDBusVariant &new_tags);
    void deleteTags(const QDBusVariant &be_deleted_tags);
    void changeTagColor(const QVariantMap &old_and_new_color);
    void changeTagName(const QVariantMap &old_and_new_name);
    void filesWereTagged(const QVariantMap &files_were_tagged);
    void untagFiles(const QVariantMap &tag_beg_removed_files);



private slots:
    void onAddNewTags(const QVariant &new_tags)noexcept;
    void onDeleteTags(const QVariant &be_deleted_tags)noexcept;
    void onFileWereTagged(const QVariantMap &files_were_tagged)noexcept;
    void onUntagFiles(const QVariantMap &tag_beg_removed_files)noexcept;
    void onChangeTagColor(const QVariantMap &old_and_new_color)noexcept;
    void onChangeTagName(const QVariantMap &old_and_new_name)noexcept;

private:
    void init_connection()noexcept;

    TagManagerDaemonAdaptor *adaptor{ nullptr };
    DAnythingMonitorFilter *m_anything_monitor{ nullptr };
};
#endif // TAGMANAGERDAEMON_H
