// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "config.h"   //cmake
#include "defaultitemmanager.h"
#include "defaultitemmanager_p.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/systempathutil.h>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)

#include <dfm-framework/lifecycle/lifecycle.h>

USING_IO_NAMESPACE

using namespace dfm_upgrade;
DFMBASE_USE_NAMESPACE

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
        QString path { SystemPathUtil::instance()->systemPath(nameKey) };
        const QString &displayName = SystemPathUtil::instance()->systemPathDisplayName(nameKey);
        BookmarkData bookmarkData;
        bookmarkData.name = nameKey;   //For default item, save the english name to config.
        bookmarkData.isDefaultItem = true;
        bookmarkData.index = i;
        bookmarkData.transName = displayName;
        bookmarkData.url = QUrl::fromLocalFile(path);
        d->defaultItemInitOrder.append(bookmarkData);
    }
}

void DefaultItemManager::initPreDefineItems()
{
    DPF_USE_NAMESPACE
    static constexpr char kFmPluginInterface[] { "org.deepin.plugin.filemanager" };
    static constexpr char kCommonPluginInterface[] { "org.deepin.plugin.common" };
    QStringList pluginsDirs;

#ifdef QT_DEBUG
    const QString &pluginsDir { DFM_BUILD_PLUGIN_DIR };
    qCInfo(logToolUpgrade) << QString("Load plugins path (Debug mode): %1").arg(pluginsDir);
    pluginsDirs.push_back(pluginsDir + "/filemanager");
    pluginsDirs.push_back(pluginsDir + "/common");
    pluginsDirs.push_back(pluginsDir);
#else
    pluginsDirs << QString(DFM_PLUGIN_COMMON_CORE_DIR)
                << QString(DFM_PLUGIN_FILEMANAGER_CORE_DIR)
                << QString(DFM_PLUGIN_COMMON_EDGE_DIR)
                << QString(DFM_PLUGIN_FILEMANAGER_EDGE_DIR);
#endif

    // just read json
    LifeCycle::initialize({ kFmPluginInterface, kCommonPluginInterface }, pluginsDirs);
    LifeCycle::readPlugins();
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
            d->defaultItemPreDefOrder.append(quickAccessInfo);
        }
    });
}

QList<BookmarkData> DefaultItemManager::defaultItemInitOrder()
{
    return d->defaultItemInitOrder;
}

QList<BookmarkData> DefaultItemManager::defaultPreDefInitOrder()
{
    return d->defaultItemPreDefOrder;
}

DefaultItemManager::DefaultItemManager(QObject *parent)
    : QObject(parent), d(new DefaultItemManagerPrivate(this))
{
}
