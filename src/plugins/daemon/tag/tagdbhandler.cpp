// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagdbhandler.h"

#include "beans/filetaginfo.h"
#include "beans/tagproperty.h"
#include "beans/trashfiletaginfo.h"

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
static constexpr char kTagTableTrashFileTags[] = "trash_file_tags";

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
        fmDebug() << "TagDbHandler::getAllTags: No tags found in database";
        return {};
    }

    QVariantMap tagPropertyMap;
    for (auto &bean : tagPropertyBean)
        tagPropertyMap.insert(bean->getTagName(), QVariant { bean->getTagColor() });

    fmDebug() << "TagDbHandler::getAllTags: Retrieved" << tagPropertyMap.size() << "tags from database";
    return tagPropertyMap;
}

QVariantMap TagDbHandler::getTagsColor(const QStringList &tags)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (tags.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::getTagsColor: Empty tag list provided";
        finally.dismiss();
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

    fmDebug() << "TagDbHandler::getTagsColor: Retrieved colors for" << tagColorsMap.size() << "out of" << tags.size() << "requested tags";
    return tagColorsMap;
}

QVariantMap TagDbHandler::getTagsByUrls(const QStringList &urlList)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });
    if (urlList.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::getTagsByUrls: Empty URL list provided";
        finally.dismiss();
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

    fmDebug() << "TagDbHandler::getTagsByUrls: Retrieved tags for" << allFileTags.size() << "out of" << urlList.size() << "requested files";
    return allFileTags;
}

QVariant TagDbHandler::getSameTagsOfDiffUrls(const QStringList &urlList)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (urlList.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::getSameTagsOfDiffUrls: Empty URL list provided";
        finally.dismiss();
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

    fmDebug() << "TagDbHandler::getSameTagsOfDiffUrls: Found" << sameTags.size() << "common tags among" << urlList.size() << "files";
    return sameTags;
}

QVariantMap TagDbHandler::getFilesByTag(const QStringList &tags)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (tags.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::getFilesByTag: Empty tag list provided";
        finally.dismiss();
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

    fmDebug() << "TagDbHandler::getFilesByTag: Retrieved files for" << tags.size() << "tags";
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

    fmDebug() << "TagDbHandler::getAllFileWithTags: Retrieved" << fileTagsMap.size() << "files with tags";
    return fileTagsMap;
}

bool TagDbHandler::addTagProperty(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::addTagProperty: Empty data provided";
        finally.dismiss();
        return false;
    }

    fmInfo() << "TagDbHandler::addTagProperty: Adding" << data.size() << "tag properties";

    // insert tagProPerty
    auto it = data.begin();
    for (; it != data.end(); ++it) {
        if (!checkTag(it.key())) {
            if (!insertTagProperty(it.key(), it.value())) {
                fmCritical() << "TagDbHandler::addTagProperty: Failed to insert tag property for tag:" << it.key();
                return false;
            }
        } else {
            fmDebug() << "TagDbHandler::addTagProperty: Tag already exists, skipping:" << it.key();
        }
    }

    emit newTagsAdded(data);
    fmInfo() << "TagDbHandler::addTagProperty: Successfully added tag properties";
    return true;
}

bool TagDbHandler::addTagsForFiles(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::addTagsForFiles: Empty data provided";
        finally.dismiss();
        return false;
    }

    fmInfo() << "TagDbHandler::addTagsForFiles: Adding tags for" << data.size() << "files";

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
            if (!ret) {
                fmCritical() << "TagDbHandler::addTagsForFiles: Failed to tag file:" << dataIt.key();
                return ret;
            }
        }
        return true;
    });

    if (!ret) {
        fmCritical() << "TagDbHandler::addTagsForFiles: Transaction failed while adding tags for files";
    } else {
        fmInfo() << "TagDbHandler::addTagsForFiles: Successfully added tags for files";
    }

    emit filesWereTagged(data);
    return ret;
}

bool TagDbHandler::removeTagsOfFiles(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::removeTagsOfFiles: Empty data provided";
        finally.dismiss();
        return false;
    }

    fmInfo() << "TagDbHandler::removeTagsOfFiles: Removing tags from" << data.size() << "files";

    // remove file--tags
    bool ret = handle->transaction([data, this]() -> bool {
        for (auto it = data.begin(); it != data.end(); ++it) {
            if (!removeSpecifiedTagOfFile(it.key(), it.value())) {
                fmCritical() << "TagDbHandler::removeTagsOfFiles: Failed to remove tags from file:" << it.key();
                return false;
            }
        }
        return true;
    });

    if (!ret) {
        fmCritical() << "TagDbHandler::removeTagsOfFiles: Transaction failed while removing tags from files";
    } else {
        fmInfo() << "TagDbHandler::removeTagsOfFiles: Successfully removed tags from files";
    }

    emit filesUntagged(data);
    return ret;
}

bool TagDbHandler::deleteTags(const QStringList &tags)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (tags.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::deleteTags: Empty tag list provided";
        finally.dismiss();
        return false;
    }

    fmInfo() << "TagDbHandler::deleteTags: Deleting" << tags.size() << "tags";

    const auto &fieldOne = Expression::Field<TagProperty>;
    const auto &fieldTwo = Expression::Field<FileTagInfo>;

    bool ret = true;
    for (const auto &tag : tags) {
        ret = handle->remove<TagProperty>(fieldOne("tagName") == tag);
        if (!ret) {
            fmCritical() << "TagDbHandler::deleteTags: Failed to remove tag property for tag:" << tag;
            return ret;
        }
        ret = handle->remove<FileTagInfo>(fieldTwo("tagName") == tag);
        if (!ret) {
            fmCritical() << "TagDbHandler::deleteTags: Failed to remove file tag info for tag:" << tag;
            return ret;
        }
    }

    emit tagsDeleted(tags);
    fmInfo() << "TagDbHandler::deleteTags: Successfully deleted" << tags.size() << "tags";
    return ret;
}

bool TagDbHandler::deleteFiles(const QStringList &urls)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (urls.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::deleteFiles: Empty URL list provided";
        finally.dismiss();
        return false;
    }

    fmInfo() << "TagDbHandler::deleteFiles: Deleting tag information for" << urls.size() << "files";

    auto field = Expression::Field<FileTagInfo>;
    for (const auto &url : urls) {
        if (!handle->remove<FileTagInfo>(field("filePath") == url)) {
            fmCritical() << "TagDbHandler::deleteFiles: Failed to delete tag information for file:" << url;
            return false;
        }
    }

    fmInfo() << "TagDbHandler::deleteFiles: Successfully deleted tag information for" << urls.size() << "files";
    return true;
}

bool TagDbHandler::changeTagColors(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::changeTagColors: Empty data provided";
        finally.dismiss();
        return false;
    }

    fmInfo() << "TagDbHandler::changeTagColors: Changing colors for" << data.size() << "tags";

    auto it = data.begin();
    bool ret = true;
    for (; it != data.end(); ++it) {
        ret = changeTagColor(it.key(), it.value().toString());
        if (!ret) {
            fmCritical() << "TagDbHandler::changeTagColors: Failed to change color for tag:" << it.key();
            return ret;
        }
    }

    emit tagsColorChanged(data);
    fmInfo() << "TagDbHandler::changeTagColors: Successfully changed colors for" << data.size() << "tags";
    return ret;
}

bool TagDbHandler::changeTagNamesWithFiles(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::changeTagNamesWithFiles: Empty data provided";
        finally.dismiss();
        return false;
    }

    fmInfo() << "TagDbHandler::changeTagNamesWithFiles: Changing names for" << data.size() << "tags";

    auto it = data.begin();
    QVariantMap updatedData;
    bool ret = true;
    for (; it != data.end(); ++it) {
        if (changeTagNameWithFile(it.key(), it.value().toString())) {
            updatedData.insert(it.key(), it.value());
        } else {
            fmCritical() << "TagDbHandler::changeTagNamesWithFiles: Failed to change name for tag:" << it.key();
            ret = false;
        }
    }

    if (!updatedData.isEmpty()) {
        emit tagsNameChanged(updatedData);
        fmInfo() << "TagDbHandler::changeTagNamesWithFiles: Successfully changed names for" << updatedData.size() << "tags";
    }


    return ret;
}

bool TagDbHandler::changeFilePaths(const QVariantMap &data)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (data.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::changeFilePaths: Empty data provided";
        finally.dismiss();
        return false;
    }

    fmInfo() << "TagDbHandler::changeFilePaths: Changing paths for" << data.size() << "files";

    auto it = data.begin();
    for (; it != data.end(); ++it) {
        if (!changeFilePath(it.key(), it.value().toString())) {
            fmCritical() << "TagDbHandler::changeFilePaths: Failed to change path for file:" << it.key();
            return false;
        }
    }

    fmInfo() << "TagDbHandler::changeFilePaths: Successfully changed paths for" << data.size() << "files";
    return true;
}

QString TagDbHandler::lastError() const
{
    return lastErr;
}

TagDbHandler::TagDbHandler(QObject *parent)
    : QObject(parent)
{
    fmInfo() << "TagDbHandler: Initializing tag database handler";
    initialize();
}

void TagDbHandler::initialize()
{
    using namespace dfmio;
    const auto &dbPath = DFMUtils::buildFilePath(StandardPaths::location(StandardPaths::kApplicationConfigPath).toLocal8Bit(),
                                                 "/deepin/dde-file-manager/database",
                                                 nullptr);

    QDir dir(dbPath);
    if (!dir.exists()) {
        if (!dir.mkpath(dbPath)) {
            fmCritical() << "TagDbHandler::initialize: Failed to create database directory:" << dbPath;
            return;
        }
        fmInfo() << "TagDbHandler::initialize: Created database directory:" << dbPath;
    }

    const auto &dbFilePath = DFMUtils::buildFilePath(dbPath.toLocal8Bit(),
                                                     Global::DataBase::kDfmDBName,
                                                     nullptr);
    handle.reset(new SqliteHandle(dbFilePath));
    QSqlDatabase db { SqliteConnectionPool::instance().openConnection(dbFilePath) };
    if (!db.isValid() || db.isOpenError()) {
        fmCritical() << "TagDbHandler::initialize: Failed to open tag database:" << dbFilePath;
        return;
    }
    db.close();

    if (!createTable(kTagTableFileTags)) {
        fmCritical() << "TagDbHandler::initialize: Failed to create table:" << kTagTableFileTags;
    } else {
        fmDebug() << "TagDbHandler::initialize: Table created or verified:" << kTagTableFileTags;
    }

    if (!createTable(kTagTableTagProperty)) {
        fmCritical() << "TagDbHandler::initialize: Failed to create table:" << kTagTableTagProperty;
    } else {
        fmDebug() << "TagDbHandler::initialize: Table created or verified:" << kTagTableTagProperty;
    }

    if (!createTable(kTagTableTrashFileTags)) {
        fmCritical() << "TagDbHandler::initialize: Failed to create table:" << kTagTableTrashFileTags;
    } else {
        fmDebug() << "TagDbHandler::initialize: Table created or verified:" << kTagTableTrashFileTags;
    }

    fmInfo() << "TagDbHandler::initialize: Tag database handler initialized successfully";
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

    if (SqliteHelper::tableName<TrashFileTagInfo>() == tableName) {

        ret = handle->createTable<TrashFileTagInfo>(
                SqliteConstraint::primary("trashIndex"),
                SqliteConstraint::autoIncreament("trashIndex"),
                SqliteConstraint::unique("trashIndex"));
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
        fmWarning() << "TagDbHandler::insertTagProperty: Empty parameters provided - name:" << name << "value:" << value;
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
        fmCritical() << "TagDbHandler::insertTagProperty: Failed to insert tag property - name:" << name << "color:" << value.toString();
        return false;
    }

    fmDebug() << "TagDbHandler::insertTagProperty: Successfully inserted tag property - name:" << name << "color:" << value.toString();
    return true;
}

bool TagDbHandler::tagFile(const QString &file, const QVariant &tags)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (file.isEmpty() || tags.isNull()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::tagFile: Empty parameters provided - file:" << file << "tags:" << tags;
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
        if (-1 == handle->insert<FileTagInfo>(temp)) {
            fmCritical() << "TagDbHandler::tagFile: Failed to insert file tag - file:" << file << "tag:" << tag;
            break;
        }
        suc--;
    }

    if (0 < suc) {
        lastErr = QString("Tag file failed! file: %1, tagName: %2").arg(file).arg(tempTags.at(--suc));
        return false;
    }

    fmDebug() << "TagDbHandler::tagFile: Successfully tagged file:" << file << "with" << tempTags.size() << "tags";
    return true;
}

bool TagDbHandler::removeSpecifiedTagOfFile(const QString &url, const QVariant &val)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (url.isEmpty() || val.isNull()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::removeSpecifiedTagOfFile: Empty parameters provided - url:" << url << "tags:" << val;
        return false;
    }

    auto field = Expression::Field<FileTagInfo>;
    const auto tempTags = val.toStringList();
    int suc = tempTags.count();
    for (const auto &tag : tempTags) {
        if (!handle->remove<FileTagInfo>((field("filePath") == url) && (field("tagName") == tag))) {
            fmCritical() << "TagDbHandler::removeSpecifiedTagOfFile: Failed to remove tag from file - file:" << url << "tag:" << tag;
            break;
        }
        suc--;
    }

    if (0 < suc) {
        lastErr = QString("Remove specified tag Of File failed! file: %1, tagName: %2").arg(url).arg(tempTags.at(--suc));
        return false;
    }

    fmDebug() << "TagDbHandler::removeSpecifiedTagOfFile: Successfully removed" << tempTags.size() << "tags from file:" << url;
    return true;
}

bool TagDbHandler::changeTagColor(const QString &tagName, const QString &newTagColor)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (tagName.isEmpty() || newTagColor.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::changeTagColor: Empty parameters provided - tagName:" << tagName << "newColor:" << newTagColor;
        return false;
    }

    const auto &field = Expression::Field<TagProperty>;
    if (!handle->update<TagProperty>(field("tagColor") = newTagColor, field("tagName") == tagName)) {
        lastErr = QString("Change tag Color failed! tagName: %1, newTagColor: %2").arg(tagName).arg(newTagColor);
        fmCritical() << "TagDbHandler::changeTagColor: Failed to update tag color - tagName:" << tagName << "newColor:" << newTagColor;
        return false;
    }

    fmDebug() << "TagDbHandler::changeTagColor: Successfully changed tag color - tagName:" << tagName << "newColor:" << newTagColor;
    return true;
}

bool TagDbHandler::changeTagNameWithFile(const QString &tagName, const QString &newName)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (tagName.isEmpty() || newName.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::changeTagNameWithFile: Empty parameters provided - oldName:" << tagName << "newName:" << newName;
        return false;
    }

    // update tagname and files tagname
    bool ret = handle->transaction([tagName, newName, this]() -> bool {
        if (!handle->update<TagProperty>(Expression::Field<TagProperty>("tagName") = newName,
                                         Expression::Field<TagProperty>("tagName") == tagName)) {
            lastErr = QString("Change tag name failed! tagName: %1, newName: %2").arg(tagName).arg(newName);
            fmCritical() << "TagDbHandler::changeTagNameWithFile: Failed to update tag property - oldName:" << tagName << "newName:" << newName;
            return false;
        }
        if (!handle->update<FileTagInfo>(Expression::Field<FileTagInfo>("tagName") = newName,
                                         Expression::Field<FileTagInfo>("tagName") == tagName)) {
            lastErr = QString("Change file tag name failed! tagName: %1, newName: %2").arg(tagName).arg(newName);
            fmCritical() << "TagDbHandler::changeTagNameWithFile: Failed to update file tag info - oldName:" << tagName << "newName:" << newName;
            return false;
        }

        return true;
    });

    if (ret) {
        fmDebug() << "TagDbHandler::changeTagNameWithFile: Successfully changed tag name - oldName:" << tagName << "newName:" << newName;
    }

    return ret;
}

bool TagDbHandler::changeFilePath(const QString &oldPath, const QString &newPath)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (oldPath.isEmpty() || newPath.isEmpty()) {
        lastErr = "input parameter is empty!";
        fmWarning() << "TagDbHandler::changeFilePath: Empty parameters provided - oldPath:" << oldPath << "newPath:" << newPath;
        return false;
    }

    const auto &field = Expression::Field<FileTagInfo>;
    if (!handle->update<FileTagInfo>(field("filePath") = newPath, field("filePath") == oldPath)) {
        lastErr = QString("Change file path failed! oldPath: %1, newPath: %2").arg(oldPath).arg(newPath);
        fmCritical() << "TagDbHandler::changeFilePath: Failed to update file path - oldPath:" << oldPath << "newPath:" << newPath;
        return false;
    }

    fmDebug() << "TagDbHandler::changeFilePath: Successfully changed file path - oldPath:" << oldPath << "newPath:" << newPath;
    return true;
}

bool TagDbHandler::saveTrashFileTags(const QString &originalPath, qint64 inode, const QStringList &tags)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (originalPath.isEmpty() || inode <= 0 || tags.isEmpty()) {
        lastErr = "input parameter is empty or invalid!";
        fmWarning() << "TagDbHandler::saveTrashFileTags: Invalid parameters - path:" << originalPath << "inode:" << inode << "tags:" << tags;
        return false;
    }

    fmInfo() << "TagDbHandler::saveTrashFileTags: Saving trash file tags - path:" << originalPath << "inode:" << inode << "tags:" << tags;

    // Remove existing record first
    const auto &field = Expression::Field<TrashFileTagInfo>;
    handle->remove<TrashFileTagInfo>(field("originalPath") == originalPath && field("fileInode") == inode);

    // Insert new record
    TrashFileTagInfo info;
    info.setOriginalPath(originalPath);
    info.setFileInode(inode);
    info.setTagNames(tags.join(","));
    info.setDeleteTime(QDateTime::currentSecsSinceEpoch());
    info.setFuture("null");

    if (-1 == handle->insert<TrashFileTagInfo>(info)) {
        lastErr = QString("Insert trash file tags failed! path: %1, inode: %2").arg(originalPath).arg(inode);
        fmCritical() << "TagDbHandler::saveTrashFileTags: Failed to insert - path:" << originalPath << "inode:" << inode;
        return false;
    }

    emit trashFileTagsChanged();
    fmInfo() << "TagDbHandler::saveTrashFileTags: Successfully saved trash file tags";
    return true;
}

QStringList TagDbHandler::getTrashFileTags(const QString &originalPath, qint64 inode)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });
    finally.dismiss();

    if (originalPath.isEmpty() || inode <= 0) {
        fmWarning() << "TagDbHandler::getTrashFileTags: Invalid parameters - path:" << originalPath << "inode:" << inode;
        return {};
    }

    const auto &field = Expression::Field<TrashFileTagInfo>;
    const auto &beans = handle->query<TrashFileTagInfo>()
                                .where(field("originalPath") == originalPath && field("fileInode") == inode)
                                .toBeans();

    if (beans.isEmpty()) {
        fmDebug() << "TagDbHandler::getTrashFileTags: No trash tags found - path:" << originalPath << "inode:" << inode;
        return {};
    }

    QString tagNames = beans.first()->getTagNames();
    QStringList tags = tagNames.split(",", Qt::SkipEmptyParts);
    fmDebug() << "TagDbHandler::getTrashFileTags: Retrieved" << tags.size() << "tags - path:" << originalPath;
    return tags;
}
QVariantMap TagDbHandler::getTrashFileTags(const QStringList &queryParams)
{
    QVariantMap result;

    if (queryParams.size() < 2) {
        fmWarning() << "TagDbHandler::getTrashFileTags: Invalid query parameters, expected at least 2";
        return result;
    }

    QString path, inodeStr;
    for (const QString &item : queryParams) {
        if (item.startsWith("originalPath:"))
            path = item.mid(13);
        else if (item.startsWith("inode:"))
            inodeStr = item.mid(6);
    }

    if (path.isEmpty() || inodeStr.isEmpty()) {
        fmWarning() << "TagDbHandler::getTrashFileTags: Failed to parse parameters";
        return result;
    }

    QStringList tags = getTrashFileTags(path, inodeStr.toLongLong());
    result["tags"] = tags;
    return result;
}
bool TagDbHandler::removeTrashFileTags(const QString &originalPath, qint64 inode)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    if (originalPath.isEmpty() || inode <= 0) {
        lastErr = "input parameter is empty or invalid!";
        fmWarning() << "TagDbHandler::removeTrashFileTags: Invalid parameters - path:" << originalPath << "inode:" << inode;
        return false;
    }

    fmInfo() << "TagDbHandler::removeTrashFileTags: Removing trash file tags - path:" << originalPath << "inode:" << inode;

    const auto &field = Expression::Field<TrashFileTagInfo>;
    if (!handle->remove<TrashFileTagInfo>(field("originalPath") == originalPath && field("fileInode") == inode)) {
        lastErr = QString("Remove trash file tags failed! path: %1, inode: %2").arg(originalPath).arg(inode);
        fmCritical() << "TagDbHandler::removeTrashFileTags: Failed to remove - path:" << originalPath << "inode:" << inode;
        return false;
    }

    emit trashFileTagsChanged();
    fmInfo() << "TagDbHandler::removeTrashFileTags: Successfully removed trash file tags";
    return true;
}

bool TagDbHandler::clearAllTrashTags()
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });

    fmInfo() << "TagDbHandler::clearAllTrashTags: Clearing all trash file tags";

    const auto &field = Expression::Field<TrashFileTagInfo>;
    if (!handle->remove<TrashFileTagInfo>(field("trashIndex") > 0)) {
        lastErr = "Clear all trash tags failed!";
        fmCritical() << "TagDbHandler::clearAllTrashTags: Failed to clear trash tags";
        return false;
    }

    emit trashFileTagsChanged();
    fmInfo() << "TagDbHandler::clearAllTrashTags: Successfully cleared all trash file tags";
    return true;
}

bool TagDbHandler::hasTrashFileTags(const QString &originalPath, qint64 inode)
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });
    finally.dismiss();

    if (originalPath.isEmpty() || inode <= 0) {
        fmWarning() << "TagDbHandler::hasTrashFileTags: Invalid parameters - path:" << originalPath << "inode:" << inode;
        return false;
    }

    const auto &field = Expression::Field<TrashFileTagInfo>;
    const auto &beans = handle->query<TrashFileTagInfo>()
                                .where(field("originalPath") == originalPath && field("fileInode") == inode)
                                .toBeans();

    bool exists = !beans.isEmpty();
    fmDebug() << "TagDbHandler::hasTrashFileTags: Trash tags exist:" << exists << "- path:" << originalPath;
    return exists;
}

QVariantHash TagDbHandler::getAllTrashFileTags()
{
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { lastErr.clear(); });
    finally.dismiss();

    QVariantHash result;
    const auto &beans = handle->query<TrashFileTagInfo>().toBeans();

    for (const auto &bean : beans) {
        QString originalPath = bean->property("originalPath").toString();
        qint64 inode = bean->property("fileInode").toLongLong();
        QString tagNames = bean->property("tagNames").toString();

        if (!originalPath.isEmpty() && inode > 0 && !tagNames.isEmpty()) {
            QString key = QString("%1:%2").arg(originalPath).arg(inode);
            result[key] = tagNames.split(",", Qt::SkipEmptyParts);
        }
    }

    fmInfo() << "TagDbHandler::getAllTrashFileTags: Loaded" << result.size() << "trash tag records";
    return result;
}

DAEMONPTAG_END_NAMESPACE
