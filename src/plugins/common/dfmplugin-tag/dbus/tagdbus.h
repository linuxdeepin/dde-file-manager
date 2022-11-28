// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
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
public:
    explicit TagDBus(QObject *parent = nullptr);
    ~TagDBus();
    Q_INVOKABLE QVariant Query(const quint8 &type, const QStringList value = {});
    Q_INVOKABLE bool Insert(const quint8 &type, const QVariantMap value = QVariantMap());
    Q_INVOKABLE bool Delete(const quint8 &type, const QVariantMap value = QVariantMap());
    Q_INVOKABLE bool Update(const quint8 &type, const QVariantMap value = QVariantMap());
    Q_INVOKABLE bool CanTagFile(const QString &filePath);

    Q_INVOKABLE QString LastError();

private:
    QVariantMap getAllTags();
    QVariantMap getTagsThroughFile(const QStringList value);
    QVariant getSameTagsOfDiffFiles(const QStringList value);
    QVariantMap getFilesThroughTag(const QStringList value);
    QVariantMap getTagsColor(const QStringList value);
    bool changeTagColor(const QVariantMap value);
    bool changeTagName(const QVariantMap value);
    bool changeFilesName(const QVariantMap value);
    bool deleteTags(const QVariantMap value);
    bool deleteFiles(const QVariantMap value);
    bool deleteFileTags(const QVariantMap value);

    void initconnect();

Q_SIGNALS:
    void addedNewTags(const QStringList &tags);
    void tagsDeleted(const QStringList &tags);
    void tagColorChanged(const QMap<QString, QString> &oldAndNew);
    void tagNameChanged(const QMap<QString, QString> &oldAndNew);
    void filesTagged(const QMap<QString, QList<QString>> &fileAndTags);
    void filesUntagged(const QMap<QString, QList<QString>> &fileAndTags);

private Q_SLOTS:
    void onAddNewTags(const QVariant &newTags);
    void onChangeTagColor(const QVariantMap &oldAndNewColor);
    void onChangeTagName(const QVariantMap &oldAndNewName);
    void onDeleteTags(const QVariant &deletedTags);
    void onFilesWereTagged(const QVariantMap &taggedFiles);
    void onUntagFiles(const QVariantMap &untaggedFiles);

private:
    QString lastErr;
};

#endif   // TAGDBUS_H
