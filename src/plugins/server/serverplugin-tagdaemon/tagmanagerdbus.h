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
    Q_CLASSINFO("D-Bus Interface", "org.deepin.filemanager.server.TagManager")

public:
    explicit TagManagerDBus(QObject *parent = nullptr);

public Q_SLOTS:
    QDBusVariant Query(const quint32 &type, const QStringList value = {});
    bool Insert(const quint32 &type, const QVariantMap value = QVariantMap());
    bool Delete(const quint32 &type, const QVariantMap value = QVariantMap());
    bool Update(const quint32 &type, const QVariantMap value = QVariantMap());
    bool CanTagFile(const QString &filePath);

Q_SIGNALS:
    void NewTagsAdded(const QVariantMap &tags);
    void TagsDeleted(const QStringList &tags);
    void TagsColorChanged(const QVariantMap &oldAndNew);
    void TagsNameChanged(const QVariantMap &oldAndNew);
    void FilesTagged(const QVariantMap &fileAndTags);
    void FilesUntagged(const QVariantMap &fileAndTags);
};

#endif   // TAGMANAGERDBUS_H
