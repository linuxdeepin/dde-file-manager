// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bookmarkupgradeunit.h"
#include "bookmarkupgrade/defaultitemmanager.h"
#include <dfm-base/base/urlroute.h>
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
    // `bookmarkOrderList` is the bookmark sort data from config.
    QStringList bookmarkOrderList = bookmarkOrderFromConfig.toStringList();
    // `bookmarkDataList` is the bookmark raw data from config, without sort.
    const QVariantList &bookmarkDataList = configObject.value(kConfigGroupBookmark).toObject().value(kConfigKeyName).toArray().toVariantList();
    // 1. prepare a `bookmarkDataMap` from `bookmarkDataList`
    QVariantMap bookmarkDataMap;
    for (const QVariant &var : bookmarkDataList) {
        const QUrl &url = var.toHash().value(kKeyUrl).toUrl();
        const QString &bookmarkName = var.toHash().value(kKeyName).toString();
        if (url.isEmpty() || bookmarkName.isEmpty())   // check the bookmark raw data
            continue;

        const QVariantHash &item = var.toHash();
        bookmarkDataMap.insert(url.toString(), item);
    }

    auto parseUrlFromOrderData = [](const QString &src) {
        QString bookmarkOrderItem = src;
        bookmarkOrderItem.remove(0, 9);   //remove `bookmark:`
        QUrl url(bookmarkOrderItem);
        QString urlString = url.toString(QUrl::RemoveFragment | QUrl::RemoveQuery);   //remove ?# or #
        return urlString;
    };
    // 2. sort the bookmark data to `sortedBookmarkOrderList` according to `bookmarkDataList`
    QVariantList sortedBookmarkOrderList;
    int increasedIndex = quickAccessItemList.count();
    while (bookmarkOrderList.count() > 0) {
        const QString &var = bookmarkOrderList.takeFirst();
        const QString &urlString = parseUrlFromOrderData(var);
        QVariantHash item = bookmarkDataMap.value(urlString).toHash();
        if (!item.value("name").toString().isEmpty()) {
            item.insert(kIndex, increasedIndex++);
            item.insert(kKeydefaultItem, false);
            QString urlString = item.value("url").toUrl().toString();
            item.insert("url", urlString);
            sortedBookmarkOrderList.append(item);
        }
    }
    // 3. if do not get the `sortedBookmarkOrderList` for some reason, also fill it with unsort bookmark data from `bookmarkDataMap`
    if (sortedBookmarkOrderList.isEmpty()) {
        increasedIndex = quickAccessItemList.count();
        for (const QVariant &var : bookmarkDataMap.values()) {
            QVariantHash item = var.toHash();
            item.insert(kIndex, increasedIndex++);
            item.insert(kKeydefaultItem, false);
            sortedBookmarkOrderList.append(item);
        }
        qInfo() << "sortedBookmarkOrderList.count = " << sortedBookmarkOrderList.count();
        qInfo() << "Warning: Do not get the bookmark order data from config, so transfer the bookmark raw data without sort as well.";
    }

    qInfo() << "before: quickAccessItemList.count = " << quickAccessItemList.count();
    // 4. append the final sort data to `quickAccessItemList` which is for writting to config in field `QuickAccess`
    for (const QVariant &item : sortedBookmarkOrderList) {
        QVariantHash data = item.toHash();
        QString readableStr = data.value("url").toUrl().toString();   // convert url to human readable string.
        data.insert("url", readableStr);
        quickAccessItemList.append(data);
        qInfo() << "Bookmark raw data is sorded to quickAccessItemList";
    }
    qInfo() << "after: quickAccessItemList.count = " << quickAccessItemList.count();
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
