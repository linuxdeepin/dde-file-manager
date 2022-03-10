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
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "utils/bookmarkhelper.h"
#include "events/bookmarkeventcaller.h"
#include "utils/bookmarkhelper.h"

#include <DDialog>

#include <QFileInfo>
#include <QMenu>
#include <QApplication>

DPBOOKMARK_BEGIN_NAMESPACE
const char *const kConfigGroupName = "BookMark";
const char *const kConfigKeyName = "Items";

void BookmarkData::resetData(const QVariantMap &map)
{
    created = QDateTime::fromString(map.value("created").toString(), Qt::ISODate);
    lastModified = QDateTime::fromString(map.value("lastModified").toString(), Qt::ISODate);
    QByteArray ba;
    if (map.value("locateUrl").toString().startsWith("/")) {
        ba = map.value("locateUrl").toString().toLocal8Bit().toBase64();
    } else {
        ba = map.value("locateUrl").toString().toLocal8Bit();
    }
    locateUrl = QString(ba);
    mountPoint = map.value("mountPoint").toString();
    name = map.value("name").toString();
    url = QUrl::fromUserInput(map.value("url").toString());
}

DPBOOKMARK_END_NAMESPACE

DPBOOKMARK_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

BookMarkManager *BookMarkManager::instance()
{
    static BookMarkManager instance;
    return &instance;
}

bool BookMarkManager::removeBookMark(const QUrl &url)
{
    BookMarkHelper::sideBarServIns()->removeItem(url);

    if (!bookmarkDataMap.contains(url))
        return true;
    bookmarkDataMap.remove(url);

    QVariantList list =
            Application::genericSetting()->value(kConfigGroupName, kConfigKeyName).toList();
    for (int i = 0; i < list.size(); ++i) {
        const QVariantMap &map = list.at(i).toMap();
        if (map.value("url").toUrl() == url) {
            list.removeAt(i);
        }
    }
    Application::genericSetting()->setValue(kConfigGroupName, kConfigKeyName, list);

    return true;
}

bool BookMarkManager::addBookMark(const QList<QUrl> &urls) const
{
    int count = urls.size();
    if (count < 0)
        return false;
    for (const QUrl &url : urls) {
        QFileInfo info(url.path());
        if (info.isDir()) {
            BookmarkData bookmarkData;
            bookmarkData.created = QDateTime::currentDateTime();
            bookmarkData.lastModified = bookmarkData.created;
            bookmarkData.locateUrl = "";
            bookmarkData.mountPoint = "";
            bookmarkData.name = info.fileName();
            bookmarkData.url = url;
            QVariantList list =
                    Application::genericSetting()->value(kConfigGroupName, kConfigKeyName).toList();
            list << QVariantMap { { "name", bookmarkData.name },
                                  { "url", bookmarkData.url },
                                  { "created", bookmarkData.created.toString(Qt::ISODate) },
                                  { "lastModified",
                                    bookmarkData.lastModified.toString(Qt::ISODate) },
                                  { "mountPoint", bookmarkData.mountPoint },
                                  { "locateUrl", bookmarkData.locateUrl } };
            Application::genericSetting()->setValue(kConfigGroupName, kConfigKeyName, list);

            bookmarkDataMap[url] = bookmarkData;
            addBookMarkItem(url, info.fileName());
        }
    }

    return true;
}

void BookMarkManager::addBookMarkItemsFromConfig()
{
    QVariantList list =
            Application::genericSetting()->value(kConfigGroupName, kConfigKeyName).toList();
    for (const QVariant &data : list) {
        QMap<QString, QVariant> bookMarkMap = data.toMap();
        if (bookMarkMap.contains("url")) {
            BookmarkData bookmarkData;
            bookmarkData.resetData(bookMarkMap);
            const QString &name = bookMarkMap.value("name").toString();
            bookmarkDataMap[bookmarkData.url] = bookmarkData;
            addBookMarkItem(bookmarkData.url, name);
        }
    }
}

void BookMarkManager::addBookMarkItem(const QUrl &url, const QString &bookmarkName) const
{
    SideBar::ItemInfo item;
    item.group = SideBar::DefaultGroup::kBookmark;
    item.url = url;
    item.iconName = BookMarkHelper::instance()->icon().name();
    item.text = bookmarkName;
    item.flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    item.contextMenuCb = BookMarkManager::contenxtMenuHandle;
    item.renameCb = BookMarkManager::renameCB;
    item.cdCb = BookMarkManager::cdBookMarkUrlCB;

    BookMarkHelper::sideBarServIns()->addItem(item);
}

BookMarkManager::BookMarkManager(QObject *parent)
    : QObject(parent)
{
    // TODO: 配置文件内容改变后，当前并没有发送valueEdited信号
    connect(Application::genericSetting(), &Settings::valueEdited, this,
            &BookMarkManager::onFileEdited);
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
    }
}

void BookMarkManager::removeAllBookMarkSidebarItems()
{
    QList<QUrl> bookmarkUrllist = bookmarkDataMap.keys();
    for (const QUrl &url : bookmarkUrllist) {
        BookMarkHelper::sideBarServIns()->removeItem(url);
    }
}

void BookMarkManager::bookMarkRename(const QUrl &url, const QString &newName)
{
    if (!url.isValid() || newName.isEmpty() || !bookmarkDataMap.contains(url))
        return;

    QVariantList list =
            Application::genericSetting()->value(kConfigGroupName, kConfigKeyName).toList();
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap map = list.at(i).toMap();
        if (map.value("name").toString() == bookmarkDataMap[url].name) {
            map["name"] = newName;
            map["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
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
    auto window = BookMarkHelper::winServIns()->findWindowById(winId);
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

    QVariantList list =
            Application::genericSetting()->value(kConfigGroupName, kConfigKeyName).toList();
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap map = list.at(i).toMap();
        if (map.value("name").toString() == bookmarkDataMap.value(oldUrl).name) {
            QString locatePath = newUrl.path();
            int indexOfFirstDir = 0;
            if (locatePath.startsWith("/media")) {
                indexOfFirstDir = locatePath.lastIndexOf("/", locatePath.length() - 1);
            } else {
                indexOfFirstDir = locatePath.indexOf("/", 1);
            }
            locatePath = locatePath.mid(indexOfFirstDir);
            QByteArray ba = locatePath.toLocal8Bit().toBase64();
            map["locateUrl"] = QString(ba);
            map["url"] = newUrl;
            list[i] = map;

            BookmarkData newData;
            newData.resetData(map);

            bookmarkDataMap.remove(oldUrl);
            bookmarkDataMap.insert(newUrl, newData);

            Application::genericSetting()->setValue(kConfigGroupName, kConfigKeyName, list);
            BookMarkHelper::sideBarServIns()->removeItem(oldUrl);
            // TODO:(gongheng) 此处应该是插入新的item到原来的位置
            addBookMarkItem(newUrl, bookmarkDataMap[newUrl].name);

            break;
        }
    }
}

void BookMarkManager::contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    QFileInfo info(url.path());
    bool bEnabled = info.exists();

    QMenu *menu = new QMenu;
    auto newWindowAct = menu->addAction(QObject::tr("Open in new window"),
                                        [url]() { BookMarkEventCaller::sendBookMarkOpenInNewWindow(url); });
    newWindowAct->setEnabled(bEnabled);

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        BookMarkEventCaller::sendBookMarkOpenInNewTab(windowId, url);
    });
    newTabAct->setEnabled(bEnabled && BookMarkHelper::workspaceServIns()->tabAddable(windowId));

    menu->addSeparator();

    auto renameAct = menu->addAction(QObject::tr("Rename"), [url, windowId]() {
        BookMarkHelper::sideBarServIns()->triggerItemEdit(windowId, url);
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

void BookMarkManager::renameCB(quint64 windowId, const QUrl &url, const QString &name)
{
    Q_UNUSED(windowId);
    BookMarkManager::instance()->bookMarkRename(url, name);
    BookMarkHelper::sideBarServIns()->updateItem(url, name, true);
}

void BookMarkManager::cdBookMarkUrlCB(quint64 windowId, const QUrl &url)
{
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));

    QFileInfo info(url.path());
    if (info.exists() && info.isDir()) {
        BookMarkEventCaller::sendOpenBookMarkInWindow(windowId, url);
    } else {
        if (BookMarkManager::instance()->showRemoveBookMarkDialog(windowId))
            BookMarkManager::instance()->removeBookMark(url);
    }
}

QString BookMarkManager::bookMarkActionCreatedCB(bool isNormal, const QUrl &currentUrl, const QUrl &focusFile, const QList<QUrl> &selected)
{
    Q_UNUSED(isNormal);
    Q_UNUSED(currentUrl);
    Q_UNUSED(focusFile);

    if (selected.size() != 1)
        return QString();

    QUrl url = selected.at(0);
    if (!QFileInfo(url.path()).isDir())
        return QString();

    if (BookMarkManager::instance()->getBookMarkDataMap().contains(url))
        return QString(tr("Remove bookmark"));
    else
        return QString(tr("Add to bookmark"));
}

void BookMarkManager::bookMarkActionClickedCB(bool isNormal, const QUrl &currentUrl, const QUrl &focusFile, const QList<QUrl> &selected)
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
