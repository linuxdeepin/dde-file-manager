/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "tagmanager.h"
#include "taghelper.h"
#include "tagdbushelper.h"
#include "events/tageventcaller.h"
#include "widgets/tagcolorlistwidget.h"
#include "files/tagfileinfo.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/utils/fileutils.h"

#include <QMap>
#include <QColor>
#include <QMenu>
#include <QWidgetAction>

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

TagManager::TagManager(QObject *parent)
    : QObject(parent)
{
    dbusHelper = new TagDBusHelper(this);
    initializeConnection();
}

void TagManager::initializeConnection()
{
    connect(dbusHelper, &TagDBusHelper::addNewTags, this, &TagManager::onTagAdded);
    connect(dbusHelper, &TagDBusHelper::tagsDeleted, this, &TagManager::onTagDeleted);
    connect(dbusHelper, &TagDBusHelper::tagColorChanged, this, &TagManager::onTagColorChanged);
    connect(dbusHelper, &TagDBusHelper::tagNameChanged, this, &TagManager::onTagNameChanged);
    connect(dbusHelper, &TagDBusHelper::filesTagged, this, &TagManager::onFilesTagged);
    connect(dbusHelper, &TagDBusHelper::filesUntagged, this, &TagManager::onFilesUntagged);
}

TagManager *TagManager::instance()
{
    static TagManager ins;
    return &ins;
}

QUrl TagManager::rootUrl()
{
    QUrl rootUrl;
    rootUrl.setScheme(scheme());
    rootUrl.setPath("/");
    return rootUrl;
}

TagManager::TagsMap TagManager::getAllTags()
{
    QMap<QString, QVariant> dataMap = { { QString { " " }, QVariant { QList<QString> { QString { " " } } } } };

    QVariant var = dbusHelper->sendDataToDBus(dataMap, TagActionType::kGetAllTags);
    dataMap = var.toMap();

    TagsMap result;
    QMap<QString, QVariant>::const_iterator it = dataMap.begin();

    for (; it != dataMap.end(); ++it)
        result[it.key()] = TagHelper::instance()->qureyColorByColorName(it.value().toString());

    return result;
}

QMap<QString, QString> TagManager::getTagsColorName(const QStringList &tags) const
{
    QMap<QString, QString> result;

    if (!tags.isEmpty()) {
        QMap<QString, QVariant> dataMap {};

        for (const QString &tagName : tags)
            dataMap[tagName] = QVariant { QList<QString> { QString { " " } } };

        dataMap = dbusHelper->sendDataToDBus(dataMap, TagActionType::kGetTagsColor).toMap();

        QMap<QString, QVariant>::const_iterator it = dataMap.begin();
        for (; it != dataMap.end(); ++it)
            result[it.key()] = it.value().toString();
    }

    return result;
}

QList<QColor> TagManager::getTagColors(const QList<QUrl> &urls) const
{
    QStringList tags = getTagsByUrls(urls);
    TagsMap tagMap = getTagsColor(tags);
    QList<QColor> colors {};

    TagsMap::const_iterator it = tagMap.begin();
    while (it != tagMap.end()) {
        colors.append(it.value());
        ++it;
    }

    return colors;
}

TagManager::TagsMap TagManager::getTagsColor(const QList<QString> &tags) const
{
    TagsMap result;

    if (!tags.isEmpty()) {
        QMap<QString, QString> dataMap = getTagsColorName(tags);

        QMap<QString, QString>::const_iterator it = dataMap.begin();
        for (; it != dataMap.end(); ++it)
            result[it.key()] = TagHelper::instance()->qureyColorByColorName(it.value());
    }

    return result;
}

QStringList TagManager::getTagsByUrls(const QList<QUrl> &urlList) const
{
    QMap<QString, QVariant> dataMap;

    if (!urlList.isEmpty()) {
        for (const QUrl &url : urlList) {
            const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(url);
            if (info) {
                dataMap[info->filePath()] = QVariant { QList<QString> {} };
            } else {
                dataMap[UrlRoute::urlToLocalPath(url)] = QVariant { QList<QString> {} };
            }
        }

        if (dataMap.isEmpty())
            return QList<QString> {};

        QVariant var = dbusHelper->sendDataToDBus(dataMap, TagActionType::kGetTagsThroughFile);
        return var.toStringList();
    }

    return QList<QString> {};
}

QString TagManager::getTagColorName(const QString &tag) const
{
    const TagsMap &dataMap = getTagsColor({ tag });
    const QColor &color = dataMap.value(tag);

    if (!color.isValid())
        return QString();

    return TagHelper::instance()->qureyColorNameByColor(color);
}

QString TagManager::getTagIconName(const QString &tag) const
{
    const QMap<QString, QString> &dataMap = getTagsColorName({ tag });
    if (dataMap.contains(tag))
        return TagHelper::instance()->qureyIconNameByColorName(dataMap.value(tag));

    return QString();
}

QString TagManager::getTagIconName(const QColor &color) const
{
    return TagHelper::instance()->qureyIconNameByColor(color);
}

QStringList TagManager::getFilesByTag(const QString &tag)
{
    QStringList list;

    if (!tag.isEmpty()) {
        QMap<QString, QVariant> dataMap = { { tag, QVariant { QList<QString> { QString { " " } } } } };
        QVariant var = dbusHelper->sendDataToDBus(dataMap, TagActionType::kGetFilesThroughTag);
        list = var.toStringList();
    }

    return list;
}

bool TagManager::setTagsForFiles(const QList<QString> &tags, const QList<QUrl> &files)
{
    QStringList mutualTagNames = TagManager::instance()->getTagsByUrls(files);   // the mutual tags of multi files.
    QStringList dirtyTagNames;   // for deleting.

    for (const QString &tag : mutualTagNames) {
        if (!tags.contains(tag))
            dirtyTagNames << tag;
    }

    bool result = false;
    if (!dirtyTagNames.isEmpty())
        result = TagManager::instance()->removeTagsOfFiles(dirtyTagNames, files) || result;

    for (const QUrl &url : files) {
        QStringList tagsOfFile = TagManager::instance()->getTagsByUrls({ url });
        QStringList newTags;

        for (const QString &tag : tags) {
            if (!tagsOfFile.contains(tag))
                newTags.append(tag);
        }

        if (!newTags.isEmpty()) {
            tagsOfFile.append(newTags);
            result = TagManager::instance()->addTagsForFiles(tagsOfFile, { url }) || result;
        }
    }

    return result;
}

bool TagManager::addTagsForFiles(const QList<QString> &tags, const QList<QUrl> &files)
{
    if (!tags.isEmpty() && !files.isEmpty()) {
        QMap<QString, QVariant> tagWithColorName {};

        for (const QString &tagName : tags) {
            QString colorName = tagColorMap.contains(tagName) ? tagColorMap[tagName] : TagHelper::instance()->getColorNameByTag(tagName);
            tagWithColorName[tagName] = QVariant { QList<QString> { colorName } };
        }

        QVariant checkTagResult { dbusHelper->sendDataToDBus(tagWithColorName, TagActionType::kBeforeMakeFilesTags) };

        if (checkTagResult.toBool()) {
            QMap<QString, QVariant> fileWithTag {};

            for (const QUrl &url : files) {
                const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(url);
                fileWithTag[info->filePath()] = QVariant { tags };
            }

            qInfo() << fileWithTag;
            QVariant tagResult = dbusHelper->sendDataToDBus(fileWithTag, TagActionType::kMakeFilesTags);

            if (!tagResult.toBool())
                qWarning() << "Create tags successfully! But failed to tag files";

            return true;
        } else {
            qWarning() << "The tag don't exist.";
        }
    }

    return false;
}

bool TagManager::removeTagsOfFiles(const QList<QString> &tags, const QList<QUrl> &files)
{
    bool result { true };

    if (!tags.isEmpty() && !files.isEmpty()) {
        QMap<QString, QVariant> fileWithTag;

        for (const QUrl &url : files) {
            const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(url);
            if (info) {
                fileWithTag[info->filePath()] = QVariant(tags);
            } else {
                fileWithTag[UrlRoute::urlToLocalPath(url)] = QVariant(tags);
            }
        }

        QVariant var = dbusHelper->sendDataToDBus(fileWithTag, TagActionType::kRemoveTagsOfFiles);
        result = var.toBool();
    }

    return result;
}

bool TagManager::changeTagColor(const QString &tagName, const QString &newTagColor)
{
    bool result = false;

    if (!tagName.isEmpty() && !newTagColor.isEmpty()) {
        QMap<QString, QVariant> stringVar = { { tagName, QVariant { QList<QString> { newTagColor } } } };
        QVariant var = dbusHelper->sendDataToDBus(stringVar, TagActionType::kChangeTagColor);
        result = var.toBool();
    }

    return result;
}

bool TagManager::changeTagName(const QString &tagName, const QString &newName)
{
    bool result = false;

    if (getAllTags().contains(newName)) {
        DialogManagerInstance->showRenameNameSameErrorDialog(newName);
        return result;
    }

    if (!tagName.isEmpty() && !newName.isEmpty()) {
        QMap<QString, QVariant> oldAndNewName = { { tagName, QVariant { newName } } };
        QVariant var = dbusHelper->sendDataToDBus(oldAndNewName, TagActionType::kChangeTagName);
        result = var.toBool();
    }

    return result;
}

void TagManager::deleteTags(const QList<QString> &tags)
{
    if (!tags.isEmpty()) {
        QMap<QString, QVariant> tagMap {};

        for (const QString &tagName : tags) {
            tagMap[tagName] = QVariant { QList<QString> {} };
        }

        dbusHelper->sendDataToDBus(tagMap, TagActionType::kDeleteTags);
    }
}

void TagManager::deleteFiles(const QList<QUrl> &urls)
{
    QStringList tagNames;
    QList<QUrl> localFiles;

    for (const QUrl &url : urls) {
        if (!url.fragment(QUrl::FullyDecoded).isEmpty()) {
            localFiles << url;
        } else {
            tagNames.append(TagHelper::instance()->getTagNameFromUrl(url));
        }
    }

    if (!localFiles.isEmpty()) {
        return;
    }

    if (DialogManagerInstance->showDeleteFilesClearTrashDialog(urls) == QDialog::Accepted)
        deleteTags(tagNames);
}

bool TagManager::canTagFile(const AbstractFileInfoPointer &fileInfo) const
{
    if (!fileInfo || !fileInfo->canTag())
        return false;

    QString filePath = fileInfo->filePath();

    if (!filePath.startsWith("/home/") && !filePath.startsWith(FileUtils::bindPathTransform("/home/", true)))
        return false;

    return true;
}

bool TagManager::paintListTagsHandle(int role, const QUrl &url, QPainter *painter, QRectF *rect)
{
    auto fileInfo = InfoCache::instance().getCacheInfo(url);
    if (!canTagFile(fileInfo))
        return false;

    if (role != kItemFileDisplayNameRole && role != kItemNameRole)
        return false;

    const QList<QColor> &colors = getTagColors({ url });

    if (!colors.isEmpty()) {
        QRectF boundingRect(0, 0, (colors.size() + 1) * kTagDiameter / 2, kTagDiameter);
        boundingRect.moveCenter(rect->center());
        boundingRect.moveRight(rect->right());

        TagHelper::instance()->paintTags(painter, boundingRect, colors);

        rect->setRight(boundingRect.left() - 10);
    }

    return false;
}

bool TagManager::paintIconTagsHandle(int role, const QUrl &url, QPainter *painter, QRectF *rect)
{
    auto fileInfo = InfoCache::instance().getCacheInfo(url);
    if (!canTagFile(fileInfo))
        return false;

    if (role != kItemFileDisplayNameRole && role != kItemNameRole)
        return false;

    const QList<QColor> &colors = getTagColors({ url });

    if (!colors.isEmpty()) {
        QRectF boundingRect(0, 0, (colors.size() + 1) * kTagDiameter / 2, kTagDiameter);
        boundingRect.moveCenter(rect->center());
        boundingRect.moveTop(rect->top());

        TagHelper::instance()->paintTags(painter, boundingRect, colors);

        rect->setTop(boundingRect.bottom());
    }

    return false;
}

bool TagManager::pasteHandle(quint64 winId, const QUrl &to)
{
    Q_UNUSED(winId)

    if (to.scheme() == scheme()) {
        auto action = ClipBoard::instance()->clipboardAction();
        if (action == ClipBoard::kCutAction)
            return true;

        auto sourceUrls = ClipBoard::instance()->clipboardFileUrlList();
        QList<QUrl> canTagFiles;
        for (const auto &url : sourceUrls) {
            const auto &info = InfoFactory::create<AbstractFileInfo>(url);
            if (canTagFile(info))
                canTagFiles << url;
        }

        if (canTagFiles.isEmpty())
            return true;

        const auto &tagInfo = InfoFactory::create<TagFileInfo>(to);
        TagManager::setTagsForFiles(QList<QString>() << tagInfo->tagName(), canTagFiles);
        return true;
    }

    return false;
}

bool TagManager::fileDropHandle(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    if (toUrl.scheme() == scheme()) {
        QList<QUrl> canTagFiles;
        for (const auto &url : fromUrls) {
            const auto &info = InfoFactory::create<AbstractFileInfo>(url);
            if (canTagFile(info))
                canTagFiles << url;
        }

        if (canTagFiles.isEmpty())
            return true;

        const auto &tagInfo = InfoFactory::create<TagFileInfo>(toUrl);
        TagManager::setTagsForFiles(QList<QString>() << tagInfo->tagName(), canTagFiles);
        return true;
    }

    return false;
}

bool TagManager::fileDropHandleWithAction(const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction action)
{
    Q_UNUSED(action)

    return fileDropHandle(fromUrls, toUrl);
}

bool TagManager::sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup)
{
    Q_ASSERT(mapGroup);
    if (url.scheme() == TagManager::scheme()) {
        QVariantMap map;
        const QString &tagName = TagHelper::instance()->getTagNameFromUrl(url);
        map["CrumbData_Key_Url"] = url;
        map["CrumbData_Key_IconName"] = TagManager::instance()->getTagIconName(tagName);
        mapGroup->push_back(map);
        return true;
    }

    return false;
}

bool TagManager::registerTagColor(const QString &tagName, const QString &color)
{
    if (tagColorMap.contains(tagName)) {
        qInfo() << "This tag name has registed: " << tagName;
        return false;
    }

    tagColorMap[tagName] = color;
    return true;
}

void TagManager::contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    QMenu *menu = new QMenu;

    // base action
    menu->addAction(QObject::tr("Open in new window"), [url]() {
        TagEventCaller::sendOpenWindow(url);
    });

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        TagEventCaller::sendOpenTab(windowId, url);
    });

    newTabAct->setDisabled(!TagEventCaller::sendCheckTabAddable(windowId));

    menu->addSeparator();

    // tag action
    menu->addAction(QObject::tr("Rename"), [url, windowId]() {
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_TriggerEdit", windowId, url);
    });

    menu->addAction(QObject::tr("Remove"), [url]() {
        TagManager::instance()->deleteFiles({ url });
    });

    menu->addSeparator();

    TagColorListWidget *tagWidget = new TagColorListWidget;
    QWidgetAction *tagAction = new QWidgetAction(menu);
    tagAction->setDefaultWidget(tagWidget);
    tagAction->setText("Change color of present tag");
    tagWidget->setExclusive(true);
    tagWidget->setToolTipVisible(false);

    menu->addAction(tagAction);
    connect(tagAction, &QWidgetAction::triggered, TagManager::instance(), [url, tagWidget]() {
        if (tagWidget->checkedColorList().size() > 0) {
            QString tagName = TagHelper::instance()->getTagNameFromUrl(url);
            QString colorName = TagHelper::instance()->qureyColorNameByColor(tagWidget->checkedColorList().first());

            TagManager::instance()->changeTagColor(tagName, colorName);
        }
    });

    menu->exec(globalPos);
    delete menu;
}

void TagManager::renameHandle(quint64 windowId, const QUrl &url, const QString &name)
{
    Q_UNUSED(windowId);
    TagManager::instance()->changeTagName(TagHelper::instance()->getTagNameFromUrl(url), name);
}

void TagManager::onTagAdded(const QStringList &tags)
{
    for (const QString &tag : tags) {
        auto &&url { TagHelper::instance()->makeTagUrlByTagName(tag) };
        auto &&map { TagHelper::instance()->createSidebarItemInfo(tag) };
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Add", url, map);
    }
}

void TagManager::onTagDeleted(const QStringList &tags)
{
    for (const QString &tag : tags) {
        QUrl url = TagHelper::instance()->makeTagUrlByTagName(tag);
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", url);

        emit tagDeleted(tag);
    }
}

void TagManager::onTagColorChanged(const QMap<QString, QString> &tagAndColorName)
{
    QMap<QString, QString>::const_iterator it = tagAndColorName.begin();

    while (it != tagAndColorName.end()) {
        QUrl url = TagHelper::instance()->makeTagUrlByTagName(it.key());
        QString iconName = TagHelper::instance()->qureyIconNameByColorName(it.value());
        QIcon icon = QIcon::fromTheme(iconName);
        QVariantMap map {
            { "Property_Key_Icon", icon }
        };
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", url, map);
        ++it;
    }
}

void TagManager::onTagNameChanged(const QMap<QString, QString> &oldAndNew)
{
    QMap<QString, QString>::const_iterator it = oldAndNew.begin();

    while (it != oldAndNew.end()) {
        QUrl &&url { TagHelper::instance()->makeTagUrlByTagName(it.key()) };
        auto &&map { TagHelper::instance()->createSidebarItemInfo(it.value()) };
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", url, map);
        ++it;
    }
}

void TagManager::onFilesTagged(const QMap<QString, QList<QString>> &fileAndTags)
{
    if (!fileAndTags.isEmpty()) {
        TagEventCaller::sendFileUpdate(fileAndTags.firstKey());
    }
    emit filesTagged(fileAndTags);
}

void TagManager::onFilesUntagged(const QMap<QString, QList<QString>> &fileAndTags)
{
    if (!fileAndTags.isEmpty()) {
        TagEventCaller::sendFileUpdate(fileAndTags.firstKey());
    }
    emit filesUntagged(fileAndTags);
}
