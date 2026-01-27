// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGMANAGERDBUS_H
#define TAGMANAGERDBUS_H

#include <QObject>
#include <QVariantMap>
#include <QDBusVariant>
#include <QDBusMessage>

class TagManagerDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.Daemon.TagManager")

public:
    explicit TagManagerDBus(QObject *parent = nullptr);

public Q_SLOTS:
    QDBusVariant Query(int opt, const QStringList value = {});
    bool Insert(int opt, const QVariantMap value);
    bool Delete(int opt, const QVariantMap value);
    bool Update(int opt, const QVariantMap value);

Q_SIGNALS:
    void TagsServiceReady();
    void NewTagsAdded(const QVariantMap &tags);
    void TagsDeleted(const QStringList &tags);
    void TagsColorChanged(const QVariantMap &oldAndNew);
    void TagsNameChanged(const QVariantMap &oldAndNew);
    void FilesTagged(const QVariantMap &fileAndTags);
    void FilesUntagged(const QVariantMap &fileAndTags);
    void TrashFileTagsChanged();

private:
    void initConnect();
};

#endif   // TAGMANAGERDBUS_H
