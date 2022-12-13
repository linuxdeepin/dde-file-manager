// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagmanager.h"

#include "dfmplugin_tag_global.h"
#include "dbus/tagdbus.h"
#include "taghelper.h"
#include "events/tageventcaller.h"
#include "widgets/tagcolorlistwidget.h"
#include "files/tagfileinfo.h"
#include "utils/anythingmonitorfilter.h"

#include <dfm-framework/dpf.h>
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/file/local/desktopfileinfo.h"

#include <QMenu>
#include <QWidgetAction>

Q_DECLARE_METATYPE(Qt::DropAction *)

DPTAG_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

TagManager::TagManager(QObject *parent)
    : QObject(parent)
{
    tagDbus = new TagDBus();
    initializeConnection();
}

TagManager::~TagManager()
{
    if (tagDbus) {
        delete tagDbus;
        tagDbus = nullptr;
    }
}

void TagManager::initializeConnection()
{
    connect(tagDbus, &TagDBus::addedNewTags, this, &TagManager::onTagAdded);
    connect(tagDbus, &TagDBus::tagsDeleted, this, &TagManager::onTagDeleted);
    connect(tagDbus, &TagDBus::tagColorChanged, this, &TagManager::onTagColorChanged);
    connect(tagDbus, &TagDBus::tagNameChanged, this, &TagManager::onTagNameChanged);
    connect(tagDbus, &TagDBus::filesTagged, this, &TagManager::onFilesTagged);
    connect(tagDbus, &TagDBus::filesUntagged, this, &TagManager::onFilesUntagged);
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

bool TagManager::canTagFile(const QUrl &url) const
{
    // TODO(perf) costs when first painting (while loading files)
    if (url.scheme() == Global::Scheme::kFile) {
        auto &&fileInfo { InfoFactory::create<AbstractFileInfo>(url) };
        if (!fileInfo)
            return false;

        if (!AnythingMonitorFilter::instance().whetherFilterCurrentPath(fileInfo->urlOf(AbstractFileInfo::FileUrlInfoType::kParentUrl).toLocalFile()))
            return false;

        const QString filePath { fileInfo->pathOf(AbstractFileInfo::FilePathInfoType::kFilePath) };
        const QString &compressPath { QDir::homePath() + "/.avfs/" };
        if (filePath.startsWith(compressPath))
            return false;

        const QString &parentPath { fileInfo->urlOf(AbstractFileInfo::FileUrlInfoType::kParentUrl).path() };
        if (parentPath == "/home" || parentPath == FileUtils::bindPathTransform("/home", true))
            return false;

        if (FileUtils::isDesktopFile(url)) {
            auto desktopInfo { dynamic_cast<DesktopFileInfo *>(fileInfo.data()) };
            if (desktopInfo)
                return desktopInfo->canTag();
        }

        return !SystemPathUtil::instance()->isSystemPath(filePath);
    }

    if (dpfHookSequence->run("dfmplugin_tag", "hook_CanTag", url))
        return true;

    return false;
}

bool TagManager::paintListTagsHandle(int role, const QUrl &url, QPainter *painter, QRectF *rect)
{
    if (!canTagFile(url.toString()))
        return false;

    if (role != kItemFileDisplayNameRole && role != kItemNameRole)
        return false;

    const auto &tags = getTagsByUrls({ url }, false).toMap();
    if (tags.isEmpty())
        return false;
    auto tempTags = TagHelper::instance()->displayTagNameConversion(tags.first().toStringList());
    const auto &tagsColor = getTagsColor(tempTags);

    if (!tagsColor.isEmpty()) {
        QRectF boundingRect(0, 0, (tagsColor.size() + 1) * kTagDiameter / 2, kTagDiameter);
        boundingRect.moveCenter(rect->center());
        boundingRect.moveRight(rect->right());

        TagHelper::instance()->paintTags(painter, boundingRect, tagsColor.values());

        rect->setRight(boundingRect.left() - 10);
    }

    return false;
}

bool TagManager::paintIconTagsHandle(int role, const QUrl &url, QPainter *painter, QRectF *rect)
{
    if (!canTagFile(url.toString()))
        return false;

    if (role != kItemFileDisplayNameRole && role != kItemNameRole)
        return false;

    const auto &fileTags = getTagsByUrls({ url }, false).toMap();
    if (fileTags.isEmpty())
        return false;

    QStringList tempTags = fileTags.first().toStringList();
    const auto &tagsColor = getTagsColor(tempTags);

    if (!tagsColor.isEmpty()) {
        QRectF boundingRect(0, 0, (tagsColor.size() + 1) * kTagDiameter / 2, kTagDiameter);
        boundingRect.moveCenter(rect->center());
        boundingRect.moveTop(rect->top());
        TagHelper::instance()->paintTags(painter, boundingRect, tagsColor.values());
        rect->setTop(boundingRect.bottom());
    }

    return false;
}

bool TagManager::fileDropHandle(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    if (toUrl.scheme() == scheme()) {
        QList<QUrl> canTagFiles;
        for (const auto &url : fromUrls) {
            if (canTagFile(url.toString()))
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

bool TagManager::fileDropHandleWithAction(const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction *action)
{
    if (toUrl.scheme() == scheme()) {
        *action = Qt::IgnoreAction;
    }
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

QString TagManager::getTagIconName(const QString &tag) const
{
    if (tag.isEmpty())
        return QString();

    QStringList tagLst = TagHelper::instance()->dbTagNameConversion({ tag });
    if (tagLst.isEmpty())
        return QString();

    const auto &dataMap = getTagsColorName({ tagLst.first() });
    if (dataMap.contains(tagLst.first()))
        return TagHelper::instance()->qureyIconNameByColor(QColor(dataMap.value(tagLst.first())));

    return QString();
}

TagManager::TagColorMap TagManager::getAllTags()
{
    auto var = tagDbus->Query(static_cast<std::size_t>(TagActionType::kGetAllTags));
    if (var.isNull())
        return {};

    const auto &dataMap = var.toMap();
    TagColorMap result;
    QMap<QString, QVariant>::const_iterator it = dataMap.begin();

    for (; it != dataMap.end(); ++it)
        result[it.key()] = it.value().value<QColor>();

    return result;
}

TagManager::TagColorMap TagManager::getTagsColor(const QStringList &tags) const
{
    if (tags.isEmpty())
        return {};

    auto var = tagDbus->Query(static_cast<std::size_t>(TagActionType::kGetTagsColor), tags);
    if (var.isNull())
        return {};

    const auto &dataMap = var.toMap();
    TagColorMap result;
    QMap<QString, QVariant>::const_iterator it = dataMap.begin();
    for (; it != dataMap.end(); ++it)
        if (it.value().isValid())
            result[it.key()] = QColor(it.value().toString());

    return result;
}

QVariant TagManager::getTagsByUrls(const QList<QUrl> &filePaths, bool same) const
{
    // single path:  get all tags of path
    // mult paths:  get same tags of paths

    if (filePaths.isEmpty())
        return {};

    QStringList paths;
    for (const auto &temp : filePaths) {
        const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(temp);
        if (info) {
            paths.append(temp.path());
        } else {
            paths.append(UrlRoute::urlToLocalPath(temp));
        }
    }

    auto type = same ? static_cast<std::uint8_t>(TagActionType::kGetSameTagsOfDiffFiles) : static_cast<std::uint8_t>(TagActionType::kGetTagsThroughFile);
    return tagDbus->Query(type, paths);
}

QStringList TagManager::getFilesByTag(const QString &tag)
{
    if (tag.isEmpty())
        return {};

    QString tagName = TagHelper::instance()->qureyColorNameByDisplayName(tag);
    if (tagName.isEmpty())
        tagName = tag;
    auto var = tagDbus->Query(static_cast<std::size_t>(TagActionType::kGetFilesThroughTag), { tagName });
    if (var.isNull())
        return {};

    auto files = var.toMap().value(tagName);
    return var.toMap().value(tagName).toStringList();
}

bool TagManager::setTagsForFiles(const QStringList &tags, const QList<QUrl> &files)
{

    // if tags is empty means delete all files's tags
    if (files.isEmpty())
        return false;

    // set tags for mult files
    QStringList mutualTagNames = TagManager::instance()->getTagsByUrls(files, true).toStringList();
    // for deleting.
    QStringList dirtyTagNames;
    for (const QString &tag : mutualTagNames)
        if (!tags.contains(tag))
            dirtyTagNames << tag;

    bool result = false;
    if (!dirtyTagNames.isEmpty())
        result = TagManager::instance()->removeTagsOfFiles(dirtyTagNames, files) || result;

    for (const QUrl &url : files) {
        QStringList tagsOfFile = TagManager::instance()->getTagsByUrls({ url }, true).toStringList();
        QStringList newTags;

        for (const QString &tag : tags) {
            if (!tagsOfFile.contains(tag))
                newTags.append(tag);
        }

        if (!newTags.isEmpty()) {
            tagsOfFile.append(newTags);
            result = TagManager::instance()->addTagsForFiles(newTags, { url }) || result;
        }
    }

    return result;
}

bool TagManager::addTagsForFiles(const QList<QString> &tags, const QList<QUrl> &files)
{
    if (tags.isEmpty() || files.isEmpty())
        return false;

    auto tempTags = TagHelper::instance()->dbTagNameConversion(tags);

    // add Tag Property
    QMap<QString, QVariant> tagWithColor {};
    for (const QString &tagName : tags) {
        QString colorName = tagColorMap.contains(tagName) ? tagColorMap[tagName] : TagHelper::instance()->qureyColorByColorName(tagName).name();
        tagWithColor[tagName] = QVariant { QList<QString> { colorName } };
    }

    // make tag for files
    QVariant checkTagResult { tagDbus->Insert(static_cast<std::size_t>(TagActionType::kAddTags), tagWithColor) };
    if (checkTagResult.toBool()) {
        QVariantMap infos;
        for (const auto &f : files)
            infos[f.path()] = QVariant(tempTags);

        QVariant ret = tagDbus->Insert(static_cast<std::size_t>(TagActionType::kMakeFilesTags), infos);
        if (ret.toBool())
            return true;

        qWarning() << "Create tags successfully! But failed to tag files";
    } else {
        qWarning() << "The tag don't exist.";
    }
    return false;
}

bool TagManager::removeTagsOfFiles(const QList<QString> &tags, const QList<QUrl> &files)
{
    if (tags.isEmpty() || files.isEmpty())
        return false;

    QMap<QString, QVariant> fileWithTag;

    for (const QUrl &url : files) {
        const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(url);
        if (info) {
            fileWithTag[info->pathOf(AbstractFileInfo::FilePathInfoType::kFilePath)] = QVariant(tags);
        } else {
            fileWithTag[UrlRoute::urlToLocalPath(url)] = QVariant(tags);
        }
    }

    return tagDbus->Delete(static_cast<std::size_t>(TagActionType::kRemoveTagsOfFiles), fileWithTag);
}

bool TagManager::pasteHandle(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to)
{
    Q_UNUSED(winId)
    Q_UNUSED(fromUrls)

    if (to.scheme() == scheme()) {
        auto action = ClipBoard::instance()->clipboardAction();
        if (action == ClipBoard::kCutAction)
            return true;

        auto sourceUrls = ClipBoard::instance()->clipboardFileUrlList();
        QList<QUrl> canTagFiles;
        for (const auto &url : sourceUrls) {
            if (canTagFile(url))
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
void TagManager::deleteTags(const QStringList &tags)
{
    if (this->deleteTagData(tags, static_cast<std::size_t>(TagActionType::kDeleteTags))) {
        for (const auto &tag : tags) {
            QUrl url = TagHelper::instance()->makeTagUrlByTagName(tag);
            dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", url);
            emit tagDeleted(tag);
        }
    }
}

void TagManager::deleteFiles(const QList<QUrl> &urls)
{
    QStringList paths;
    for (const auto &temp : urls)
        paths.append(temp.toString());

    this->deleteTagData(paths, static_cast<std::size_t>(TagActionType::kDeleteFiles));
}

bool TagManager::changeTagColor(const QString &tagName, const QString &newTagColor)
{
    if (tagName.isEmpty() || newTagColor.isEmpty())
        return false;

    QMap<QString, QVariant> changeMap { { tagName, QVariant { QString(newTagColor) } } };
    return tagDbus->Update(static_cast<std::size_t>(TagActionType::kChangeTagColor), changeMap);
}

bool TagManager::changeTagName(const QString &tagName, const QString &newName)
{
    if (tagName.isEmpty() || newName.isEmpty())
        return false;

    bool result = false;

    if (getAllTags().contains(newName)) {
        DialogManagerInstance->showRenameNameSameErrorDialog(newName);
        return result;
    }

    QMap<QString, QVariant> oldAndNewName = { { tagName, QVariant { newName } } };
    return tagDbus->Update(static_cast<std::size_t>(TagActionType::kChangeTagName), oldAndNewName);
}

QMap<QString, QString> TagManager::getTagsColorName(const QStringList &tags) const
{
    if (tags.isEmpty())
        return {};

    QVariant var = tagDbus->Query(static_cast<std::size_t>(TagActionType::kGetTagsColor), tags);

    const auto &dataMap = var.toMap();
    QMap<QString, QString> result;
    QMap<QString, QVariant>::const_iterator it = dataMap.begin();
    for (; it != dataMap.end(); ++it)
        result[it.key()] = it.value().toString();

    return result;
}

bool TagManager::deleteTagData(const QStringList &data, const uint8_t &type)
{
    if (data.isEmpty())
        return false;

    QMap<QString, QVariant> tagDelMap {};

    // Just to make sure the interface parameters are consistent, we only care about the values
    tagDelMap["deleteTagData"] = QVariant { data };
    return tagDbus->Delete(type, tagDelMap);
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
        if (url.fragment().isEmpty() && (DialogManagerInstance->showDeleteFilesDialog({ url }) == QDialog::Accepted))
            TagManager::instance()->deleteTags({ TagHelper::instance()->getTagNameFromUrl(url) });
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

    QAction *act = menu->exec(globalPos);
    if (act) {
        QList<QUrl> urls { url };
        dpfSignalDispatcher->publish("dfmplugin_tag", "signal_ReportLog_MenuData", act->text(), urls);
    }
    delete menu;
}

void TagManager::renameHandle(quint64 windowId, const QUrl &url, const QString &name)
{
    Q_UNUSED(windowId);
    TagManager::instance()->changeTagName(TagHelper::instance()->getTagNameFromUrl(url), name);
}

QMap<QString, QColor> TagManager::assignColorToTags(const QStringList &tagList) const
{
    const auto &allTags = TagManager::instance()->getAllTags();
    QMap<QString, QColor> tagsMap;

    for (const auto &tag : tagList) {
        const auto &tagMap = TagManager::instance()->getTagsColor({ tag });
        if (tagMap.isEmpty()) {
            QColor tagColor;
            if (allTags.contains(tag)) {
                tagColor = allTags[tag];
            } else {
                const auto &colorName = TagHelper::instance()->getColorNameByTag(tag);
                tagColor = TagHelper::instance()->qureyColorByColorName(colorName);
                TagManager::instance()->registerTagColor(tag, tagColor.name());
            }

            tagsMap[tag] = tagColor;
        } else {
            tagsMap.unite(tagMap);
        }
    }

    return tagsMap;
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
