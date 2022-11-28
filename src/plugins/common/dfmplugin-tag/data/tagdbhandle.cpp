// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagdbhandle.h"

#include "dfmplugin_tag_global.h"
#include "base/db/sqlitehandle.h"
#include "beans/filetaginfo.h"
#include "beans/tagproperty.h"

#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/finallyutil.h"

#include <dfm-io/dfmio_utils.h>

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QVariant>

DPTAG_USE_NAMESPACE
USING_IO_NAMESPACE

static constexpr char kTagDbName[] = ".__tag.db";

TagDbHandle *TagDbHandle::instance()
{
    static TagDbHandle ins;
    return &ins;
}

TagDbHandle::TagDbHandle(QObject *parent)
    : QObject(parent),
      handle(new SqliteHandle(DFMUtils::buildFilePath(StandardPaths::location(StandardPaths::kApplicationConfigPath).toLocal8Bit(),
                                                      "/deepin/dde-file-manager/database",
                                                      kTagDbName,
                                                      nullptr)))
{
}

QVariantMap TagDbHandle::getAllTags()
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });
    finally.dismiss();

    const auto &tagPropertyBean = handle->query<TagProperty>().toBeans();
    if (tagPropertyBean.isEmpty()) {
        return {};
    }

    QVariantMap tagPropertyMap;
    for (auto &bean : tagPropertyBean)
        tagPropertyMap.insert(bean->getTagName(), QVariant { bean->getTagColor() });

    return tagPropertyMap;
}

QVariantMap TagDbHandle::getTagsColor(const QStringList &tags)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (tags.isEmpty()) {
        lastErr = "input parameter is empty!";
        return {};
    }

    // query
    const auto &field = Expression::Field<TagProperty>;
    QVariantMap tagColorsMap;
    for (auto &tag : tags) {
        const auto &beanList = handle->query<TagProperty>().where(field("tagName") == tag).toBeans();
        const auto &color = beanList.isEmpty() ? QVariant() : beanList.first()->getTagColor();
        tagColorsMap.insert(tag, QVariant { QVariant { color } });
    }

    finally.dismiss();
    return tagColorsMap;
}

QVariantMap TagDbHandle::getTagsByUrls(const QStringList &urlList)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });
    if (urlList.isEmpty()) {
        lastErr = "input parameter is empty!";
        return {};
    }

    // query
    const auto &field = Expression::Field<FileTagInfo>;
    QVariantMap allFileTags;
    for (auto &path : urlList) {
        const auto &beanList = handle->query<FileTagInfo>().where(field("filePath") == path).toBeans();

        QStringList fileTags;
        for (auto oneBean : beanList)
            fileTags.append(oneBean->getTagName());

        allFileTags.insert(path, fileTags);
    }

    finally.dismiss();
    return allFileTags;
}

QVariant TagDbHandle::getSameTagsOfDiffUrls(const QStringList &urlList)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (urlList.isEmpty()) {
        lastErr = "input parameter is empty!";
        return {};
    }

    QMap<QString, int> tagCount;
    const auto &allTags = getTagsByUrls(urlList);
    QMap<QString, QVariant>::const_iterator it = allTags.begin();
    for (; it != allTags.end(); ++it) {
        const auto tempTags = it.value().toStringList();
        for (const auto &tagName : tempTags)
            ++tagCount[tagName];
    }

    int size { urlList.size() };
    QStringList sameTags;
    QMap<QString, int>::const_iterator ct = tagCount.begin();
    for (; ct != tagCount.end(); ++ct) {
        if (ct.value() == size)
            sameTags.append(ct.key());
    }

    finally.dismiss();
    return sameTags;
}

QVariantMap TagDbHandle::getFilesByTag(const QStringList &tags)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (tags.isEmpty()) {
        lastErr = "input parameter is empty!";
        return {};
    }

    // query
    const auto &field = Expression::Field<FileTagInfo>;
    QVariantMap allTagFiles;
    for (auto &tag : tags) {
        const auto &obj = handle->query<FileTagInfo>().where(field("tagName") == tag).toBeans();
        QStringList files;
        for (auto tempBean : obj)
            files.append(tempBean->getFilePath());

        allTagFiles.insert(tag, QVariant { files });
    }

    finally.dismiss();
    return allTagFiles;
}

bool TagDbHandle::addTagProperty(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    // insert tagProPerty
    QMap<QString, QVariant>::const_iterator it = data.begin();
    for (; it != data.end(); ++it) {
        if (!checkTag(it.key())) {
            if (!insertTagProperty(it.key(), it.value()))
                return false;
        }
    }

    emit addedNewTags(QVariant { data.keys() });
    finally.dismiss();
    return true;
}

bool TagDbHandle::addTagsForFiles(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    const auto &allTags = getTagsByUrls(data.keys());
    QMap<QString, QVariant>::const_iterator it = allTags.begin();
    QMap<QString, QStringList> allMutualTags;

    for (; it != allTags.end(); ++it)
        allMutualTags.insert(it.key(), it.value().toStringList());

    // insert file--tags
    bool ret = true;
    QMap<QString, QVariant>::const_iterator dataIt = data.begin();
    for (; dataIt != data.end(); ++dataIt) {
        if (allMutualTags.keys().contains(dataIt.key())) {
            const auto &tmpList = dataIt.value().toStringList();
            const auto &tempMutualTags = allMutualTags.value(dataIt.key());
            for (const auto &tp : tmpList) {
                if (!tempMutualTags.contains(tp)) {
                    ret = tagFile(dataIt.key(), dataIt.value());
                    if (!ret) {
                        return false;
                    }
                }
            }
        }
    }

    emit filesWereTagged(data);
    finally.dismiss();
    return true;
}

bool TagDbHandle::removeTagsOfFiles(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    // remove file--tags
    QMap<QString, QVariant>::const_iterator it = data.begin();
    for (; it != data.end(); ++it)
        if (!removeSpecifiedTagOfFile(it.key(), it.value()))
            return false;

    finally.dismiss();
    return true;
}

bool TagDbHandle::deleteTags(const QStringList &tags)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (tags.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    const auto &fieldOne = Expression::Field<TagProperty>;
    const auto &fieldTwo = Expression::Field<FileTagInfo>;

    bool ret = true;
    for (const auto &tag : tags) {
        ret = handle->remove<TagProperty>(fieldOne("tagName") == tag);
        if (!ret)
            return ret;
        ret = handle->remove<FileTagInfo>(fieldTwo("tagName") == tag);
        if (!ret)
            return ret;
    }

    emit deletedTags(tags);
    finally.dismiss();
    return ret;
}

bool TagDbHandle::deleteFiles(const QStringList &urls)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (urls.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    auto field = Expression::Field<FileTagInfo>;
    for (const auto &url : urls) {
        if (!handle->remove<FileTagInfo>(field("filePath") == url))
            return false;
    }

    finally.dismiss();
    return true;
}

bool TagDbHandle::changeTagColors(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    QMap<QString, QVariant>::const_iterator it = data.begin();
    bool ret = true;
    for (; it != data.end(); ++it) {
        ret = changeTagColor(it.key(), it.value().toString());
        if (!ret)
            return ret;
    }

    emit changedTagColor(data);
    finally.dismiss();
    return ret;
}

bool TagDbHandle::changeTagNames(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    QMap<QString, QVariant>::const_iterator it = data.begin();
    bool ret = true;
    for (; it != data.end(); ++it) {
        ret = changeTagName(it.key(), it.value().toString());
        if (!ret)
            return ret;
    }

    emit changedTagName(data);
    finally.dismiss();
    return ret;
}

bool TagDbHandle::changeFilePaths(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    QMap<QString, QVariant>::const_iterator it = data.begin();
    for (; it != data.end(); ++it)
        if (!changeFilePath(it.key(), it.value().toString()))
            return false;

    finally.dismiss();
    return true;
}

QString TagDbHandle::lastError()
{
    return lastErr;
}

bool TagDbHandle::checkTag(const QString &tag)
{
    return handle->query<TagProperty>().where(Expression::Field<TagProperty>("tagName") == tag).toBeans().size() > 0;
}

bool TagDbHandle::checkFile(const QString &file)
{
    // Query field 'filePath' of database table file_tags
    return handle->query<FileTagInfo>().where(Expression::Field<FileTagInfo>("filePath") == file).toBeans().size() > 0;
}

bool TagDbHandle::insertTagProperty(const QString &name, const QVariant &value)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (name.isEmpty() || value.isNull()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    // insert TagProperty
    TagProperty temp;
    temp.setTagName(name);
    temp.setTagColor(value.toString());
    temp.setFuture("null");
    temp.setAmbiguity(1);

    if (-1 == handle->insert<TagProperty>(temp)) {
        lastErr = QString("insert TagProperty failed! tagName: %1, tagValue: %2").arg(name).arg(value.toString());
        return false;
    }

    finally.dismiss();
    return true;
}

bool TagDbHandle::tagFile(const QString &file, const QVariant &tags)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (file.isEmpty() || tags.isNull()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    // insert file--tags
    const QStringList &tempTags = tags.toStringList();
    int suc = tempTags.count();
    for (const auto &tag : tempTags) {
        FileTagInfo temp;
        temp.setFilePath(file);
        temp.setTagName(tag);
        temp.setTagOrder(0);
        temp.setFuture("null");
        if (-1 == handle->insert<FileTagInfo>(temp))
            break;
        suc--;
    }

    if (0 < suc) {
        lastErr = QString("Tag file failed! file: %1, tagName: %2").arg(file).arg(tempTags.at(--suc));
        return false;
    }

    finally.dismiss();
    return true;
}

bool TagDbHandle::removeSpecifiedTagOfFile(const QString &url, const QVariant &val)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (url.isEmpty() || val.isNull()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    auto field = Expression::Field<FileTagInfo>;
    const auto tempTags = val.toStringList();
    int suc = tempTags.count();
    for (const auto &tag : tempTags) {
        if (!handle->remove<FileTagInfo>((field("filePath") == url) && (field("tagName") == tag)))
            break;
        suc--;
    }

    if (0 < suc) {
        lastErr = QString("Remove specified tag Of File failed! file: %1, tagName: %2").arg(url).arg(tempTags.at(--suc));
        return false;
    }

    finally.dismiss();
    return true;
}

bool TagDbHandle::changeTagColor(const QString &tagName, const QString &newTagColor)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (tagName.isEmpty() || newTagColor.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    const auto &field = Expression::Field<TagProperty>;
    if (!handle->update<TagProperty>(field("tagColor") = newTagColor, field("tagName") == tagName)) {
        lastErr = QString("Change tag Color failed! tagName: %1, newTagColor: %2").arg(tagName).arg(newTagColor);
        return false;
    }

    finally.dismiss();
    return true;
}

bool TagDbHandle::changeTagName(const QString &tagName, const QString &newName)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (tagName.isEmpty() || newName.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    const auto &field = Expression::Field<TagProperty>;
    if (!handle->update<TagProperty>(field("tagName") = newName, field("tagName") == tagName)) {
        lastErr = QString("Change tag name failed! tagName: %1, newName: %2").arg(tagName).arg(newName);
        return false;
    }

    finally.dismiss();
    return true;
}

bool TagDbHandle::changeFilePath(const QString &oldPath, const QString &newPath)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (oldPath.isEmpty() || newPath.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    const auto &field = Expression::Field<FileTagInfo>;
    if (!handle->update<TagProperty>(field("filePath") = newPath, field("filePath") == oldPath)) {
        lastErr = QString("Change file path failed! oldPath: %1, newPath: %2").arg(oldPath).arg(oldPath);
        return false;
    }

    finally.dismiss();
    return true;
}
