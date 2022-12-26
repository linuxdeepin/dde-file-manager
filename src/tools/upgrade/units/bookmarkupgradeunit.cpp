/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu<zhuangshu@uniontech.com>
 *
 * Maintainer: zhuangshu<zhuangshu@uniontech.com>
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
#include "bookmarkupgradeunit.h"
#include "bookmarkupgrade/defaultitemmanager.h"
#include "dfm-base/base/urlroute.h"
#include <QDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QStandardPaths>
#include <QVariantMap>

using namespace dfm_upgrade;
DFMBASE_USE_NAMESPACE
static constexpr char kConfigGroupQuickAccess[] { "QuickAccess" };
static constexpr char kBookmarkOrder[] { "SideBar/ItemOrder" };
static constexpr char kConfigGroupBookmark[] { "BookMark" };
static constexpr char kBookmark[] { "bookmark" };
static constexpr char kConfigKeyName[] { "Items" };
static constexpr char kIndex[] { "index" };
static constexpr char kKeyName[] { "name" };
static constexpr char kKeyUrl[] { "url" };
static constexpr char kKeyIndex[] { "index" };
static constexpr char kKeydefaultItem[] { "defaultItem" };
static constexpr char kKeyCreated[] { "created" };
static constexpr char kKeyLastModi[] { "lastModified" };
static constexpr char kKeyLocateUrl[] { "locateUrl" };
static constexpr char kKeyMountPoint[] { "mountPoint" };

static QString kConfigurationPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first() + "/deepin/dde-file-manager.json";

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
BookMarkUpgradeUnit::BookMarkUpgradeUnit()
    : UpgradeUnit()
{
}

QString BookMarkUpgradeUnit::name()
{
    return "BookMarkUpgradeUnit";
}

bool BookMarkUpgradeUnit::initialize(const QMap<QString, QString> &args)
{
    Q_UNUSED(args)
    qInfo() << "begin upgrade";
    QFile file(kConfigurationPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QByteArray data = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    configObject = doc.object();

    if (configObject.keys().contains(kConfigGroupQuickAccess)) {
        return false;
    } else {
        DefaultItemManager::instance()->initDefaultItems();
        return true;
    }
}

bool BookMarkUpgradeUnit::upgrade()
{
    qInfo() << "upgrading";
    const QVariantList &quickAccessItems = initData();
    doUpgrade(quickAccessItems);   //generate quick access field to configuration

    return true;
}

void BookMarkUpgradeUnit::completed()
{
    qInfo() << "completed";
}

QVariantList BookMarkUpgradeUnit::initData() const
{
    QVariantList quickAccessItemList;
    QList<BookmarkData> defItemInitOrder = DefaultItemManager::instance()->defaultItemInitOrder();
    int index = 0;
    foreach (const BookmarkData &data, defItemInitOrder) {
        BookmarkData temData = data;
        if (data.name.isEmpty())
            continue;
        temData.index = index++;
        const QVariantMap &item = temData.serialize();
        quickAccessItemList.append(item);
    }

    const QVariant &bookmarkOrderFromConfig = configObject.value(kBookmarkOrder).toObject().value(kBookmark).toArray().toVariantList();
    QStringList bookmarkOrderList = bookmarkOrderFromConfig.toStringList();

    const QVariantList &bookmarkDataList = configObject.value(kConfigGroupBookmark).toObject().value(kConfigKeyName).toArray().toVariantList();
    QVariantMap bookmarkDataMap;
    for (const QVariant &var : bookmarkDataList) {
        const QUrl &url = var.toHash().value(kKeyUrl).toUrl();
        const QString &urlString = url.toString();
        const QString &fileName = urlString.mid(urlString.lastIndexOf("/") + 1);
        if (var.toHash().value(kKeyName).toString().isEmpty() || fileName.isEmpty())
            continue;
        const QString &compareName = QString("%1:%2#%3").arg(kBookmark).arg(urlString).arg(fileName);
        if (!url.isValid() && bookmarkOrderList.contains(compareName)) {
            bookmarkOrderList.removeOne(compareName);
            qInfo() << "bookmark url is invalid but it is included in the bookmark order : " << url;
            continue;
        }
        const QVariantHash &item = var.toHash();
        bookmarkDataMap.insert(url.toString(), item);
    }

    auto getUrlFromOrder = [](const QString &src, const QString &start, const QString &end) {
        int startIndex = src.indexOf(start, 0) + 1;
        int endIndex = src.indexOf(end, startIndex);
        const QString &subStr = src.mid(startIndex, endIndex - startIndex);
        return subStr;
    };
    while (bookmarkOrderList.count() > 0) {
        const QString &var = bookmarkOrderList.takeFirst();
        const QString &fileName = var.mid(var.lastIndexOf("#"));
        const QString &middleString = getUrlFromOrder(var, ":", fileName);
        QVariantHash item = bookmarkDataMap.value(middleString).toHash();
        if (item.value("name").toString().isEmpty())
            continue;
        item.insert(kIndex, quickAccessItemList.count());
        item.insert(kKeydefaultItem, false);
        quickAccessItemList.append(item);
    }

    return quickAccessItemList;
}
bool BookMarkUpgradeUnit::doUpgrade(const QVariantList &quickAccessDatas)
{
    QFile file(kConfigurationPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QJsonObject quickAccess;
    quickAccess.insert(kConfigKeyName, QJsonArray::fromVariantList(quickAccessDatas));
    configObject.insert(kConfigGroupQuickAccess, quickAccess);
    QJsonDocument doc(configObject);
    QByteArray data = doc.toJson();
    file.write(data);
    file.close();

    return true;
}
