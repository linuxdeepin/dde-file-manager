// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
static constexpr char kKeyLocateUrl[] { "locateUrl" };
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
    QByteArray ba;
    if (map.value(kKeyLocateUrl).toString().startsWith("/")) {
        ba = map.value(kKeyLocateUrl).toString().toLocal8Bit().toBase64();
    } else {
        ba = map.value(kKeyLocateUrl).toString().toLocal8Bit();
    }
    locateUrl = QString(ba);
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
    v.insert(kKeyLocateUrl, locateUrl);
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
    sortItemsByOrder(sortedUrls);

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
            getMountInfo(url, bookmarkData.deviceUrl, bookmarkData.locateUrl);
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

            //add data to dconfig
            newData.remove(kKeydefaultItem);   // bookmark data in dconfig dont have keys : `defaultItem` and `index`
            newData.remove(kKeyIndex);
            newData.insert(kKeyUrl, url.toEncoded());   //here must be encoded and keep compatibility
            newData.insert(kKeyLocateUrl, url.path().toUtf8().toBase64());
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
    if (list.count() <= 0)
        sortItemsByOrder(sortedUrls);   //write the default items to config

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
    if (isDefaultItem) {
        QString iconName = SystemPathUtil::instance()->systemPathIconName(bookmarkName);
        if (!iconName.contains("-symbolic"))
            iconName.append("-symbolic");
        bookmarkIcon = QIcon::fromTheme(iconName);
        displayName = SystemPathUtil::instance()->systemPathDisplayName(bookmarkName);
        Qt::ItemFlags flags { Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled };
        if (DefaultItemManager::instance()->isDefaultPluginItem(bookmarkName)) {   // It's a plugin item
            const QVariantMap &bookmarkMap = DefaultItemManager::instance()->pluginItemData().value(bookmarkName);
            bool isPluginItemAdded = bookmarkMap.contains("Property_Key_PluginItemData");
            if (!isPluginItemAdded)   // Plugin would be responsible for adding item to side bar.
                return;

            map = bookmarkMap.value("Property_Key_PluginItemData").toMap();   // Add plugin by cache data
            bookmarkUrl = bookmarkMap.value("Property_Key_Url").toString();
        } else {
            const QString &path { SystemPathUtil::instance()->systemPath(bookmarkName) };
            if (DefaultItemManager::instance()->isDefaultPluginItem(bookmarkName))
                bookmarkUrl = url;
            else
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
    } else {
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
            { "Property_Key_VisiableControl", bookmarkName.toLower() },
            { "Property_Key_ReportName", kConfigGroupBookmark },
            { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) },
            { "Property_Key_CallbackRename", QVariant::fromValue(renameCb) }
        };
    }

    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Add", bookmarkUrl, map);
}

BookMarkManager::BookMarkManager(QObject *parent)
    : QObject(parent)
{
    connect(Application::genericSetting(), &Settings::valueEdited, this,
            &BookMarkManager::onFileEdited);
    connect(DefaultItemManager::instance(), &DefaultItemManager::pluginItemDataAdded,
            [this](const QUrl &url, const QString &bookmarkName, bool isDefaultItem, int index) {
                if (!quickAccessDataMap.contains(url)) {
                    BookmarkData data = quickAccessDataMap[url];
                    data.index = index;
                    data.name = bookmarkName;
                    data.url = url;
                    data.isDefaultItem = isDefaultItem;
                    quickAccessDataMap[url] = data;

                    QVariantList list = Application::genericSetting()->value(kConfigGroupQuickAccess, kConfigKeyName).toList();

                    list.insert(index, quickAccessDataMap[url].serialize());
                    Application::genericSetting()->setValue(kConfigGroupQuickAccess, kConfigKeyName, list);
                    //plugin item is default item and dont need add to dconfig.
                }
            });
}

void BookMarkManager::initData()
{
    // Init default item to quick access group
    QList<BookmarkData> defItemInitOrder = DefaultItemManager::instance()->defaultItemInitOrder();
    int index = 0;
    foreach (const BookmarkData &data, defItemInitOrder) {
        BookmarkData temData = data;
        temData.index = index++;
        quickAccessDataMap[data.url] = temData;
        sortedUrls << data.url;
    }
}

void BookMarkManager::update(const QVariant &value)
{
    removeAllBookMarkSidebarItems();
    quickAccessDataMap.clear();
    sortedUrls.clear();

    initData();
    addQuickAccessDataFromConfig(value.toList());
    // Add items to sidebar according to `sortedUrls`
    for (const QUrl &url : sortedUrls) {
        const BookmarkData &data = quickAccessDataMap[url];
        addBookMarkItem(data.url, data.name, data.isDefaultItem);
    }
}

void BookMarkManager::removeAllBookMarkSidebarItems()
{
    QList<QUrl> bookmarkUrllist = quickAccessDataMap.keys();
    for (const QUrl &url : bookmarkUrllist) {
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", url);
    }
}

bool BookMarkManager::bookMarkRename(const QUrl &url, const QString &newName)
{
    if (!url.isValid() || newName.isEmpty() || !quickAccessDataMap.contains(url))
        return false;

    QVariantList list = Application::genericSetting()->value(kConfigGroupQuickAccess, kConfigKeyName).toList();
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap map = list.at(i).toMap();
        if (map.value(kKeyName).toString() == quickAccessDataMap[url].name) {
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
        qCritical("can not find window");
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

void BookMarkManager::getMountInfo(const QUrl &url, QString &mountPoint, QString &localUrl)
{
    Q_UNUSED(localUrl);

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

QSet<QString> BookMarkManager::getBookmarkDisabledSchemes()
{
    return bookmarkDisabledSchemes;
}

void BookMarkManager::sortItemsByOrder(const QList<QUrl> &order)
{
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

    Application::genericSetting()->setValue(kConfigGroupQuickAccess, kConfigKeyName, sorted);
}

void BookMarkManager::addQuickAccessDataFromConfig(const QVariantList &dataList)
{
    const QVariantList &list = dataList.isEmpty()
            ? Application::genericSetting()->value(kConfigGroupQuickAccess, kConfigKeyName).toList()
            : dataList;

    for (const QVariant &data : list) {
        QMap<QString, QVariant> bookMarkMap = data.toMap();
        bool isDataValid = bookMarkMap.contains(kKeyUrl) && !bookMarkMap.value(kKeyName).toString().isEmpty();
        if (!isDataValid)
            continue;

        BookmarkData bookmarkData;
        bookmarkData.resetData(bookMarkMap);

        if (DefaultItemManager::instance()->isDefaultItem(bookmarkData)) {
            bookmarkData.isDefaultItem = true;
            if (bookmarkData.index < 0) {   // If bookmarkData.index > 0, just keep the value from config since its order maybe changed by user.
                int index = quickAccessDataMap[bookmarkData.url].index;
                bookmarkData.index = index >= 0 ? index : -1;
            }
            sortedUrls.removeOne(bookmarkData.url);   // For updating the order
        } else if (DefaultItemManager::instance()->isDefaultPluginItem(bookmarkData.name)) {
            bookmarkData.isDefaultItem = true;
            bookmarkData.index = bookMarkMap.value(kKeyIndex).toInt();
            quickAccessDataMap[bookmarkData.url] = bookmarkData;
            sortedUrls.removeOne(bookmarkData.url);   // plugin is responsible for adding plugin item
            if (bookmarkData.index < 0)
                continue;
        } else {
            bookmarkData.isDefaultItem = false;
        }

        quickAccessDataMap[bookmarkData.url] = bookmarkData;
        sortedUrls << bookmarkData.url;
    }
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
            map[kKeyName] = newName;   //update name
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
            map[kKeyUrl] = newUrl.toEncoded();   //update url, here must be encoded
            map[kKeyLastModi] = QDateTime::currentDateTime().toString(Qt::ISODate);
            map[kKeyLocateUrl] = newUrl.path().toUtf8().toBase64();
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
            qInfo() << "Duplicated bookmark path :" << it.value().url;
            return true;
        }
    }

    return false;
}

void BookMarkManager::onFileEdited(const QString &group, const QString &key, const QVariant &value)
{
    if (group != kConfigGroupQuickAccess || key != kConfigKeyName)   // Only care about quick access group !!!
        return;

    update(value);
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
        if (map.value(kKeyName).toString() == quickAccessDataMap.value(oldUrl).name) {
            QString locatePath = newUrl.path();
            int indexOfFirstDir = 0;
            if (locatePath.startsWith("/media"))
                indexOfFirstDir = locatePath.lastIndexOf("/", locatePath.length() - 1);
            else
                indexOfFirstDir = locatePath.indexOf("/", 1);
            locatePath = locatePath.mid(indexOfFirstDir);
            const QByteArray &ba = locatePath.toLocal8Bit();
            map[kKeyLocateUrl] = QString(ba.toBase64());
            map[kKeyUrl] = newUrl;
            list[i] = map;

            BookmarkData newData;
            newData.resetData(map);

            quickAccessDataMap.remove(oldUrl);
            dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", oldUrl);

            quickAccessDataMap.insert(newUrl, newData);
            Application::genericSetting()->setValue(kConfigGroupQuickAccess, kConfigKeyName, list);
            update(list);

            updateBookmarkUrlToDconfig(oldUrl, newUrl);
            break;
        }
    }
}

void BookMarkManager::addSchemeOfBookMarkDisabled(const QString &scheme)
{
    bookmarkDisabledSchemes.insert(scheme);
}

bool BookMarkManager::handleItemSort(const QUrl &a, const QUrl &b)
{
    int aIndex = quickAccessDataMap.value(a).index;
    int bIndex = quickAccessDataMap.value(b).index;

    return aIndex <= bIndex;
}
