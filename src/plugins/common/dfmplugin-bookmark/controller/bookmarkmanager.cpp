// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bookmarkcallback.h"
#include "bookmarkmanager.h"
#include "defaultitemmanager.h"
#include "utils/bookmarkhelper.h"
#include "events/bookmarkeventcaller.h"
#include "utils/bookmarkhelper.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/event/event.h>

#include <DDialog>

#include <dfm-io/dfmio_utils.h>
#include <dfm-io/dfileinfo.h>

#include <QFileInfo>
#include <QMenu>
#include <QApplication>
#include <QStorageInfo>

using ItemClickedActionCallback = std::function<void(quint64 windowId, const QUrl &url)>;
using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;
using RenameCallback = std::function<void(quint64 windowId, const QUrl &url, const QString &name)>;

Q_DECLARE_METATYPE(QList<QUrl> *);
Q_DECLARE_METATYPE(ItemClickedActionCallback);
Q_DECLARE_METATYPE(ContextMenuCallback);
Q_DECLARE_METATYPE(RenameCallback);

USING_IO_NAMESPACE

namespace dfmplugin_bookmark {
static constexpr char kConfigGroupQuickAccess[] { "QuickAccess" };
static constexpr char kConfigGroupBookmark[] { "BookMark" };
static constexpr char kConfigKeyName[] { "Items" };

static constexpr char kKeyCreated[] { "created" };
static constexpr char kKeyLastModi[] { "lastModified" };
static constexpr char kKeyMountPoint[] { "mountPoint" };
static constexpr char kKeyName[] { "name" };
static constexpr char kKeyUrl[] { "url" };
static constexpr char kKeyIndex[] { "index" };
static constexpr char kKeydefaultItem[] { "defaultItem" };

static constexpr char kConfName[] { "org.deepin.dde.file-manager" };
static constexpr char kconfBookmark[] { "bookmark" };

void BookmarkData::resetData(const QVariantMap &map)
{
    created = QDateTime::fromString(map.value(kKeyCreated).toString(), Qt::ISODate);
    lastModified = QDateTime::fromString(map.value(kKeyLastModi).toString(), Qt::ISODate);
    deviceUrl = map.value(kKeyMountPoint).toString();
    name = map.value(kKeyName).toString();
    url = QUrl::fromUserInput(map.value(kKeyUrl).toString());
    index = map.value(kKeyIndex, -1).toInt();
    isDefaultItem = map.value(kKeydefaultItem, false).toBool();
}

QVariantMap BookmarkData::serialize()
{
    QVariantMap v;
    v.insert(kKeyCreated, created.toString(Qt::ISODate));
    v.insert(kKeyLastModi, lastModified.toString(Qt::ISODate));
    v.insert(kKeyMountPoint, deviceUrl);
    v.insert(kKeyName, name);
    v.insert(kKeyUrl, url);
    v.insert(kKeyIndex, index);
    v.insert(kKeydefaultItem, isDefaultItem);
    return v;
}

}

using namespace BookmarkCallBack;
using namespace dfmplugin_bookmark;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

BookMarkManager *BookMarkManager::instance()
{
    static BookMarkManager instance;
    return &instance;
}

bool BookMarkManager::removeBookMark(const QUrl &url)
{
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", url);

    if (!quickAccessDataMap.contains(url))
        return true;

    bool result = false;
    quickAccessDataMap.remove(url);
    sortedUrls.removeOne(url);
    QVariantList list = Application::genericSetting()->value(kConfigGroupQuickAccess, kConfigKeyName).toList();

    for (int i = 0; i < list.size(); ++i) {
        const QVariantMap &map = list.at(i).toMap();
        if (map.value(kKeyUrl).toUrl() == url) {
            list.removeAt(i);
            result = true;
        }
    }
    saveSortedItemsToConfigFile(sortedUrls);

    // remove data from dconfig
    removeBookmarkFromDConfig(url);

    return result;
}

bool BookMarkManager::addBookMark(const QList<QUrl> &urls)
{
    int count = urls.size();
    if (count < 0)
        return false;

    QList<QUrl> urlsTemp = urls;
    if (!urlsTemp.isEmpty()) {
        QList<QUrl> urlsTrans {};
        bool ok = UniversalUtils::urlsTransformToLocal(urlsTemp, &urlsTrans);

        if (ok && !urlsTrans.isEmpty())
            urlsTemp = urlsTrans;
    }

    for (const QUrl &url : urlsTemp) {
        QFileInfo info(url.path());
        if (info.isDir()) {
            BookmarkData bookmarkData;
            bookmarkData.created = QDateTime::currentDateTime();
            bookmarkData.lastModified = bookmarkData.created;
            getMountInfo(url, bookmarkData.deviceUrl);
            bookmarkData.name = info.fileName();
            bookmarkData.url = url;
            QString temPath = url.path();
            QUrl temUrl = url;
            temUrl.setPath(QUrl::fromPercentEncoding(temPath.toUtf8()));   // Convert to readable character.
            QString temName;
            int pos = temUrl.path().lastIndexOf('/') + 1;
            temName = temUrl.path().right(temUrl.path().length() - pos);

            if (isItemDuplicated(bookmarkData))
                continue;

            QVariantList list = Application::genericSetting()->value(kConfigGroupQuickAccess, kConfigKeyName).toList();
            bookmarkData.index = list.count();
            QVariantMap newData = bookmarkData.serialize();
            list << newData;   // append new bookmark data

            for (int i = 0; i < list.count(); i++) {
                QVariantMap map = list.at(i).toMap();
                map.insert(kKeyIndex, i);
                list.replace(i, map);
                quickAccessDataMap[map.value(kKeyUrl).toString()].index = i;
            }

            Application::genericSetting()->setValue(kConfigGroupQuickAccess, kConfigKeyName, list);
            quickAccessDataMap[url] = bookmarkData;
            sortedUrls.removeOne(url);
            sortedUrls.append(url);
            addBookMarkItem(url, info.fileName());

            // add data to dconfig
            newData.remove(kKeydefaultItem);   // bookmark data in dconfig dont have keys : `defaultItem` and `index`
            newData.remove(kKeyIndex);
            newData.insert(kKeyUrl, url.toEncoded());   // here must be encoded and keep compatibility
            addBookmarkToDConfig(newData);
        }
    }

    return true;
}

void BookMarkManager::addQuickAccessItemsFromConfig()
{
    sortedUrls.clear();
    initData();
    const QVariantList &list = Application::genericSetting()->value(kConfigGroupQuickAccess, kConfigKeyName).toList();
    if (list.count() <= 0 || !BookMarkHelper::instance()->isValidQuickAccessConf(list)) {
        fmWarning() << "Reset quick access list";
        saveQuickAccessToSortedItems(list);
        saveSortedItemsToConfigFile(sortedUrls);   // write the default items to config
    }

    addQuickAccessDataFromConfig();

    // Add items to sidebar according to `sortedUrls`
    for (const QUrl &url : sortedUrls) {
        const BookmarkData &data = quickAccessDataMap[url];
        addBookMarkItem(data.url, data.name, data.isDefaultItem);
    }
}

void BookMarkManager::addBookMarkItem(const QUrl &url, const QString &bookmarkName, bool isDefaultItem) const
{
    ItemClickedActionCallback cdCb { BookmarkCallBack::cdBookMarkUrlCallBack };
    ItemClickedActionCallback cdCbDef { BookmarkCallBack::cdDefaultItemUrlCallBack };
    ContextMenuCallback contextMenuCb { BookmarkCallBack::contextMenuHandle };
    RenameCallback renameCb { BookmarkCallBack::renameCallBack };

    QIcon bookmarkIcon;
    QString displayName;
    QUrl bookmarkUrl;
    QVariantMap map;
    const QString &group = "Group_Common";
    if (isDefaultItem) {   // default item and predef item
        if (DefaultItemManager::instance()->isPreDefItem(quickAccessDataMap[url])) {   // predef item
            map = quickAccessDataMap[url].sidebarProperties;   // Add plugin by cache data
            QUrl preDefUrl;
            preDefUrl.setScheme(url.scheme());
            preDefUrl.setPath(url.path());
            preDefUrl.setHost("");
            bookmarkUrl = preDefUrl;
        } else {   // default items
            QString iconName = SystemPathUtil::instance()->systemPathIconName(bookmarkName);
            if (!iconName.contains("-symbolic"))
                iconName.append("-symbolic");
            bookmarkIcon = QIcon::fromTheme(iconName);
            displayName = SystemPathUtil::instance()->systemPathDisplayName(bookmarkName);
            Qt::ItemFlags flags { Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled };
            const QString &path { SystemPathUtil::instance()->systemPath(bookmarkName) };
            bookmarkUrl = UrlRoute::pathToReal(path);

            map = {
                { "Property_Key_Group", group },
                { "Property_Key_DisplayName", displayName },
                { "Property_Key_Icon", bookmarkIcon },
                { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
                { "Property_Key_CallbackItemClicked", QVariant::fromValue(cdCbDef) },
                { "Property_Key_VisiableControl", bookmarkName.toLower() },
                { "Property_Key_ReportName", displayName },
            };
        }
    } else {   // bookmark
        bookmarkIcon = BookMarkHelper::instance()->icon();
        displayName = bookmarkName;
        bookmarkUrl = url;
        Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled };
        map = {
            { "Property_Key_Group", group },
            { "Property_Key_DisplayName", displayName },
            { "Property_Key_Icon", bookmarkIcon },
            { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
            { "Property_Key_CallbackItemClicked", QVariant::fromValue(cdCb) },
            { "Property_Key_VisiableControl", "hidden_me" },
            { "Property_Key_ReportName", kConfigGroupBookmark },
            { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) },
            { "Property_Key_CallbackRename", QVariant::fromValue(renameCb) },
            // 确保非默认书签逻辑可编辑和视图上的默认可编辑（上面的flags标志）一致，否则会出现不对书签进行修改的情况下，
            // 从新窗口打开的操作导致后续的视图可编辑标志都被移除，从而书签变的无法编辑
            { "Property_Key_Editable", true }
        };
    }

    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Add", bookmarkUrl, map);
}

BookMarkManager::BookMarkManager(QObject *parent)
    : QObject(parent)
{
}

void BookMarkManager::initData()
{
    // Init default item to quick access group
    const QList<BookmarkData> &defItemInitOrder { DefaultItemManager::instance()->defaultItemInitOrder() };
    int index = 0;
    for (const BookmarkData &data : defItemInitOrder) {
        BookmarkData temData = data;
        temData.index = index++;
        quickAccessDataMap[data.url] = temData;
        sortedUrls << data.url;
    }

    // from pre define infos
    const QList<BookmarkData> &defPreDefInitOrder { DefaultItemManager::instance()->defaultPreDefInitOrder() };
    for (const auto &data : defPreDefInitOrder) {
        quickAccessDataMap[data.url] = data;

        if (sortedUrls.size() < data.index || data.index < 0)
            sortedUrls.append(data.url);
        else
            sortedUrls.insert(data.index, data.url);
    }
}

bool BookMarkManager::bookMarkRename(const QUrl &url, const QString &newName)
{
    if (!url.isValid() || newName.isEmpty() || !quickAccessDataMap.contains(url))
        return false;

    QVariantList list = Application::genericSetting()->value(kConfigGroupQuickAccess, kConfigKeyName).toList();
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap map = list.at(i).toMap();
        if (quickAccessDataMap.contains(url) && url == map.value(kKeyUrl).toUrl()) {
            QString oldName = quickAccessDataMap[url].name;
            map[kKeyName] = newName;
            map[kKeyLastModi] = QDateTime::currentDateTime().toString(Qt::ISODate);
            quickAccessDataMap[url].name = newName;
            list.replace(i, map);
            Application::genericSetting()->setValue(kConfigGroupQuickAccess, kConfigKeyName, list);

            renameBookmarkToDConfig(oldName, newName);

            return true;
        }
    }

    return false;
}

QMap<QUrl, BookmarkData> BookMarkManager::getBookMarkDataMap() const
{
    return quickAccessDataMap;
}

int BookMarkManager::showRemoveBookMarkDialog(quint64 winId)
{
    auto window = FMWindowsIns.findWindowById(winId);
    if (!window) {
        fmCritical("can not find window");
        abort();
    }
    DDialog dialog(window);
    dialog.setTitle(tr("Sorry, unable to locate your quick access directory, remove it?"));
    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel", "button"));
    buttonTexts.append(tr("Remove", "button"));
    dialog.addButton(buttonTexts[0], true);
    dialog.addButton(buttonTexts[1], false, DDialog::ButtonRecommend);
    dialog.setDefaultButton(1);
    dialog.setIcon(QIcon::fromTheme("folder-bookmark", QIcon::fromTheme("folder")).pixmap(64, 64));
    return dialog.exec();
}

void BookMarkManager::getMountInfo(const QUrl &url, QString &mountPoint)
{
    QStorageInfo info(url.path());
    QString devStr(info.device());
    if (devStr.startsWith("/dev/")) {
        QUrl tmp;
        tmp.setScheme(Global::Scheme::kTrash);
        tmp.setPath(devStr);
        devStr = tmp.toString();
    } else if (devStr == "gvfsd-fuse") {
        if (info.bytesTotal() <= 0) {
            devStr = DFMIO::DFMUtils::devicePathFromUrl(url);
        }
    }
    mountPoint = devStr;
}

void BookMarkManager::saveSortedItemsToConfigFile(const QList<QUrl> &order)
{
    Q_ASSERT(!order.isEmpty());

    QVariantList sorted;
    int index = 0;
    for (auto url : order) {
        BookmarkData data = quickAccessDataMap.value(url);
        QVariantMap bData = data.serialize();
        bData.insert(kKeyIndex, index);
        data.resetData(bData);
        quickAccessDataMap.insert(url, data);
        sorted << bData;
        index++;
    }

    if (!sorted.isEmpty())
        Application::genericSetting()->setValue(kConfigGroupQuickAccess, kConfigKeyName, sorted);
}

void BookMarkManager::saveQuickAccessToSortedItems(const QVariantList &list)
{
    // 数据异常时，防止用户的书签丢失
    for (const QVariant &data : list) {
        const auto &bookMarkMap { data.toMap() };
        BookmarkData bookmarkData;
        bookmarkData.resetData(bookMarkMap);
        if (!bookmarkData.isDefaultItem) {
            if (!bookmarkData.url.isValid()) {
                fmWarning() << "Ignore invalid url quickaccess:" << bookMarkMap;
                continue;
            }
            quickAccessDataMap[bookmarkData.url] = bookmarkData;
            sortedUrls.append(bookmarkData.url);
        }
    }
}

void BookMarkManager::addQuickAccessDataFromConfig(const QVariantList &dataList)
{
    const QVariantList &list = dataList.isEmpty()
            ? Application::genericSetting()->value(kConfigGroupQuickAccess, kConfigKeyName).toList()
            : dataList;

    // 配置文件中包含的 item，在实际的场景中可以对应的插件并不会被加载，因此需要过滤
    // 预定义数据中的 index 也会被用户配置文件中的数据重写
    QList<QUrl> curSortedUrls;
    for (const QVariant &data : list) {
        QMap<QString, QVariant> bookMarkMap = data.toMap();
        bool isDataValid = bookMarkMap.contains(kKeyUrl) && !bookMarkMap.value(kKeyName).toString().isEmpty();
        if (!isDataValid)
            continue;

        BookmarkData bookmarkData;
        bookmarkData.resetData(bookMarkMap);

        // bookmark items
        if (!bookmarkData.isDefaultItem) {
            curSortedUrls << bookmarkData.url;
            quickAccessDataMap[bookmarkData.url] = bookmarkData;
            continue;
        }

        // default items
        if (DefaultItemManager::instance()->isDefaultItem(bookmarkData)) {
            bookmarkData.isDefaultItem = true;
            if (bookmarkData.index < 0) {   // If bookmarkData.index > 0, just keep the value from config since its order maybe changed by user.
                int index = quickAccessDataMap[bookmarkData.url].index;
                bookmarkData.index = index >= 0 ? index : -1;
            }
            curSortedUrls << bookmarkData.url;
            quickAccessDataMap[bookmarkData.url] = bookmarkData;
            continue;
        }

        // pre define items
        if (DefaultItemManager::instance()->isPreDefItem(bookmarkData)) {
            QUrl preDefUrl { bookmarkData.url };
            curSortedUrls << preDefUrl;
            bookmarkData.index = bookMarkMap.value(kKeyIndex).toInt();
            bookmarkData.sidebarProperties = quickAccessDataMap[preDefUrl].sidebarProperties;
            quickAccessDataMap[preDefUrl] = bookmarkData;
            continue;
        }
        fmWarning() << "QuickAccess hide item: " << bookmarkData.name;
    }

    sortedUrls = curSortedUrls;
}

void BookMarkManager::removeBookmarkFromDConfig(const QUrl &url)
{
    QVariantList list = DConfigManager::instance()->value(kConfName, kconfBookmark).toList();
    QVariantList removeList;
    for (int i = 0; i < list.size(); ++i) {
        const QVariantMap &map = list.at(i).toMap();
        if (map.value(kKeyUrl).toString() == url.toEncoded())
            removeList.append(map);
    }

    for (const QVariant &var : removeList)
        list.removeOne(var);

    if (!removeList.isEmpty())
        DConfigManager::instance()->setValue(kConfName, kconfBookmark, list);
}

void BookMarkManager::addBookmarkToDConfig(const QVariantMap &data)
{
    QVariantList list = DConfigManager::instance()->value(kConfName, kconfBookmark).toList();
    list.append(data);
    DConfigManager::instance()->setValue(kConfName, kconfBookmark, list);
}

/**
 * @brief BookMarkManager::renameBookmarkToDConfig
 * update the name and don't effect the url
 * @param oldName
 * @param newName
 */
void BookMarkManager::renameBookmarkToDConfig(const QString &oldName, const QString &newName)
{
    QVariantList list = DConfigManager::instance()->value(kConfName, kconfBookmark).toList();
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap map = list.at(i).toMap();
        if (map.value(kKeyName).toString() == oldName) {
            map[kKeyName] = newName;   // update name
            map[kKeyLastModi] = QDateTime::currentDateTime().toString(Qt::ISODate);
            list.replace(i, map);
            DConfigManager::instance()->setValue(kConfName, kconfBookmark, list);
            break;
        }
    }
}

void BookMarkManager::updateBookmarkUrlToDconfig(const QUrl &oldUrl, const QUrl &newUrl)
{
    QVariantList list = DConfigManager::instance()->value(kConfName, kconfBookmark).toList();
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap map = list.at(i).toMap();
        if (map.value(kKeyUrl).toString() == oldUrl.toEncoded()) {
            map[kKeyUrl] = newUrl.toEncoded();   // update url, here must be encoded
            map[kKeyLastModi] = QDateTime::currentDateTime().toString(Qt::ISODate);
            list.replace(i, map);
            DConfigManager::instance()->setValue(kConfName, kconfBookmark, list);
            break;
        }
    }
}

bool BookMarkManager::isItemDuplicated(const BookmarkData &data)
{
    QMapIterator<QUrl, BookmarkData> it(quickAccessDataMap);
    while (it.hasNext()) {
        it.next();
        if (it.value().name == data.name && UniversalUtils::urlEquals(it.value().url, data.url)) {
            fmInfo() << "Duplicated bookmark path :" << it.value().url;
            return true;
        }
    }

    return false;
}

/**
 * @brief BookMarkManager::fileRenamed
 * if the actual dir is renamed, would call this function.
 * note: here only update the url and don't effect the bookmark name
 * @param oldUrl
 * @param newUrl
 */
void BookMarkManager::fileRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (!oldUrl.isValid() || !quickAccessDataMap.contains(oldUrl))
        return;

    QVariantList list = Application::genericSetting()->value(kConfigGroupQuickAccess, kConfigKeyName).toList();
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap map = list.at(i).toMap();
        if (quickAccessDataMap.contains(oldUrl) && oldUrl == map.value(kKeyUrl).toUrl()) {
            map[kKeyUrl] = newUrl;
            list[i] = map;

            BookmarkData newData;
            newData.resetData(map);

            quickAccessDataMap.remove(oldUrl);
            quickAccessDataMap.insert(newUrl, newData);

            QVariantMap map {
                { "Property_Key_Url", newUrl },
                { "Property_Key_Editable", true }
            };
            dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", oldUrl, map);

            Application::genericSetting()->setValue(kConfigGroupQuickAccess, kConfigKeyName, list);
            updateBookmarkUrlToDconfig(oldUrl, newUrl);
            break;
        }
    }
}
