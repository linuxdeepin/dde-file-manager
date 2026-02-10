// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "protocoldisplayutilities.h"
#include "typedefines.h"
#include "displaycontrol/datahelper/virtualentrydbhandler.h"
#include "displaycontrol/info/protocolvirtualentryentity.h"
#include "displaycontrol/protocoldevicedisplaymanager.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/base/device/devicealiasmanager.h>
#include <dfm-framework/event/event.h>

#include <DMenu>

#include <QApplication>
#include <QSettings>

#undef signals
extern "C" {
#include <libsecret/secret.h>
}
#define signals public

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace plugin_events {
static constexpr char kComputerEventNS[] { "dfmplugin_computer" };
static constexpr char kCptSlotAdd[] { "slot_Item_Add" };
static constexpr char kCptSlotRemove[] { "slot_Item_Remove" };
static constexpr char kCptSlotRefresh[] { "slot_View_Refresh" };
static constexpr char kCptSlotForget[] { "slot_Passwd_Clear" };

static constexpr char kSidebarEventNS[] { "dfmplugin_sidebar" };
static constexpr char kSbSlotAdd[] { "slot_Item_Add" };
static constexpr char kSbSlotRemove[] { "slot_Item_Remove" };
}   // namespace plugin_events
using namespace plugin_events;

namespace plugin_menu_names {
static constexpr char kComputerMenu[] { "ComputerMenu" };
}   // namespace plugin_menu_names
using namespace plugin_menu_names;

namespace AcName {
inline constexpr char kAcSidebarDeviceMenu[] { "sidebar_deviceitem_menu" };
}

Q_GLOBAL_STATIC(QSet<QUrl>, entryUrlList)

void computer_sidebar_event_calls::callItemAdd(const QUrl &vEntryUrl)
{
    entryUrlList->insert(vEntryUrl);
    const char *kTransContext = "dfmplugin_computer::ComputerItemWatcher";
    dpfSlotChannel->push(kComputerEventNS, kCptSlotAdd,
                         qApp->translate(kTransContext, "Disks"), vEntryUrl, 1, false);

    // build params
    DFMEntryFileInfoPointer info(new EntryFileInfo(vEntryUrl));
    QVariantMap opts {
        { "Property_Key_Group", "Group_Network" },
        { "Property_Key_SubGroup", "" },
        { "Property_Key_DisplayName", info->displayName() },
        { "Property_Key_EditDisplayText", info->editDisplayText() },
        { "Property_Key_Icon", QIcon::fromTheme(info->fileIcon().name() + "-symbolic") },
        { "Property_Key_FinalUrl", info->targetUrl().isValid() ? info->targetUrl() : QUrl() },
        { "Property_Key_QtItemFlags", QVariant::fromValue(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable) },
        { "Property_Key_Ejectable", false },
        { "Property_Key_CallbackContextMenu", QVariant::fromValue(ContextMenuCallback(sidebarMenuCall)) },
        { "Property_Key_CallbackItemClicked", QVariant::fromValue(ItemClickedActionCallback(sidebarItemClicked)) },
        { "Property_Key_CallbackFindMe", QVariant::fromValue(FindMeCallback(sidebarUrlEquals)) },
        { "Property_Key_CallbackRename", QVariant::fromValue(RenameCallback(sidebarItemRename)) },
        { "Property_Key_VisiableControl", "mounted_share_dirs" },
        { "Property_Key_VisiableDisplayName", QObject::tr("Mounted sharing folders") },
        { "Property_Key_Editable", info->renamable() }
        //        { "Property_Key_ReportName", reportName }
    };
    auto stdSmb = vEntryUrl.path().remove("." + QString(kVEntrySuffix));
    QUrl sidebarUrl(stdSmb);
    sidebarUrl.setScheme("vsmb");
    dpfSlotChannel->push(kSidebarEventNS, kSbSlotAdd, sidebarUrl, opts);
}

void computer_sidebar_event_calls::callItemRemove(const QUrl &vEntryUrl)
{
    entryUrlList->remove(vEntryUrl);
    dpfSlotChannel->push(kComputerEventNS, kCptSlotRemove,
                         vEntryUrl);

    // build params
    auto stdSmb = vEntryUrl.path().remove("." + QString(kVEntrySuffix));
    QUrl sidebarUrl(stdSmb);
    sidebarUrl.setScheme("vsmb");
    dpfSlotChannel->push(kSidebarEventNS, kSbSlotRemove, sidebarUrl);
}

void computer_sidebar_event_calls::callComputerRefresh()
{
    // remove all virtual entry from sidebar.
    QStringList allStdSmbs = VirtualEntryDbHandler::instance()->allSmbIDs();

    const QStringList &allMounted = protocol_display_utilities::getStandardSmbPaths(protocol_display_utilities::getMountedSmb());
    for (const auto &mounted : allMounted) {
        if (!allStdSmbs.contains(mounted))
            allStdSmbs.append(mounted);
        auto host = protocol_display_utilities::getSmbHostPath(mounted);
        if (!allStdSmbs.contains(host))
            allStdSmbs.append(host);
    }

    std::for_each(allStdSmbs.cbegin(), allStdSmbs.cend(), [=](const QString &smb) {
        QUrl url(smb);
        url.setScheme("vsmb");
        dpfSlotChannel->push(kSidebarEventNS, kSbSlotRemove, url);
        entryUrlList->remove(protocol_display_utilities::makeVEntryUrl(smb));
    });

    dpfSlotChannel->push(kComputerEventNS, kCptSlotRefresh);
}

void computer_sidebar_event_calls::sidebarMenuCall(quint64 winId, const QUrl &url, const QPoint &pos)
{
    using namespace dfmplugin_menu_util;
    using namespace protocol_display_utilities;

    QScopedPointer<AbstractMenuScene> scene(menuSceneCreateScene(kComputerMenu));
    if (!scene) {
        fmWarning() << "Create scene for computer failed: " << kComputerMenu;
        return;
    }

    if (url.scheme() != "vsmb")
        return;
    QUrl smbUrl = url;
    smbUrl.setScheme("smb");
    QVariant selectedUrls = QVariant::fromValue<QList<QUrl>>({ makeVEntryUrl(smbUrl.toString()) });
    QVariantHash params {
        { MenuParamKey::kIsEmptyArea, false },
        { MenuParamKey::kWindowId, winId },
        { MenuParamKey::kSelectFiles, selectedUrls },
    };

    if (!scene->initialize(params))
        return;

    DTK_WIDGET_NAMESPACE::DMenu m;
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(&m), AcName::kAcSidebarDeviceMenu);
#endif
    m.setProperty("trigger-from-sidebar", true);

    scene->create(&m);
    scene->updateState(&m);

    auto act = m.exec(pos);
    if (act) {
        auto key = act->property(ActionPropertyKey::kActionID).toString();
        if (key == "computer-rename")
            dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_TriggerEdit", winId, smbUrl);
        else
            scene->triggered(act);
    }
}

QUrl protocol_display_utilities::makeVEntryUrl(const QString &standardSmb)
{
    QUrl u;
    u.setScheme(Global::Scheme::kEntry);
    u.setPath(standardSmb + "." + QString(kVEntrySuffix));
    return u;
}

QStringList protocol_display_utilities::getMountedSmb()
{
    auto protoDevs = DevProxyMng->getAllProtocolIds();
    for (int i = protoDevs.count() - 1; i >= 0; i--) {
        QUrl dev(protoDevs.at(i));
        if (!ProtocolUtils::isSMBFile(dev)) {
            protoDevs.removeAt(i);
            continue;
        }
    }
    std::sort(protoDevs.begin(), protoDevs.end());
    if (protoDevs.count() > 0)
        fmDebug() << "mounted smbs: " << protoDevs;
    return protoDevs;
}

QStringList protocol_display_utilities::getStandardSmbPaths(const QStringList &devIds)
{
    QStringList stds;
    for (auto &id : devIds)
        stds << getStandardSmbPath(id);

    return stds;
}

QString protocol_display_utilities::getSmbHostPath(const QString &devId)
{
    QUrl url(getStandardSmbPath(devId));
    url.setPath("");
    return url.toString();
}

QString protocol_display_utilities::getStandardSmbPath(const QUrl &entryUrl)
{
    QString path = entryUrl.path();
    if (!path.endsWith(kComputerProtocolSuffix))
        return "";
    path.remove("." + QString(kComputerProtocolSuffix));
    return getStandardSmbPath(path);
}

QString protocol_display_utilities::getStandardSmbPath(const QString &devId)
{
    QString id = QUrl::fromPercentEncoding(devId.toLocal8Bit());
    static const QRegularExpression kCifsSmbPrefix(R"(^file:///(?:run/)?media/.*/smbmounts/)");

    if (!id.startsWith(Global::Scheme::kFile) || !id.contains(kCifsSmbPrefix))
        return id;

    QString dirName = id;
    dirName.remove(kCifsSmbPrefix);

    QString host, share, port;
    if (!DeviceUtils::parseSmbInfo(dirName, host, share, &port))
        return id;

    QString stdSmb;
    if (port.isEmpty())
        stdSmb = QString("smb://%1/%2/").arg(host).arg(share);
    else
        stdSmb = QString("smb://%1:%2/%3/").arg(host).arg(port).arg(share);
    return stdSmb;
}

QString protocol_display_utilities::getDisplayNameOf(const QString &devId)
{
    QUrl u;
    u.setScheme(Global::Scheme::kEntry);
    u.setPath(devId + "." + QString(kComputerProtocolSuffix));
    return getDisplayNameOf(u);
}

QString protocol_display_utilities::getDisplayNameOf(const QUrl &entryUrl)
{
    DFMEntryFileInfoPointer entry { new EntryFileInfo(entryUrl) };
    return entry ? entry->displayName() : "";
}

void ui_ventry_calls::addAggregatedItemForSeperatedOnlineItem(const QUrl &entryUrl)
{
    using namespace protocol_display_utilities;
    using namespace computer_sidebar_event_calls;

    const QString &stdSmbPath = getStandardSmbPath(entryUrl);
    const QString &smbHostPath = getSmbHostPath(stdSmbPath);
    if (smbHostPath.isEmpty())
        return;

    const QUrl &vEntryUrl = makeVEntryUrl(smbHostPath);
    callItemAdd(vEntryUrl);
}

void ui_ventry_calls::addAggregatedItems()
{
    using namespace protocol_display_utilities;
    using namespace computer_sidebar_event_calls;

    // 1. get all onlined smb
    QStringList smbs = getMountedSmb();
    // 1.1 convert to std smb
    smbs = getStandardSmbPaths(smbs);
    // 2. get all offlined smb
    smbs.append(VirtualEntryDbHandler::instance()->allSmbIDs());

    // 3. deduplicated, only keep smb root.
    QSet<QString> hostPaths;
    for (const auto &id : smbs)
        hostPaths.insert(getSmbHostPath(id));

    // 3. add aggregated item
    std::for_each(hostPaths.cbegin(), hostPaths.cend(), [=](const QString &host) {
        const QUrl &vEntryUrl = makeVEntryUrl(host);
        callItemAdd(vEntryUrl);
    });
}

void ui_ventry_calls::addSeperatedOfflineItems()
{
    using namespace protocol_display_utilities;
    using namespace computer_sidebar_event_calls;

    // 1. get all onlined smb
    QStringList smbs = getMountedSmb();
    // 1.1 convert to std smb
    smbs = getStandardSmbPaths(smbs);
    // 2. get all cached smb
    QStringList cachedSmbs = VirtualEntryDbHandler::instance()->allSmbIDs();
    // 3. get offlined smb
    std::for_each(smbs.cbegin(), smbs.cend(), [&](const QString &stdSmb) {
        cachedSmbs.removeAll(stdSmb);
    });
    // 4. seperated share only (keep the host out)
    QStringList offlinedSmbs;
    std::for_each(cachedSmbs.cbegin(), cachedSmbs.cend(), [&](const QString &stdSmb) {
        if (!QUrl(stdSmb).path().isEmpty())
            offlinedSmbs.append(stdSmb);
    });

    // 4. add seperated offline item
    std::for_each(offlinedSmbs.cbegin(), offlinedSmbs.cend(), [=](const QString &stdSmb) {
        const QUrl &vEntryUrl = makeVEntryUrl(stdSmb);
        callItemAdd(vEntryUrl);
    });
}

void computer_sidebar_event_calls::callForgetPasswd(const QString &stdSmb)
{
    dpfSlotChannel->push(kComputerEventNS, kCptSlotForget, stdSmb);
}

bool protocol_display_utilities::hasMountedShareOf(const QString &stdHost)
{
    const QStringList &allMounted = getStandardSmbPaths(getMountedSmb());
    return std::any_of(allMounted.cbegin(), allMounted.cend(), [&](const QString &stdSmb) {
        return stdSmb.startsWith(stdHost);
    });
}

void computer_sidebar_event_calls::sidebarItemClicked(quint64 winId, const QUrl &url)
{
    QUrl smbUrl(url);
    smbUrl.setScheme("smb");
    auto sUrl = smbUrl.toString();
    if (!sUrl.endsWith("/"))
        sUrl += "/";
    auto fullPath = VirtualEntryDbHandler::instance()->getFullSmbPath(sUrl);
    dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, QUrl(fullPath));
}

bool computer_sidebar_event_calls::sidebarUrlEquals(const QUrl &item, const QUrl &target)
{
    if (item.scheme() == "vsmb" && target.scheme() == "smb") {
        auto pathA = item.path();
        auto pathB = target.path();
        if (!pathA.endsWith('/'))
            pathA += "/";
        if (!pathB.endsWith('/'))
            pathB += "/";
        return pathA == pathB && item.host() == target.host();
    }
    return false;
}

void secret_utils::forgetPasswordInSession(const QString &host)
{
    const SecretSchema schema = {
        "org.gnome.keyring.NetworkPassword",
        SECRET_SCHEMA_NONE,
        { { "user", SECRET_SCHEMA_ATTRIBUTE_STRING },
          { "domain", SECRET_SCHEMA_ATTRIBUTE_STRING },
          { "server", SECRET_SCHEMA_ATTRIBUTE_STRING },
          { "protocol", SECRET_SCHEMA_ATTRIBUTE_STRING } }
    };

    GError_autoptr error = NULL;
    SecretService *service = secret_service_get_sync(
            SECRET_SERVICE_NONE,
            NULL,
            &error);
    if (error) {
        fmWarning() << "Error connecting to service:" << error->message;
        return;
    }

    SecretCollection *sessionCollection = secret_collection_for_alias_sync(
            service,
            "session",   // 使用 "session" 别名获取会话集合
            SECRET_COLLECTION_LOAD_ITEMS,
            NULL,
            &error);
    if (error) {
        fmWarning() << "Error getting session collection:" << error->message;
        g_error_free(error);
        g_object_unref(service);
        return;
    }
    if (!sessionCollection) {
        fmWarning() << "Session collection not found";
        g_object_unref(service);
        return;
    }

    // 在会话集合中搜索密码
    GHashTable *query = g_hash_table_new_full(g_str_hash,
                                              g_str_equal,
                                              g_free,
                                              g_free);
    g_hash_table_insert(query, g_strdup("server"), g_strdup(host.toStdString().c_str()));
    g_hash_table_insert(query, g_strdup("protocol"), g_strdup("smb"));

    GList_autoptr items = secret_collection_search_sync(
            sessionCollection,
            &schema,
            query,
            SECRET_SEARCH_ALL,
            NULL,
            &error);
    if (error) {
        fmWarning() << "Error searching in session collection:" << error->message;
        g_object_unref(sessionCollection);
        g_object_unref(service);
        if (query)
            g_hash_table_unref(query);
        return;
    }

    GList *gList = items;
    while (gList) {
        SecretItem *item = reinterpret_cast<SecretItem *>(gList->data);
        gList = gList->next;
        char *label = secret_item_get_label(item);
        fmInfo() << "Remove saved item:" << QString(label);
        secret_item_delete(item, nullptr, nullptr, nullptr);
        g_free(label);
    }
    g_object_unref(sessionCollection);
    g_object_unref(service);
    if (query)
        g_hash_table_unref(query);
}

void computer_sidebar_event_calls::sidebarItemRename(quint64 windowId, const QUrl &url, const QString &name)
{
    QUrl smbUrl(url);
    smbUrl.setScheme("smb");
    NPDeviceAliasManager::instance()->setAlias(smbUrl, name);
    // Update related sidebar items
    for (const auto &entryUrl : std::as_const(*entryUrlList)) {
        auto stdSmb = entryUrl.path().remove("." + QString(kVEntrySuffix));
        QUrl smbUrl(stdSmb);
        if (smbUrl.host() != url.host())
            continue;

        DFMEntryFileInfoPointer info(new EntryFileInfo(entryUrl));
        QVariantMap map {
            { "Property_Key_DisplayName", info->displayName() },
            { "Property_Key_EditDisplayText", info->editDisplayText() },
            { "Property_Key_Editable", true }
        };

        smbUrl.setScheme("vsmb");
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", smbUrl, map);
        dpfSlotChannel->push("dfmplugin_titlebar", "slot_Crumb_Update", info->urlOf(UrlInfoType::kUrl));
    }
    dpfSlotChannel->push(kComputerEventNS, kCptSlotRefresh);
}
