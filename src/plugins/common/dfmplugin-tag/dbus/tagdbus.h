// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGDBUS_H
#define TAGDBUS_H

#include <QObject>
#include <QVariantMap>
#include <QtDBus>

class TagDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.filemanager.server.Tag")

public:
    explicit TagDBus(QObject *parent = nullptr);
    ~TagDBus();

    Q_INVOKABLE QDBusVariant Query(const quint32 &type, const QStringList value = {});
    Q_INVOKABLE bool Insert(const quint32 &type, const QVariantMap value = QVariantMap());
    Q_INVOKABLE bool Delete(const quint32 &type, const QVariantMap value = QVariantMap());
    Q_INVOKABLE bool Update(const quint32 &type, const QVariantMap value = QVariantMap());
    Q_INVOKABLE bool CanTagFile(const QString &filePath);

    Q_INVOKABLE QString LastError();

private:
    QVariantMap getAllTags();
    QVariantMap getTagsThroughFile(const QStringList value);
    QVariant getSameTagsOfDiffFiles(const QStringList value);
    QVariantMap getFilesThroughTag(const QStringList value);
    QVariantMap getTagsColor(const QStringList value);
    bool changeTagColors(const QVariantMap value);
    bool changeTagNamesWithFiles(const QVariantMap value);
    bool changeFilePaths(const QVariantMap value);
    bool deleteTags(const QVariantMap value);
    bool deleteFiles(const QVariantMap value);
    bool deleteFileTags(const QVariantMap value);

    void initconnect();

Q_SIGNALS:
    void NewTagsAdded(const QVariantMap &tags);
    void TagsDeleted(const QStringList &tags);
    void TagsColorChanged(const QVariantMap &oldAndNew);
    void TagsNameChanged(const QVariantMap &oldAndNew);
    void FilesTagged(const QVariantMap &fileAndTags);
    void FilesUntagged(const QVariantMap &fileAndTags);

private:
    QString lastErr;
};

#endif   // TAGDBUS_H
