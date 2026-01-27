// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagmanager.h"

#include "dfmplugin_tag_global.h"
#include "taghelper.h"
#include "events/tageventcaller.h"
#include "widgets/tagcolorlistwidget.h"
#include "files/tagfileinfo.h"
#include "utils/anythingmonitorfilter.h"
#include "utils/filetagcache.h"
#include "utils/tagpainter.h"
#include "utils/tagtextformat.h"
#include "data/tagproxyhandle.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/file/local/desktopfileinfo.h>

#include <dfm-framework/dpf.h>

#include <DMenu>
#include <QWidgetAction>
#include <QAbstractTextDocumentLayout>

Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(bool *)

DPTAG_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

TagManager::TagManager(QObject *parent)
    : QObject(parent),
      textObjectType(QTextFormat::UserObject + 1),
      tagPainter(new TagPainter)
{
    initializeConnection();
}

TagManager::~TagManager()
{
}

void TagManager::initializeConnection()
{
    connect(&FileTagCacheIns, &FileTagCacheController::newTagsAdded, this, &TagManager::onTagAdded);
    connect(&FileTagCacheIns, &FileTagCacheController::tagsDeleted, this, &TagManager::onTagDeleted);
    connect(&FileTagCacheIns, &FileTagCacheController::tagsColorChanged, this, &TagManager::onTagColorChanged);
    connect(&FileTagCacheIns, &FileTagCacheController::tagsNameChanged, this, &TagManager::onTagNameChanged);
    connect(&FileTagCacheIns, &FileTagCacheController::filesTagged, this, &TagManager::onFilesTagged);
    connect(&FileTagCacheIns, &FileTagCacheController::filesUntagged, this, &TagManager::onFilesUntagged);
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
    if (!url.isValid())
        return false;

    bool canTaged { true };
    if (dpfHookSequence->run("dfmplugin_tag", "hook_CanTaged", url, &canTaged)) {
        return canTaged;
    }

    QUrl localUrl;
    if (url.scheme() == Global::Scheme::kFile) {
        localUrl = url;
    } else {
        QList<QUrl> transUrls {};
        QList<QUrl> srcUrls { url };
        bool ok = UniversalUtils::urlsTransformToLocal(srcUrls, &transUrls);
        if (ok && !transUrls.isEmpty()) {
            localUrl = transUrls.first();
        }
    }

    if (!localUrl.isEmpty() && localUrl.scheme() == Global::Scheme::kFile) {
        auto info = InfoFactory::create<FileInfo>(localUrl);
        return localFileCanTagFilter(info);
    }

    return false;
}

bool TagManager::canTagFile(const FileInfoPointer &info) const
{
    if (info.isNull())
        return false;

    const QUrl &url = info->urlOf(UrlInfoType::kUrl);
    bool canTaged { true };
    if (dpfHookSequence->run("dfmplugin_tag", "hook_CanTaged", url, &canTaged)) {
        return canTaged;
    }

    bool canTag = localFileCanTagFilter(info);

    return canTag;
}

bool TagManager::paintListTagsHandle(int role, const FileInfoPointer &info, QPainter *painter, QRectF *rect)
{
    if (!canTagFile(info))
        return false;

    if (role != kItemFileDisplayNameRole && role != kItemNameRole)
        return false;

    QString path = info->pathOf(PathInfoType::kFilePath);
    path = FileUtils::bindPathTransform(path, false);
    const auto &tags = FileTagCacheIns.getTagsByFile(path);
    if (tags.isEmpty())
        return false;

    const auto &tagsColor = FileTagCacheIns.getCacheTagsColor(tags);
    if (!tagsColor.isEmpty()) {
        QRectF boundingRect(0, 0, (tagsColor.size() + 1) * kTagDiameter / 2, kTagDiameter);
        boundingRect.moveCenter(rect->center());
        boundingRect.moveRight(rect->right());

        TagHelper::instance()->paintTags(painter, boundingRect, tagsColor.values());

        rect->setRight(boundingRect.left() - 10);
    }

    return false;
}

bool TagManager::addIconTagsHandle(const FileInfoPointer &info, ElideTextLayout *layout)
{
    if (!canTagFile(info))
        return false;

    QString path = info->pathOf(PathInfoType::kFilePath);
    path = FileUtils::bindPathTransform(path, false);
    const auto &fileTags = FileTagCacheIns.getTagsByFile(path);
    if (fileTags.isEmpty())
        return false;

    const auto &tagsColor = FileTagCacheIns.getCacheTagsColor(fileTags);

    if (!tagsColor.isEmpty()) {
        auto document = layout->documentHandle();
        if (document) {
            document->documentLayout()->registerHandler(textObjectType, tagPainter);
            QTextCursor cursor(document);
            TagTextFormat format(textObjectType, tagsColor.values(), Qt::white);

            cursor.setPosition(0);
            cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
        }
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
        fmInfo() << "This tag name has registed: " << tagName;
        return false;
    }

    tagColorMap[tagName] = color;
    return true;
}

QString TagManager::getTagIconName(const QString &tag) const
{
    if (tag.isEmpty())
        return QString();

    const auto &dataMap = getTagsColorName({ tag });
    if (dataMap.contains(tag))
        return TagHelper::instance()->qureyIconNameByColor(QColor(dataMap.value(tag)));

    return QString();
}

TagManager::TagColorMap TagManager::getAllTags()
{
    const auto &dataMap = TagProxyHandleIns->getAllTags();
    TagColorMap result;
    auto it = dataMap.begin();

    for (; it != dataMap.end(); ++it)
        result[it.key()] = it.value().value<QColor>();

    return result;
}

TagManager::TagColorMap TagManager::getTagsColor(const QStringList &tags) const
{
    if (tags.isEmpty())
        return {};

    const auto &dataMap = TagProxyHandleIns->getTagsColor(tags);
    TagColorMap result;
    auto it = dataMap.begin();
    for (; it != dataMap.end(); ++it)
        if (it.value().isValid())
            result[it.key()] = QColor(it.value().toString());

    return result;
}

QStringList TagManager::getTagsByUrls(const QList<QUrl> &urls) const
{
    // single path:  get all tags of path
    // mult paths:  get same tags of paths
    if (urls.isEmpty())
        return {};

    QList<QUrl> realUrls;
    UniversalUtils::urlsTransformToLocal(urls, &realUrls);

    QStringList paths;
    for (const auto &url : TagHelper::commonUrls(realUrls)) {
        paths.append(url.path());
    }

    return FileTagCacheIns.getTagsByFiles(paths);
}

QStringList TagManager::getFilesByTag(const QString &tag)
{
    if (tag.isEmpty())
        return {};

    const auto &dataMap = TagProxyHandleIns->getFilesThroughTag({ tag });
    if (dataMap.isEmpty())
        return {};

    return dataMap.value(tag).toStringList();
}

QHash<QString, QStringList> TagManager::findChildren(const QString &parentPath) const
{
    const auto &url = FileUtils::bindUrlTransform(QUrl::fromLocalFile(parentPath));
    return FileTagCacheIns.findChildren(url.path());
}

bool TagManager::setTagsForFiles(const QStringList &tags, const QList<QUrl> &files)
{
    // if tags is empty means delete all files's tags
    if (files.isEmpty())
        return false;

    QList<QUrl> realUrls;
    UniversalUtils::urlsTransformToLocal(files, &realUrls);

    // set tags for mult files
    QStringList mutualTagNames = TagManager::instance()->getTagsByUrls(realUrls);
    // for deleting.
    QStringList dirtyTagNames;
    for (const QString &tag : mutualTagNames)
        if (!tags.contains(tag))
            dirtyTagNames << tag;

    bool result = false;
    if (!dirtyTagNames.isEmpty())
        result = TagManager::instance()->removeTagsOfFiles(dirtyTagNames, realUrls) || result;

    for (const QUrl &url : TagHelper::commonUrls(realUrls)) {
        QStringList tagsOfFile = TagManager::instance()->getTagsByUrls({ url });
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

    QList<QUrl> urls;
    UniversalUtils::urlsTransformToLocal(files, &urls);
    // tag --- color
    QMap<QString, QVariant> tagWithColor {};
    for (const QString &tagName : tags) {
        QString colorName = tagColorMap.contains(tagName) ? tagColorMap[tagName] : TagHelper::instance()->qureyColorByDisplayName(tagName).name();
        tagWithColor[tagName] = QVariant { QList<QString> { colorName } };
    }

    // make tag for files
    QVariant checkTagResult { TagProxyHandleIns->addTags(tagWithColor) };
    if (checkTagResult.toBool()) {
        QVariantMap infos;
        for (const auto &f : TagHelper::commonUrls(urls))
            infos[f.path()] = QVariant(tags);

        if (TagProxyHandleIns->addTagsForFiles(infos))
            return true;

        fmWarning() << "Create tags successfully! But failed to tag files";
    } else {
        fmWarning() << "The tag don't exist.";
    }
    return false;
}

bool TagManager::removeTagsOfFiles(const QList<QString> &tags, const QList<QUrl> &files)
{
    if (tags.isEmpty() || files.isEmpty())
        return false;

    QList<QUrl> urls;
    UniversalUtils::urlsTransformToLocal(files, &urls);

    QMap<QString, QVariant> fileWithTag;

    for (const QUrl &url : TagHelper::commonUrls(urls)) {
        fileWithTag[UrlRoute::urlToPath(url)] = QVariant(tags);
    }

    return TagProxyHandleIns->deleteFileTags(fileWithTag);
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
        TagManager::addTagsForFiles(QList<QString>() << tagInfo->tagName(), canTagFiles);
        return true;
    }

    return false;
}

void TagManager::hideFiles(const QList<QString> &tags, const QList<QUrl> &files)
{
    if (tags.isEmpty() || files.isEmpty())
        return;

    QMap<QString, QVariant> fileWithTag;

    for (const QUrl &url : files) {
        fileWithTag[UrlRoute::urlToPath(url)] = QVariant(tags);
    }

    emit filesHidden(fileWithTag);
}

void TagManager::deleteTags(const QStringList &tags)
{
    if (this->deleteTagData(tags, DeleteOpts::kTags)) {
        for (const auto &tag : tags) {
            QUrl url = TagHelper::instance()->makeTagUrlByTagName(tag);
            dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", url);
            emit tagDeleted(tag);
        }
    }
}

bool TagManager::changeTagColor(const QString &tagName, const QString &newTagColor)
{
    if (tagName.isEmpty() || newTagColor.isEmpty())
        return false;
    emit tagDeleted(tagName);
    QVariantMap changeMap { { tagName, QVariant { TagHelper::instance()->qureyColorByColorName(newTagColor).name() } } };
    return TagProxyHandleIns->changeTagsColor(changeMap);
}

bool TagManager::changeTagName(const QString &tagName, const QString &newName)
{
    if (tagName.isEmpty() || newName.isEmpty())
        return false;

    if (getAllTags().contains(newName)) {
        DialogManagerInstance->showRenameNameSameErrorDialog(newName);
        return false;
    }

    QVariantMap oldAndNewName = { { tagName, QVariant { newName } } };
    emit tagDeleted(tagName);
    return TagProxyHandleIns->changeTagNamesWithFiles(oldAndNewName);
}

bool TagManager::removeChildren(const QString &parentPath)
{
    bool ret = true;
    const auto &children = findChildren(parentPath);
    for (auto it = children.cbegin(); it != children.cend(); ++it) {
        ret &= removeTagsOfFiles(it.value(), { QUrl::fromLocalFile(it.key()) });
    }

    return ret;
}

bool TagManager::saveTrashFileTags(const QString &originalPath, qint64 fileInode, const QStringList &tagNames)
{
    if (originalPath.isEmpty() || fileInode <= 0 || tagNames.isEmpty())
        return false;

    // Save to database via D-Bus
    return TagProxyHandleIns->saveTrashFileTags(originalPath, fileInode, tagNames);
}

QStringList TagManager::getTrashFileTags(const QString &originalPath, qint64 fileInode)
{
    if (originalPath.isEmpty())
        return {};

    // Try cache first
    const auto &cachedTags = FileTagCacheIns.getTrashFileTags(originalPath, fileInode);
    if (!cachedTags.isEmpty())
        return cachedTags;

    // Query from database via D-Bus
    return TagProxyHandleIns->getTrashFileTags(originalPath, fileInode);
}

bool TagManager::removeTrashFileTags(const QString &originalPath, qint64 fileInode)
{
    if (originalPath.isEmpty())
        return false;

    // Remove from database via D-Bus
    return TagProxyHandleIns->removeTrashFileTags(originalPath, fileInode);
}

bool TagManager::clearAllTrashTags()
{
    // Clear database via D-Bus
    return TagProxyHandleIns->clearAllTrashTags();
}

QMap<QString, QString> TagManager::getTagsColorName(const QStringList &tags) const
{
    if (tags.isEmpty())
        return {};

    const auto &dataMap = TagProxyHandleIns->getTagsColor(tags);
    QMap<QString, QString> result;
    auto it = dataMap.begin();
    for (; it != dataMap.end(); ++it)
        result[it.key()] = it.value().toString();

    return result;
}

bool TagManager::deleteTagData(const QStringList &data, const DeleteOpts &type)
{
    if (data.isEmpty())
        return false;

    QMap<QString, QVariant> tagDelMap {};

    // Just to make sure the interface parameters are consistent, we only care about the values
    tagDelMap["deleteTagData"] = QVariant { data };
    bool ret = false;

    switch (type) {
    case DeleteOpts::kTags:
        ret = TagProxyHandleIns->deleteTags(tagDelMap);
        break;
    case DeleteOpts::kFiles:
        ret = TagProxyHandleIns->deleteFiles(tagDelMap);
        break;
    default:
        break;
    }

    return ret;
}

bool TagManager::localFileCanTagFilter(const FileInfoPointer &info) const
{
    if (info.isNull())
        return false;

    const QUrl &url = info->urlOf(UrlInfoType::kRedirectedFileUrl);
    if (!AnythingMonitorFilter::instance().whetherFilterCurrentPath(UrlRoute::urlParent(url).toLocalFile()))
        return false;

    const QString filePath { url.path() };
    const QString &compressPath { QDir::homePath() + "/.avfs/" };
    if (filePath.startsWith(compressPath))
        return false;

    const QString &parentPath { UrlRoute::urlParent(filePath).path() };
    if (parentPath == "/home" || parentPath == FileUtils::bindPathTransform("/home", true))
        return false;

    if (FileUtils::isDesktopFileSuffix(url)) {
        auto desktopInfo { dynamic_cast<DesktopFileInfo *>(info.data()) };
        if (desktopInfo)
            return desktopInfo->canTag();
    }

    if (ProtocolUtils::isSMBFile(url))
        return false;

    return !SystemPathUtil::instance()->isSystemPath(filePath);
}

QVariant TagManager::transformQueryData(const QDBusVariant &var) const
{
    QVariant variant { var.variant() };
    QDBusArgument argument { variant.value<QDBusArgument>() };
    QDBusArgument::ElementType curType { argument.currentType() };
    QVariantMap varMap {};

    if (curType == QDBusArgument::ElementType::MapType) {
        argument >> varMap;
        variant.setValue(varMap);
    }

    return variant;
}

void TagManager::contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    DMenu *menu = new DMenu;
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(menu), AcName::kAcSidebarTagitemMenu);
#endif
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
        QTimer::singleShot(200, [url, windowId] { dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_TriggerEdit", windowId, url); });
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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            tagsMap.unite(tagMap);
#else
            tagsMap.insert(tagMap);
#endif
        }
    }

    return tagsMap;
}

void TagManager::onTagAdded(const QVariantMap &tags)
{
    for (const QString &tag : tags.keys()) {
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

void TagManager::onTagColorChanged(const QVariantMap &tagAndColorName)
{
    auto it = tagAndColorName.begin();
    while (it != tagAndColorName.end()) {
        QUrl url = TagHelper::instance()->makeTagUrlByTagName(it.key());
        QString iconName = TagHelper::instance()->qureyIconNameByColor(QColor(it.value().toString()));
        QIcon icon = QIcon::fromTheme(iconName);
        QVariantMap map {
            { "Property_Key_Icon", icon },
            { "Property_Key_Editable", true }
        };
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", url, map);
        ++it;
    }
}

void TagManager::onTagNameChanged(const QVariantMap &oldAndNew)
{
    auto it = oldAndNew.begin();
    while (it != oldAndNew.end()) {
        QUrl &&url { TagHelper::instance()->makeTagUrlByTagName(it.key()) };
        auto &&map { TagHelper::instance()->createSidebarItemInfo(it.value().toString()) };
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", url, map);
        ++it;
    }
}

void TagManager::onFilesTagged(const QVariantMap &fileAndTags)
{
    if (!fileAndTags.isEmpty()) {
        TagEventCaller::sendFileUpdate(fileAndTags.firstKey());
    }

    emit filesTagged(fileAndTags);
}

void TagManager::onFilesUntagged(const QVariantMap &fileAndTags)
{
    if (!fileAndTags.isEmpty()) {
        TagEventCaller::sendFileUpdate(fileAndTags.firstKey());
    }

    emit filesUntagged(fileAndTags);
}
