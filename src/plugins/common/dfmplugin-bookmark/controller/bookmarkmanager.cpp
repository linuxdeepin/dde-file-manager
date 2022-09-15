/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng <gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "bookmarkmanager.h"
#include "defaultitemmanager.h"
#include "utils/bookmarkhelper.h"
#include "events/bookmarkeventcaller.h"
#include "utils/bookmarkhelper.h"

#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <DDialog>

#include <dfmio_utils.h>
#include <dfm-io/core/dfileinfo.h>

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
static constexpr char kConfigGroupName[] { "BookMark" };
static constexpr char kConfigKeyName[] { "Items" };

static constexpr char kKeyCreated[] { "created" };
static constexpr char kKeyLastModi[] { "lastModified" };
static constexpr char kKeyLocateUrl[] { "locateUrl" };
static constexpr char kKeyMountPoint[] { "mountPoint" };
static constexpr char kKeyName[] { "name" };
static constexpr char kKeyUrl[] { "url" };
static constexpr char kKeyIndex[] { "index" };
static constexpr char kKeydefaultItem[] { "defaultItem" };

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
    defaultItem = map.value(kKeydefaultItem, false).toBool();
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
    v.insert(kKeydefaultItem, defaultItem);
    return v;
}

}

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

    if (!bookmarkDataMap.contains(url))
        return true;
    bookmarkDataMap.remove(url);

    QVariantList list = Application::genericSetting()->value(kConfigGroupName, kConfigKeyName).toList();
    for (int i = 0; i < list.size(); ++i) {
        const QVariantMap &map = list.at(i).toMap();
        if (map.value(kKeyUrl).toUrl() == url) {
            list.removeAt(i);
        }
    }
    Application::genericSetting()->setValue(kConfigGroupName, kConfigKeyName, list);

    return true;
}

bool BookMarkManager::addBookMark(const QList<QUrl> &urls)
{
    int count = urls.size();
    if (count < 0)
        return false;

    QList<QUrl> urlsTemp = urls;
    if (!urlsTemp.isEmpty()) {
        QList<QUrl> urlsTrans {};
        bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", urlsTemp, &urlsTrans);

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
            temUrl.setPath(QUrl::fromPercentEncoding(temPath.toUtf8()));   //Convert to readable character.
            QString temName;
            int pos = temUrl.path().lastIndexOf('/') + 1;
            temName = temUrl.path().right(temUrl.path().length() - pos);

            if (isNameDublicated(temName))
                return false;

            QVariantList list = Application::genericSetting()->value(kConfigGroupName, kConfigKeyName).toList();
            bookmarkData.index = list.count();
            list << bookmarkData.serialize();
            Application::genericSetting()->setValue(kConfigGroupName, kConfigKeyName, list);
            bookmarkDataMap[url] = bookmarkData;
            addBookMarkItem(url, info.fileName());
        }
    }

    return true;
}

void BookMarkManager::addBookMarkItemsFromConfig()
{
    QVariantList list = Application::genericSetting()->value(kConfigGroupName, kConfigKeyName).toList();
    QList<QUrl> sortedUrls;
    //Step 1: Add bookmark items from dde-file-manager.json
    for (const QVariant &data : list) {
        QMap<QString, QVariant> bookMarkMap = data.toMap();
        if (!bookMarkMap.contains(kKeyUrl))
            continue;
        BookmarkData bookmarkData;
        bookmarkData.resetData(bookMarkMap);
        QString name = bookmarkData.name;
        bool isDefaultItemNameInConfig = DefaultItemManager::instance()->defaultItemDisplayName().values().contains(name);   //defaultItemDisplayName.values() are translated name.
        if (isDefaultItemNameInConfig) {
            if (bookmarkData.index >= 0 && bookmarkData.defaultItem) {   //It's a default item in the dde-file-manager.json.
                BookmarkData initData = bookmarkDataMap.value(bookmarkData.url);
                initData.index = bookmarkData.index;   //Upate the init item index to the same as dde-file-manager.json
                bookmarkData = initData;
            } else {
                //For compatible issue, if there is already exsits a bookmark name which is the same as one
                //of default item's, just ignore it and it would be instead by default item in step 2.
                continue;
            }
        }
        bookmarkDataMap[bookmarkData.url] = bookmarkData;
        sortedUrls << bookmarkData.url;   //store all the added default items

        if (!DefaultItemManager::instance()->pluginItems().keys().contains(name))   //Plugin item is added by plugin.
            addBookMarkItem(bookmarkData.url, name, bookmarkData.defaultItem);
    }

    //Step 2: Add the default items to `sortedUrls` if they have not be added above.
    QList<BookmarkData> lastOnes;
    for (int index = DefaultItemManager::instance()->defaultItemInitOrder().count() - 1; index >= 0; index--) {
        QString name = DefaultItemManager::instance()->defaultItemInitOrder().at(index).name;
        QUrl url = DefaultItemManager::instance()->defaultItemInitOrder().at(index).url;
        qInfo() << "test url = " << url;
        qInfo() << "test name = " << name;
        if (sortedUrls.contains(url))   //Item already added in Step 1.
            continue;

        if (DefaultItemManager::instance()->pluginItems().keys().contains(name)) {   //Item is from other plugin.
            if (DefaultItemManager::instance()->pluginItems().value(name).index < 0)
                lastOnes << DefaultItemManager::instance()->pluginItems().value(name);
            else
                sortedUrls.insert(0, url);
        } else {
            sortedUrls.append(url);
            addBookMarkItem(url, name, DefaultItemManager::instance()->defaultItemInitOrder().at(index).defaultItem);
        }
    }

    foreach (const BookmarkData &data, lastOnes) {
        sortedUrls.append(data.url);
        //Here we do not call `addBookMarkItem()`,
        //since plugin is responsible for adding plugin item to sidebar.
    }

    //Step 3: Write sorted bookmark items to dde-file-manager.json
    sortItemsByOrder(sortedUrls);
}

void BookMarkManager::addBookMarkItem(const QUrl &url, const QString &bookmarkName, bool isDefaultItem) const
{
    ItemClickedActionCallback cdCb { BookMarkManager::cdBookMarkUrlCallBack };
    ItemClickedActionCallback cdCbDef { BookMarkManager::cdDefaultItemUrlCallBack };
    ContextMenuCallback contextMenuCb { BookMarkManager::contextMenuHandle };
    RenameCallback renameCb { BookMarkManager::renameCallBack };

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
        const QString &path { SystemPathUtil::instance()->systemPath(bookmarkName) };
        bookmarkUrl = UrlRoute::pathToReal(path);
        Qt::ItemFlags flags { Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled };
        map = {
            { "Property_Key_Group", group },
            { "Property_Key_DisplayName", displayName },
            { "Property_Key_Icon", bookmarkIcon },
            { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
            { "Property_Key_CallbackItemClicked", QVariant::fromValue(cdCbDef) },
            { "Property_Key_VisiableControl", bookmarkName.toLower() }
        };
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
            { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) },
            { "Property_Key_CallbackRename", QVariant::fromValue(renameCb) }
        };
    }

    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Add", bookmarkUrl, map);
}

QMap<QUrl, BookmarkData> BookMarkManager::bookmarkDataMap = {};
BookMarkManager::BookMarkManager(QObject *parent)
    : QObject(parent)
{
    // TODO: 配置文件内容改变后，当前并没有发送valueEdited信号
    connect(Application::genericSetting(), &Settings::valueEdited, this,
            &BookMarkManager::onFileEdited);
}

void BookMarkManager::initDefaultItems()
{
    DefaultItemManager::instance()->initDefaultItems();
    QList<BookmarkData> defItemInitOrder = DefaultItemManager::instance()->defaultItemInitOrder();
    foreach (const BookmarkData &data, defItemInitOrder) {
        bookmarkDataMap[data.url] = data;
    }
}

void BookMarkManager::update(const QVariant &value)
{
    removeAllBookMarkSidebarItems();

    bookmarkDataMap.clear();

    const QVariantList &list = value.toList();
    for (int i = 0; i < list.count(); ++i) {
        const QVariantMap &item = list.at(i).toMap();
        BookmarkData data;
        data.resetData(item);
        bookmarkDataMap[data.url] = data;
        addBookMarkItem(data.url, data.name);
    }
}

void BookMarkManager::removeAllBookMarkSidebarItems()
{
    QList<QUrl> bookmarkUrllist = bookmarkDataMap.keys();
    for (const QUrl &url : bookmarkUrllist) {
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", url);
    }
}

void BookMarkManager::bookMarkRename(const QUrl &url, const QString &newName)
{
    if (!url.isValid() || newName.isEmpty() || !bookmarkDataMap.contains(url))
        return;

    QVariantList list = Application::genericSetting()->value(kConfigGroupName, kConfigKeyName).toList();
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap map = list.at(i).toMap();
        if (map.value(kKeyName).toString() == bookmarkDataMap[url].name) {
            map[kKeyName] = newName;
            map[kKeyLastModi] = QDateTime::currentDateTime().toString(Qt::ISODate);
            list[i] = map;
            Application::genericSetting()->setValue(kConfigGroupName, kConfigKeyName, list);
            bookmarkDataMap[url].name = newName;
            break;
        }
    }
}

QMap<QUrl, BookmarkData> BookMarkManager::getBookMarkDataMap() const
{
    return bookmarkDataMap;
}

int BookMarkManager::showRemoveBookMarkDialog(quint64 winId)
{
    auto window = FMWindowsIns.findWindowById(winId);
    if (!window) {
        qCritical("can not find window");
        abort();
    }
    DDialog dialog(window);
    dialog.setTitle(tr("Sorry, unable to locate your bookmark directory, remove it?"));
    dialog.setMessage(" ");
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

QSet<QString> BookMarkManager::getBookMarkDisabledSchemes()
{
    return bookMarkDisabledSchemes;
}

void BookMarkManager::sortItemsByOrder(const QList<QUrl> &order)
{
    QVariantList sorted;
    int index = 0;
    for (auto url : order) {
        BookmarkData data = bookmarkDataMap.value(url);
        QVariantMap bData = data.serialize();
        bData.insert(kKeyIndex, index);
        data.resetData(bData);
        bookmarkDataMap.insert(url, data);
        sorted << bData;
        index++;
    }
    Application::genericSetting()->setValue(kConfigGroupName, kConfigKeyName, sorted);
}

bool BookMarkManager::isNameDublicated(const QString &name)
{
    QMapIterator<QUrl, BookmarkData> it(bookmarkDataMap);
    while (it.hasNext()) {
        it.next();
        if (it.value().defaultItem && DefaultItemManager::instance()->defaultItemDisplayName().values().contains(name)) {
            qInfo() << "Duplicated name as default item name:" << name;
            return true;
        }

        if (it.value().name == name) {
            qInfo() << "Duplicated name as existed bookmark name:" << name;
            return true;
        }
    }

    return false;
}

void BookMarkManager::onFileEdited(const QString &group, const QString &key, const QVariant &value)
{
    if (group != kConfigGroupName || key != kConfigKeyName)
        return;

    update(value);
}

void BookMarkManager::fileRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (!oldUrl.isValid() || !bookmarkDataMap.contains(oldUrl))
        return;

    QVariantList list = Application::genericSetting()->value(kConfigGroupName, kConfigKeyName).toList();
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap map = list.at(i).toMap();
        if (map.value(kKeyName).toString() == bookmarkDataMap.value(oldUrl).name) {
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

            bookmarkDataMap.remove(oldUrl);
            dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", oldUrl);

            bookmarkDataMap.insert(newUrl, newData);
            Application::genericSetting()->setValue(kConfigGroupName, kConfigKeyName, list);
            update(list);

            break;
        }
    }
}

void BookMarkManager::addSchemeOfBookMarkDisabled(const QString &scheme)
{
    bookMarkDisabledSchemes.insert(scheme);
}

void BookMarkManager::contextMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    QFileInfo info(url.path());
    bool bEnabled = info.exists();

    QMenu *menu = new QMenu;
    auto newWindowAct = menu->addAction(QObject::tr("Open in new window"), [url]() { BookMarkEventCaller::sendBookMarkOpenInNewWindow(url); });
    newWindowAct->setEnabled(bEnabled);

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        BookMarkEventCaller::sendBookMarkOpenInNewTab(windowId, url);
    });
    newTabAct->setEnabled(bEnabled && BookMarkEventCaller::sendCheckTabAddable(windowId));

    menu->addSeparator();

    auto renameAct = menu->addAction(QObject::tr("Rename"), [url, windowId]() {
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_TriggerEdit", windowId, url);
    });
    renameAct->setEnabled(bEnabled);

    menu->addAction(QObject::tr("Remove bookmark"),
                    [url]() { BookMarkManager::instance()->removeBookMark(url); });

    menu->addSeparator();

    auto propertyAct = menu->addAction(QObject::tr("Properties"),
                                       [url]() { BookMarkEventCaller::sendShowBookMarkPropertyDialog(url); });
    propertyAct->setEnabled(bEnabled);

    menu->exec(globalPos);
    delete menu;
}

void BookMarkManager::renameCallBack(quint64 windowId, const QUrl &url, const QString &name)
{
    Q_UNUSED(windowId);

    if (isNameDublicated(name)) {
        qInfo() << "new name = " << name;
        qInfo() << "New bookmark name would be dublicated as one of the existed ones.";
        return;
    }
    BookMarkManager::instance()->bookMarkRename(url, name);

    QVariantMap map {
        { "Property_Key_DisplayName", name },
        { "Property_Key_Editable", true }
    };
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", url, map);
}

void BookMarkManager::cdBookMarkUrlCallBack(quint64 windowId, const QUrl &url)
{
    QApplication::restoreOverrideCursor();

    const QMap<QUrl, BookmarkData> &bookmarkMap = BookMarkManager::instance()->getBookMarkDataMap();

    if (!bookmarkMap.contains(url)) {
        qCritical() << "boormark:"
                    << "not find the book mark!";
        return;
    }

    if (bookmarkMap[url].deviceUrl.startsWith(Global::Scheme::kSmb)
        || bookmarkMap[url].deviceUrl.startsWith(Global::Scheme::kFtp)
        || bookmarkMap[url].deviceUrl.startsWith(Global::Scheme::kSFtp)) {
        AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
        if (info && info->exists()) {
            if (info->isDir())
                BookMarkEventCaller::sendOpenBookMarkInWindow(windowId, url);
        } else {
            DeviceManager::instance()->mountNetworkDeviceAsync(bookmarkMap[url].deviceUrl, [windowId, url](bool ok, DFMMOUNT::DeviceError err, const QString &mntPath) {
                Q_UNUSED(mntPath)
                if (!ok) {
                    DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kMount, err);
                } else {
                    BookMarkEventCaller::sendOpenBookMarkInWindow(windowId, url);
                }
            });
        }
        return;
    }

    QFileInfo info(url.path());
    if (info.exists() && info.isDir()) {
        BookMarkEventCaller::sendOpenBookMarkInWindow(windowId, url);
    } else {
        if (DDialog::Accepted == BookMarkManager::instance()->showRemoveBookMarkDialog(windowId))
            BookMarkManager::instance()->removeBookMark(url);
    }
}

void BookMarkManager::cdDefaultItemUrlCallBack(quint64 windowId, const QUrl &url)
{
    BookMarkEventCaller::sendDefaultItemActived(windowId, url);
}

QString BookMarkManager::bookMarkActionCreatedCallBack(bool isNormal, const QUrl &currentUrl, const QUrl &focusFile, const QList<QUrl> &selected)
{
    Q_UNUSED(isNormal);
    Q_UNUSED(currentUrl);
    Q_UNUSED(focusFile);

    if (selected.size() != 1)
        return QString();

    QUrl url = selected.at(0);
    if (BookMarkManager::instance()->getBookMarkDisabledSchemes().contains(url.scheme()) || !QFileInfo(url.path()).isDir())
        return QString();

    if (BookMarkManager::instance()->getBookMarkDataMap().contains(url))
        return QString(tr("Remove from quick access"));
    else
        return QString(tr("Pin to quick access"));
}

void BookMarkManager::bookMarkActionClickedCallBack(bool isNormal, const QUrl &currentUrl, const QUrl &focusFile, const QList<QUrl> &selected)
{
    Q_UNUSED(isNormal);
    Q_UNUSED(currentUrl);
    Q_UNUSED(focusFile);

    if (selected.size() != 1)
        return;

    QUrl url = selected.at(0);
    if (!QFileInfo(url.path()).isDir())
        return;

    if (BookMarkManager::instance()->getBookMarkDataMap().contains(url))
        BookMarkManager::instance()->removeBookMark(url);
    else
        BookMarkManager::instance()->addBookMark(selected);
}

bool BookMarkManager::handleItemSort(const QUrl &a, const QUrl &b)
{
    int aIndex = bookmarkDataMap.value(a).index;
    int bIndex = bookmarkDataMap.value(b).index;

    return aIndex < bIndex;
}
