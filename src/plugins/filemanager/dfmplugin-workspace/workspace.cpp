// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspace.h"
#include "views/workspacewidget.h"
#include "views/fileview.h"
#include "views/renamebar.h"
#include "utils/workspacehelper.h"
#include "utils/customtopwidgetinterface.h"
#include "utils/filedatamanager.h"
#include "events/workspaceeventreceiver.h"
#include "menus/workspacemenuscene.h"
#include "menus/sortanddisplaymenuscene.h"
#include "menus/basesortmenuscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/configs/configsynchronizer.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/configs/dconfig/global_dconf_defines.h>

#include <dfm-framework/dpf.h>

DFMBASE_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;

namespace dfmplugin_workspace {
DFM_LOG_REGISTER_CATEGORY(DPWORKSPACE_NAMESPACE)

/*!
 * \brief 视图相关配置项
 */
namespace ViewConfig {
inline constexpr char kIconSizeLevel[] { "dfm.icon.size.level" };
inline constexpr char kIconGridDensityLevel[] { "dfm.icon.griddensity.level" };
inline constexpr char kListHeightLevel[] { "dfm.list.height.level" };
}   // namespace ViewConfig

void Workspace::initialize()
{
    fmDebug() << "Workspace initialize called";
    WorkspaceHelper::instance()->registerFileView(Global::Scheme::kFile);

    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &Workspace::onWindowOpened, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &Workspace::onWindowClosed, Qt::DirectConnection);

    connect(this, &Workspace::readyToInstallWidget,
            WorkspaceHelper::instance(), &WorkspaceHelper::installWorkspaceWidgetToWindow);

    WorkspaceEventReceiver::instance()->initConnection();

    initConfig();
    fmDebug() << "Workspace initialization completed";
}

bool Workspace::start()
{
    fmDebug() << "Workspace start called";
    DFMBASE_USE_NAMESPACE

    dfmplugin_menu_util::menuSceneRegisterScene(WorkspaceMenuCreator::name(), new WorkspaceMenuCreator());
    dfmplugin_menu_util::menuSceneRegisterScene(SortAndDisplayMenuCreator::name(), new SortAndDisplayMenuCreator());
    dfmplugin_menu_util::menuSceneRegisterScene(BaseSortMenuCreator::name(), new BaseSortMenuCreator());
    dfmplugin_menu_util::menuSceneBind(SortAndDisplayMenuCreator::name(), WorkspaceMenuCreator::name());

    const QString &scheme = Global::Scheme::kFile;

    if (WorkspaceHelper::instance()->isRegistedTopWidget(scheme)) {
        fmWarning() << "Workspace: custom top widget scheme" << scheme << "has been registered!";
        return false;
    }

    WorkspaceHelper::instance()->registerTopWidgetCreator(scheme, []() {
        CustomTopWidgetInterface *interface { new CustomTopWidgetInterface };
        interface->registeCreateTopWidgetCallback([]() {
            return new RenameBar();
        });
        interface->setKeepShow(false);
        interface->setKeepTop(false);
        return interface;
    });
    fmDebug() << "Workspace: top widget creator registered for scheme:" << scheme;

    QString err;
    auto ret = DConfigManager::instance()->addConfig(DConfigInfo::kConfName, &err);
    if (!ret) {
        fmWarning() << "Workspace: create dconfig failed:" << err;
    } else {
        fmDebug() << "Workspace: dconfig created successfully";
    }

    return true;
}

void Workspace::onWindowOpened(quint64 windId)
{
    fmDebug() << "Workspace onWindowOpened called for window ID:" << windId;
    DFMBASE_USE_NAMESPACE

    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "WorkSpace", "Cannot find window by id");
    WorkspaceWidget *workspace = new WorkspaceWidget;
    WorkspaceHelper::instance()->addWorkspace(windId, workspace);

    Q_EMIT readyToInstallWidget(windId);
    fmDebug() << "Workspace: readyToInstallWidget signal emitted for window:" << windId;
}

void Workspace::onWindowClosed(quint64 windId)
{
    fmDebug() << "Workspace onWindowClosed called for window ID:" << windId;
    WorkspaceHelper::instance()->removeWorkspace(windId);
}

void Workspace::initConfig()
{
    fmDebug() << "Workspace initConfig called";

    SyncPair thumbnailPair {
        { SettingType::kGenAttr, Application::kShowThunmbnailInRemote },
        { DConfigInfo::kConfName, DConfigInfo::kRemoteThumbnailKey },
        saveRemoteThumbnailToConf,
        syncRemoteThumbnailToAppSet,
        isRemoteThumbnailConfEqual
    };
    ConfigSynchronizer::instance()->watchChange(thumbnailPair);

    SyncPair iconSizePair {
        { SettingType::kAppAttr, Application::kIconSizeLevel },
        { kViewDConfName, ViewConfig::kIconSizeLevel },
        saveIconSizeToConf,
        syncIconSizeToAppSet,
        isIconSizeConfEqual
    };
    ConfigSynchronizer::instance()->watchChange(iconSizePair);

    SyncPair gridDensityPair {
        { SettingType::kAppAttr, Application::kGridDensityLevel },
        { kViewDConfName, ViewConfig::kIconGridDensityLevel },
        saveGridDensityToConf,
        syncGridDensityToAppSet,
        isGridDensityConfEqual
    };
    ConfigSynchronizer::instance()->watchChange(gridDensityPair);

    SyncPair listHeightPair {
        { SettingType::kAppAttr, Application::kListHeightLevel },
        { kViewDConfName, ViewConfig::kListHeightLevel },
        saveListHeightToConf,
        syncListHeightToAppSet,
        isListHeightConfEqual
    };
    ConfigSynchronizer::instance()->watchChange(listHeightPair);
    fmDebug() << "Workspace: all configuration sync pairs initialized";
}

void Workspace::saveRemoteThumbnailToConf(const QVariant &var)
{
    fmDebug() << "Workspace: saving remote thumbnail config value:" << var.toBool();
    DConfigManager::instance()->setValue(DConfigInfo::kConfName, DConfigInfo::kRemoteThumbnailKey, var);
}

void Workspace::syncRemoteThumbnailToAppSet(const QString &, const QString &, const QVariant &var)
{
    Application::instance()->setGenericAttribute(Application::kShowThunmbnailInRemote, var.toBool());
}

bool Workspace::isRemoteThumbnailConfEqual(const QVariant &dcon, const QVariant &dset)
{
    return dcon.toBool() && dset.toBool();
}

void Workspace::saveIconSizeToConf(const QVariant &var)
{
    fmDebug() << "Workspace: saving icon size config value:" << var.toInt();
    DConfigManager::instance()->setValue(kViewDConfName, ViewConfig::kIconSizeLevel, var);
}

void Workspace::syncIconSizeToAppSet(const QString &, const QString &, const QVariant &var)
{
    Application::instance()->setAppAttribute(Application::kIconSizeLevel, var.toInt());
}

bool Workspace::isIconSizeConfEqual(const QVariant &dcon, const QVariant &dset)
{
    return dcon.toInt() == dset.toInt();
}

void Workspace::saveGridDensityToConf(const QVariant &var)
{
    fmDebug() << "Workspace: saving grid density config value:" << var.toInt();
    DConfigManager::instance()->setValue(kViewDConfName, ViewConfig::kIconGridDensityLevel, var);
}

void Workspace::syncGridDensityToAppSet(const QString &, const QString &, const QVariant &var)
{
    Application::instance()->setAppAttribute(Application::kGridDensityLevel, var.toInt());
}

bool Workspace::isGridDensityConfEqual(const QVariant &dcon, const QVariant &dset)
{
    return dcon.toInt() == dset.toInt();
}

void Workspace::saveListHeightToConf(const QVariant &var)
{
    fmDebug() << "Workspace: saving list height config value:" << var.toInt();
    DConfigManager::instance()->setValue(kViewDConfName, ViewConfig::kListHeightLevel, var);
}

void Workspace::syncListHeightToAppSet(const QString &, const QString &, const QVariant &var)
{
    Application::instance()->setAppAttribute(Application::kListHeightLevel, var.toInt());
}

bool Workspace::isListHeightConfEqual(const QVariant &dcon, const QVariant &dset)
{
    return dcon.toInt() == dset.toInt();
}

}   // namespace dfmplugin_workspace
