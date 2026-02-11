// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defaultitemmanager.h"
#include "defaultitemmanager_p.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/lifecycle/lifecycle.h>

USING_IO_NAMESPACE

namespace dfmplugin_bookmark {

using namespace dfmplugin_bookmark;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DefaultItemManagerPrivate::DefaultItemManagerPrivate(DefaultItemManager *qq)
    : QObject(qq),
      q(qq)
{
}

DefaultItemManager *DefaultItemManager::instance()
{
    static DefaultItemManager instance;
    return &instance;
}

void DefaultItemManager::initDefaultItems()
{
    d->defaultItemInitOrder.clear();
    static QStringList defOrder = { "Home", "Desktop", "Videos", "Music", "Pictures", "Documents", "Downloads" };

    for (int i = 0; i < defOrder.count(); i++) {
        const QString &nameKey = defOrder.at(i);
        BookmarkData bookmarkData;
        bookmarkData.name = nameKey;   //For default item, save the english name to config.
        QString path { SystemPathUtil::instance()->systemPath(nameKey) };
        const QUrl &url { QUrl::fromLocalFile(path) };
        d->defaultItemUrls.insert(nameKey, url);
        bookmarkData.url = url;
        bookmarkData.isDefaultItem = true;
        bookmarkData.index = i;
        d->defaultItemInitOrder.append(bookmarkData);
    }
}

void DefaultItemManager::initPreDefineItems()
{
    DPF_USE_NAMESPACE
    d->defaultItemPreDefOrder.clear();

    const auto &ptrs = LifeCycle::pluginMetaObjs([](PluginMetaObjectPointer ptr) {
        Q_ASSERT(ptr);
        const auto &data { ptr->customData() };
        if (data.isEmpty())
            return false;
        if (ptr->customData().value("QuickAccessDisplay").toJsonArray().isEmpty())
            return false;
        return true;
    });

    std::for_each(ptrs.begin(), ptrs.end(), [this](PluginMetaObjectPointer ptr) {
        const auto &array { ptr->customData().value("QuickAccessDisplay").toJsonArray() };
        for (int i = 0; i != array.count(); ++i) {
            const auto &obj { array.at(i).toObject() };
            QString markName { obj.value("MarkName").toString() };
            QUrl url { obj.value("Url").toString() };
            if (!url.isValid() || markName.isEmpty())
                continue;
            BookmarkData quickAccessInfo;
            quickAccessInfo.name = markName;
            quickAccessInfo.url = url;
            quickAccessInfo.isDefaultItem = true;
            quickAccessInfo.index = obj.value("DefaultIndex").toInt();

            // parse Sidebar info
            QVariantMap properties;
            Qt::ItemFlags flags { Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren };
            const auto &sidebarObj { obj.value("SidebarInfo").toObject() };
            properties.insert("Property_Key_Url", url);
            properties.insert("Property_Key_DisplayName",
                              QObject::tr(qPrintable(sidebarObj.value("Name").toString())));
            properties.insert("Property_Key_VisiableControl",
                              sidebarObj.value("VisiableControl").toString());
            properties.insert("Property_Key_VisiableDisplayName",
                              sidebarObj.value("Name").toString());
            properties.insert("Property_Key_ReportName",
                              sidebarObj.value("ReportName").toString());
            properties.insert("Property_Key_Icon",
                              QIcon::fromTheme(sidebarObj.value("Icon").toString()));
            properties.insert("Property_Key_Group",
                              sidebarObj.value("Group").toString());
            properties.insert("Property_Key_QtItemFlags",
                              QVariant::fromValue(flags));
            quickAccessInfo.sidebarProperties = properties;
            d->defaultPreDefItemUrls.insert(markName, url);
            d->defaultItemPreDefOrder.append(quickAccessInfo);
        }
    });
    // 排序使得顺序为 index 的从大到小，为了使后续为尾插法，防止乱序
    std::sort(d->defaultItemPreDefOrder.begin(), d->defaultItemPreDefOrder.end(), [](const BookmarkData &a, const BookmarkData &b) {
        return a.index > b.index;
    });
}

QMap<QString, QUrl> DefaultItemManager::defaultItemUrls()
{
    return d->defaultItemUrls;
}

QMap<QString, QUrl> DefaultItemManager::preDefItemUrls()
{
    return d->defaultPreDefItemUrls;
}

QList<BookmarkData> DefaultItemManager::defaultItemInitOrder()
{
    return d->defaultItemInitOrder;
}

QList<BookmarkData> DefaultItemManager::defaultPreDefInitOrder()
{
    return d->defaultItemPreDefOrder;
}

bool DefaultItemManager::isDefaultItem(const BookmarkData &data)
{
    bool isNameKeyEqual = false;
    bool isUrlEqual = false;
    if (data.isDefaultItem) {
        isNameKeyEqual = DefaultItemManager::instance()->defaultItemUrls().keys().contains(data.name);
        return isNameKeyEqual;
    }
    isNameKeyEqual = DefaultItemManager::instance()->defaultItemUrls().keys().contains(data.name);
    isUrlEqual = DefaultItemManager::instance()->defaultItemUrls().values().contains(data.url);

    return isNameKeyEqual && isUrlEqual;
}

bool DefaultItemManager::isPreDefItem(const BookmarkData &data)
{
    bool ret { false };
    if (!data.isDefaultItem)
        return ret;

    for (const auto &preDefItem : defaultPreDefInitOrder()) {
        bool nameEqual { data.name == preDefItem.name };
        bool urlEqual { UniversalUtils::urlEquals(data.url, preDefItem.url) };
        if (nameEqual && urlEqual) {
            ret = true;
            break;
        }
    }

    return ret;
}

DefaultItemManager::DefaultItemManager(QObject *parent)
    : QObject(parent), d(new DefaultItemManagerPrivate(this))
{
}
}
