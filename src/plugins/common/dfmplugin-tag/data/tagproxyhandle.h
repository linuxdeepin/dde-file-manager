// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGPROXYHANDLE_H
#define TAGPROXYHANDLE_H

#include "dfmplugin_tag_global.h"

#define TagProxyHandleIns DPTAG_NAMESPACE::TagProxyHandle::instance()

namespace dfmplugin_tag {

class TagProxyHandlePrivate;
class TagProxyHandle : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TagProxyHandle)

public:
    static TagProxyHandle *instance();

    QVariantMap getAllTags();
    QVariantMap getTagsThroughFile(const QStringList &value);
    QVariant getSameTagsOfDiffFiles(const QStringList &value);
    QVariantMap getFilesThroughTag(const QStringList &value);
    QVariantMap getTagsColor(const QStringList &value);
    QVariantHash getAllFileWithTags();

    bool addTags(const QVariantMap &value);
    bool addTagsForFiles(const QVariantMap &value);

    bool changeTagsColor(const QVariantMap &value);
    bool changeTagNamesWithFiles(const QVariantMap &value);
    bool changeFilePaths(const QVariantMap &value);

    bool deleteTags(const QVariantMap &value);
    bool deleteFiles(const QVariantMap &value);
    bool deleteFileTags(const QVariantMap &value);

    bool saveTrashFileTags(const QString &originalPath, qint64 inode, const QStringList &tags);
    QStringList getTrashFileTags(const QString &originalPath, qint64 inode);
    bool removeTrashFileTags(const QString &originalPath, qint64 inode);
    bool clearAllTrashTags();
    QVariantHash getAllTrashFileTags();

    bool connectToService();

    bool isValid();
Q_SIGNALS:
    void filesTagged(const QVariantMap &fileAndTags);
    void filesUntagged(const QVariantMap &fileAndTags);
    void newTagsAdded(const QVariantMap &tags);
    void tagsColorChanged(const QVariantMap &oldAndNew);
    void tagsDeleted(const QStringList &tags);
    void tagsNameChanged(const QVariantMap &oldAndNew);
    void trashFileTagsChanged();
    void tagServiceRegistered();

private:
    explicit TagProxyHandle(QObject *parent = nullptr);
    ~TagProxyHandle() = default;

    QScopedPointer<TagProxyHandlePrivate> d;
};

}

#endif   // TAGPROXYHANDLE_H
