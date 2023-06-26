// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspace.h"
#include "views/workspacewidget.h"
#include "views/fileview.h"
#include "views/renamebar.h"
#include "utils/workspacehelper.h"
#include "utils/customtopwidgetinterface.h"
#include "events/workspaceeventreceiver.h"
#include "menus/workspacemenuscene.h"
#include "menus/sortanddisplaymenuscene.h"
#include "menus/basesortmenuscene.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/dpf.h>

using namespace dfmplugin_workspace;

void Workspace::initialize()
{
    DFMBASE_USE_NAMESPACE

    WorkspaceHelper::instance()->registerFileView(Global::Scheme::kFile);

    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &Workspace::onWindowOpened, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &Workspace::onWindowClosed, Qt::DirectConnection);

    connect(this, &Workspace::readyToInstallWidget,
            WorkspaceHelper::instance(), &WorkspaceHelper::installWorkspaceWidgetToWindow);

    WorkspaceEventReceiver::instance()->initConnection();
}

bool Workspace::start()
{
    DFMBASE_USE_NAMESPACE

    dfmplugin_menu_util::menuSceneRegisterScene(WorkspaceMenuCreator::name(), new WorkspaceMenuCreator());
    dfmplugin_menu_util::menuSceneRegisterScene(SortAndDisplayMenuCreator::name(), new SortAndDisplayMenuCreator());
    dfmplugin_menu_util::menuSceneRegisterScene(BaseSortMenuCreator::name(), new BaseSortMenuCreator());
    dfmplugin_menu_util::menuSceneBind(SortAndDisplayMenuCreator::name(), WorkspaceMenuCreator::name());

    const QString &scheme = Global::Scheme::kFile;

    if (WorkspaceHelper::instance()->isRegistedTopWidget(scheme)) {
        qWarning() << "custom top widget sechme " << scheme << "has been resigtered!";
        return false;
    }

    WorkspaceHelper::instance()->registerTopWidgetCreator(scheme, []() {
        CustomTopWidgetInterface *interface { new CustomTopWidgetInterface };
        interface->registeCreateTopWidgetCallback([]() {
            return new RenameBar();
        });
        interface->setKeepShow(false);
        return interface;
    });

    return true;
}

void Workspace::onWindowOpened(quint64 windId)
{
    DFMBASE_USE_NAMESPACE

    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "WorkSpace", "Cannot find window by id");
    WorkspaceWidget *workspace = new WorkspaceWidget;
    WorkspaceHelper::instance()->addWorkspace(windId, workspace);

    Q_EMIT readyToInstallWidget(windId);
}

void Workspace::onWindowClosed(quint64 windId)
{
    WorkspaceHelper::instance()->removeWorkspace(windId);
}
