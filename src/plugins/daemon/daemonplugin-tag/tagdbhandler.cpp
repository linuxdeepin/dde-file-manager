// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagdbhandler.h"

#include "beans/filetaginfo.h"
#include "beans/tagproperty.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/finallyutil.h>
#include <dfm-base/base/db/sqlitehandle.h>
#include <dfm-base/base/db/sqlitehelper.h>

#include <dfm-io/dfmio_utils.h>

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QVariant>

DFMBASE_USE_NAMESPACE
DAEMONPTAG_BEGIN_NAMESPACE

static constexpr char kTagTableFileTags[] = "file_tags";
static constexpr char kTagTableTagProperty[] = "tag_property";

TagDbHandler *TagDbHandler::instance()
{
    static TagDbHandler ins;
    return &ins;
}

QVariantMap TagDbHandler::getAllTags()
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

QVariantMap TagDbHandler::getTagsColor(const QStringList &tags)
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
        const auto &color = beanList.isEmpty() ? "" : beanList.first()->getTagColor();
        if (!color.isEmpty())
            tagColorsMap.insert(tag, QVariant { QVariant { color } });
    }

    finally.dismiss();
    return tagColorsMap;
}

QVariantMap TagDbHandler::getTagsByUrls(const QStringList &urlList)
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

        if (!fileTags.isEmpty())
            allFileTags.insert(path, fileTags);
    }

    finally.dismiss();
    return allFileTags;
}

QVariant TagDbHandler::getSameTagsOfDiffUrls(const QStringList &urlList)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (urlList.isEmpty()) {
        lastErr = "input parameter is empty!";
        return {};
    }

    QMap<QString, int> tagCount;
    const auto &allTags = getTagsByUrls(urlList);
    auto it = allTags.begin();
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

QVariantMap TagDbHandler::getFilesByTag(const QStringList &tags)
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

QVariantHash TagDbHandler::getAllFileWithTags()
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });
    finally.dismiss();

    // query
    const auto &beans = handle->query<FileTagInfo>().toBeans();

    QVariantHash fileTagsMap;
    for (auto &bean : beans) {
        const auto &path = bean->getFilePath();
        if (fileTagsMap.contains(path)) {
            QStringList list { fileTagsMap[path].toStringList() };
            const QString &tagName { bean->getTagName() };
            if (list.contains(tagName))
                continue;
            list.append(bean->getTagName());
            fileTagsMap[path] = list;
        } else {
            fileTagsMap.insert(path, { bean->getTagName() });
        }
    }

    return fileTagsMap;
}

bool TagDbHandler::addTagProperty(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    // insert tagProPerty
    auto it = data.begin();
    for (; it != data.end(); ++it) {
        if (!checkTag(it.key())) {
            if (!insertTagProperty(it.key(), it.value()))
                return false;
        }
    }

    emit newTagsAdded(data);
    finally.dismiss();
    return true;
}

bool TagDbHandler::addTagsForFiles(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }
    //Remove duplicate data from the database
    const QVariantMap &dbData = getTagsByUrls(data.keys());
    QVariantMap tmpData = data;
    for (auto dataIt = data.begin(); dataIt != data.end(); ++dataIt) {
        if (dbData.contains(dataIt.key())) {
            //use Qset() to get difference set ,tmpData = data - dbData
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QStringList tags = dataIt.value()
                                       .toStringList()
                                       .toSet()
                                       .subtract(dbData.value(dataIt.key()).toStringList().toSet())
                                       .toList();
#else
            QStringList dataItList { dataIt.value().toStringList() };
            QSet<QString> dataItSet { dataItList.begin(), dataItList.end() };
            QStringList dbDataList { dbData.value(dataIt.key()).toStringList() };
            QSet<QString> dbDataSet { dbDataList.begin(), dbDataList.end() };
            QStringList tags { dataItSet.subtract(dbDataSet).values() };
#endif
            tmpData[dataIt.key()] = tags;
        }
    }

    // insert file--tags
    bool ret = handle->transaction([tmpData, this]() -> bool {
        for (auto dataIt = tmpData.begin(); dataIt != tmpData.end(); ++dataIt) {
            bool ret = tagFile(dataIt.key(), dataIt.value());
            if (!ret)
                return ret;
        }
        return true;
    });

    emit filesWereTagged(data);
    finally.dismiss();
    return ret;
}

bool TagDbHandler::removeTagsOfFiles(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    // remove file--tags

    bool ret = handle->transaction([data, this]() -> bool {
        for (auto it = data.begin(); it != data.end(); ++it)
            if (!removeSpecifiedTagOfFile(it.key(), it.value()))
                return false;
        return true;
    });

    emit filesUntagged(data);
    finally.dismiss();
    return ret;
}

bool TagDbHandler::deleteTags(const QStringList &tags)
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

    emit tagsDeleted(tags);
    finally.dismiss();
    return ret;
}

bool TagDbHandler::deleteFiles(const QStringList &urls)
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

bool TagDbHandler::changeTagColors(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    auto it = data.begin();
    bool ret = true;
    for (; it != data.end(); ++it) {
        ret = changeTagColor(it.key(), it.value().toString());
        if (!ret)
            return ret;
    }

    emit tagsColorChanged(data);

    finally.dismiss();
    return ret;
}

bool TagDbHandler::changeTagNamesWithFiles(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    auto it = data.begin();
    QVariantMap updatedData;
    bool ret = true;
    for (; it != data.end(); ++it) {
        if (changeTagNameWithFile(it.key(), it.value().toString()))
            updatedData.insert(it.key(), it.value());
        else
            ret = false;
    }

    if (!updatedData.isEmpty())
        emit tagsNameChanged(updatedData);

    if (ret)
        finally.dismiss();

    return ret;
}

bool TagDbHandler::changeFilePaths(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    auto it = data.begin();
    for (; it != data.end(); ++it)
        if (!changeFilePath(it.key(), it.value().toString()))
            return false;

    finally.dismiss();
    return true;
}

QString TagDbHandler::lastError() const
{
    return lastErr;
}

TagDbHandler::TagDbHandler(QObject *parent)
    : QObject(parent)
{
    initialize();
}

void TagDbHandler::initialize()
{
    using namespace dfmio;
    const auto &dbPath = DFMUtils::buildFilePath(StandardPaths::location(StandardPaths::kApplicationConfigPath).toLocal8Bit(),
                                                 "/deepin/dde-file-manager/database",
                                                 nullptr);

    QDir dir(dbPath);
    if (!dir.exists())
        dir.mkpath(dbPath);

    const auto &dbFilePath = DFMUtils::buildFilePath(dbPath.toLocal8Bit(),
                                                     Global::DataBase::kDfmDBName,
                                                     nullptr);
    handle.reset(new SqliteHandle(dbFilePath));
    QSqlDatabase db { SqliteConnectionPool::instance().openConnection(dbFilePath) };
    if (!db.isValid() || db.isOpenError()) {
        fmWarning() << "The tag database is invalid! open error";
        return;
    }
    db.close();

    if (!createTable(kTagTableFileTags))
        fmWarning() << "Create table failed:" << kTagTableFileTags;

    if (!createTable(kTagTableTagProperty))
        fmWarning() << "Create table failed:" << kTagTableFileTags;
}

bool TagDbHandler::createTable(const QString &tableName)
{
    bool ret = false;
    if (SqliteHelper::tableName<FileTagInfo>() == tableName) {

        ret = handle->createTable<FileTagInfo>(
                SqliteConstraint::primary("fileIndex"),
                SqliteConstraint::autoIncreament("fileIndex"),
                SqliteConstraint::unique("fileIndex"));
    }

    if (SqliteHelper::tableName<TagProperty>() == tableName) {

        ret = handle->createTable<TagProperty>(
                SqliteConstraint::primary("tagIndex"),
                SqliteConstraint::autoIncreament("tagIndex"),
                SqliteConstraint::unique("tagIndex"));
    }

    return ret;
}

bool TagDbHandler::checkTag(const QString &tag)
{
    return handle->query<TagProperty>().where(Expression::Field<TagProperty>("tagName") == tag).toBeans().size() > 0;
}

bool TagDbHandler::insertTagProperty(const QString &name, const QVariant &value)
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

bool TagDbHandler::tagFile(const QString &file, const QVariant &tags)
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

bool TagDbHandler::removeSpecifiedTagOfFile(const QString &url, const QVariant &val)
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

bool TagDbHandler::changeTagColor(const QString &tagName, const QString &newTagColor)
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

bool TagDbHandler::changeTagNameWithFile(const QString &tagName, const QString &newName)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (tagName.isEmpty() || newName.isEmpty()) {
        lastErr = "input parameter is empty!";
        return false;
    }

    // update tagname and files tagname
    bool ret = handle->transaction([tagName, newName, this]() -> bool {
        if (!handle->update<TagProperty>(Expression::Field<TagProperty>("tagName") = newName,
                                         Expression::Field<TagProperty>("tagName") == tagName)) {
            lastErr = QString("Change tag name failed! tagName: %1, newName: %2").arg(tagName).arg(newName);
            return false;
        }
        if (!handle->update<FileTagInfo>(Expression::Field<FileTagInfo>("tagName") = newName,
                                         Expression::Field<FileTagInfo>("tagName") == tagName)) {
            lastErr = QString("Change file tag name failed! tagName: %1, newName: %2").arg(tagName).arg(newName);
            return false;
        }

        return true;
    });

    if (ret)
        finally.dismiss();

    return ret;
}

bool TagDbHandler::changeFilePath(const QString &oldPath, const QString &newPath)
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

DAEMONPTAG_END_NAMESPACE
