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
    index = map.value(kKeyIndex).toInt();
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

            if (isDublicated(temName))
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
    int index = 0;
    for (const QVariant &data : list) {
        QMap<QString, QVariant> bookMarkMap = data.toMap();
        if (bookMarkMap.contains(kKeyUrl)) {
            BookmarkData bookmarkData;
            bookmarkData.resetData(bookMarkMap);
            QString name = bookmarkData.name;
            if (!bookmarkData.url.isValid())
                continue;
            if (name.isEmpty()) {
                if (bookmarkData.url == QUrl("recent:/"))
                    name = tr("Recent");
                else if (bookmarkData.url == QUrl("trash:/"))
                    name = tr("Trash");
            }
            if (!defaultItemNames.keys().contains(bookmarkData.url))   //It's not a default item.
            {
                bookmarkData.index = index++;
                bookmarkDataMap[bookmarkData.url] = bookmarkData;
            } else {   //It's a default item.
                QPair pair = { name, index++ };   //Here we update the index to the same as configuration
                defaultItemNames.insert(bookmarkData.url, pair);
                continue;
            }
            addBookMarkItem(bookmarkData.url, name);
        }
    }
}

void BookMarkManager::addBookMarkItem(const QUrl &url, const QString &bookmarkName) const
{
    ItemClickedActionCallback cdCb { BookMarkManager::cdBookMarkUrlCallBack };
    ContextMenuCallback contextMenuCb { BookMarkManager::contextMenuHandle };
    RenameCallback renameCb { BookMarkManager::renameCallBack };
    Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled };
    QVariantMap map {
        { "Property_Key_Group", "Group_Common" },
        { "Property_Key_DisplayName", bookmarkName },
        { "Property_Key_Icon", BookMarkHelper::instance()->icon() },
        { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
        { "Property_Key_CallbackItemClicked", QVariant::fromValue(cdCb) },
        { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) },
        { "Property_Key_CallbackRename", QVariant::fromValue(renameCb) }
    };

    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Add", url, map);
}

QMap<QUrl, BookmarkData> BookMarkManager::bookmarkDataMap = {};
QMap<QUrl, QPair<QString, int>> BookMarkManager::defaultItemNames = {};
const QList<QPair<QString, QString>> BookMarkManager::defNames {
    QPair("Recent", tr("Recent")),
    QPair("Home", SystemPathUtil::instance()->systemPathDisplayName("Home")),
    QPair("Desktop", SystemPathUtil::instance()->systemPathDisplayName("Desktop")),
    QPair("Videos", SystemPathUtil::instance()->systemPathDisplayName("Videos")),
    QPair("Music", SystemPathUtil::instance()->systemPathDisplayName("Music")),
    QPair("Pictures", SystemPathUtil::instance()->systemPathDisplayName("Pictures")),
    QPair("Documents", SystemPathUtil::instance()->systemPathDisplayName("Documents")),
    QPair("Downloads", SystemPathUtil::instance()->systemPathDisplayName("Downloads")),
    QPair("Trash", SystemPathUtil::instance()->systemPathDisplayName("Trash"))
};
BookMarkManager::BookMarkManager(QObject *parent)
    : QObject(parent)
{
    // TODO: 配置文件内容改变后，当前并没有发送valueEdited信号
    connect(Application::genericSetting(), &Settings::valueEdited, this,
            &BookMarkManager::onFileEdited);

    QVariantList list = Application::genericSetting()->value(kConfigGroupName, kConfigKeyName).toList();
    if (list.isEmpty()) {
        QVariantList defaultList;
        int index = 0;
        for (int i = 0; i < defNames.count(); i++) {
            QPair<QString, QString> name = defNames.at(i);
            QString path { SystemPathUtil::instance()->systemPath(name.first) };
            QUrl url;
            if (name.first == "Trash") {
                url = QUrl("trash:/");
            } else if (name.first == "Recent")
                url = QUrl("recent:/");
            else
                url = { UrlRoute::pathToReal(path) };
            if (list.count() <= 0) {
                QVariantMap v;
                v.insert(kKeyName, name.first);
                v.insert(kKeyUrl, url);
                v.insert(kKeyIndex, index);
                defaultList << v;
            }
            defaultItemNames.insert(url, QPair { name.first, index++ });
        }
        Application::genericSetting()->setValue(kConfigGroupName, kConfigKeyName, defaultList);
    } else {
        int index = 0;
        for (const QVariant &data : list) {
            QMap<QString, QVariant> bookMarkMap = data.toMap();
            if (bookMarkMap.contains(kKeyUrl)) {
                BookmarkData bookmarkData;
                bookmarkData.resetData(bookMarkMap);
                QString name = bookmarkData.name;
                bool isDefaultName = false;
                QUrl url;
                for (int i = 0; i < defNames.count(); i++) {
                    QPair<QString, QString> defName = defNames.at(i);
                    QString path { SystemPathUtil::instance()->systemPath(defName.first) };

                    if (defName.first == "Trash")
                        url = QUrl("trash:/");
                    else if (defName.first == "Recent")
                        url = QUrl("recent:/");
                    else
                        url = { UrlRoute::pathToReal(path) };

                    if (defName.first == name) {
                        QVariantMap v;
                        v.insert(kKeyName, defName.first);
                        v.insert(kKeyUrl, url);
                        v.insert(kKeyIndex, index);
                        isDefaultName = true;
                        defaultItemNames.insert(url, QPair { defName.first, index++ });
                        break;
                    }
                }
                if (isDefaultName)
                    continue;
                if (!bookmarkData.url.isValid())
                    continue;
                bookmarkData.index = index++;
                bookmarkDataMap[url] = bookmarkData;
            }
        }
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

int BookMarkManager::defaultItemIndex(const QUrl &url)
{
    int index = -1;
    QMapIterator<QUrl, QPair<QString, int>> it(defaultItemNames);
    while (it.hasNext()) {
        it.next();
        if (it.key() == url) {
            index = it.value().second;
            break;
        }
    }

    return index;
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
        QString name = defaultItemNames.value(url).first;
        if (name.isEmpty()) {   //Do not a default item
            auto item = bookmarkDataMap.value(url);
            if (item.url.isValid()) {
                QVariantMap v = item.serialize();
                v.insert(kKeyIndex, index++);
                sorted << v;
            }
        } else {
            QVariantMap bData;
            name = defaultItemNames.value(url).first;
            bData.insert(kKeyName, name);
            bData.insert(kKeyUrl, url);
            bData.insert(kKeyIndex, index);
            QPair pair = { name, index++ };
            defaultItemNames.insert(url, pair);
            sorted << bData;
        }
    }

    Application::genericSetting()->setValue(kConfigGroupName, kConfigKeyName, sorted);
}

bool BookMarkManager::isDublicated(const QString &name)
{
    for (int i = 0; i < defNames.count(); i++) {
        if (defNames[i].second == name)
            return true;
    }

    QMapIterator<QUrl, BookmarkData> it(bookmarkDataMap);
    while (it.hasNext()) {
        it.next();
        if (it.value().name == name)
            return true;
    }
    return false;
}

QMap<QUrl, QPair<QString, int>> BookMarkManager::defaultNames() const
{
    return defaultItemNames;
}

bool BookMarkManager::handleItemSort(const QUrl &a, const QUrl &b)
{
    int aIndex = defaultItemIndex(a);
    if (aIndex < 0)
        aIndex = bookmarkDataMap.value(a).index;
    int bIndex = defaultItemIndex(b);
    if (bIndex < 0)
        bIndex = bookmarkDataMap.value(b).index;

    return aIndex < bIndex;
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
            if (locatePath.startsWith("/media")) {
                indexOfFirstDir = locatePath.lastIndexOf("/", locatePath.length() - 1);
            } else {
                indexOfFirstDir = locatePath.indexOf("/", 1);
            }
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

    if (isDublicated(name)) {
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
