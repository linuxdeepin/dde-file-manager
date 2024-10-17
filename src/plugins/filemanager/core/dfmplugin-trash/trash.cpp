// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trash.h"
#include "trashdiriterator.h"
#include "trashfilewatcher.h"
#include "utils/trashhelper.h"
#include "utils/trashfilehelper.h"
#include "menus/trashmenuscene.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-base/utils/systempathutil.h>

using BasicViewFieldFunc = std::function<QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(const QUrl &url)>;
using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;
using CreateTopWidgetCallback = std::function<QWidget *()>;
using ShowTopWidgetCallback = std::function<bool(QWidget *, const QUrl &)>;

Q_DECLARE_METATYPE(CreateTopWidgetCallback);
Q_DECLARE_METATYPE(ShowTopWidgetCallback);
Q_DECLARE_METATYPE(ContextMenuCallback)
Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(BasicViewFieldFunc)
Q_DECLARE_METATYPE(QString *);
Q_DECLARE_METATYPE(QVariant *)
Q_DECLARE_METATYPE(bool *)

DFMBASE_USE_NAMESPACE
namespace dfmplugin_trash {
DFM_LOG_REISGER_CATEGORY(DPTRASH_NAMESPACE)

void Trash::initialize()
{
    WatcherFactory::regClass<TrashFileWatcher>(TrashHelper::scheme());
    DirIteratorFactory::regClass<TrashDirIterator>(TrashHelper::scheme());

    followEvents();
    bindWindows();
}

bool Trash::start()
{
    // show first window when all plugin initialized
    dfmplugin_menu_util::menuSceneRegisterScene(TrashMenuCreator::name(), new TrashMenuCreator());

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", TrashHelper::scheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", TrashHelper::scheme(), TrashMenuCreator::name());

    addCustomTopWidget();
    addFileOperations();

    return true;
}

Trash::~Trash()
{
}

void Trash::onWindowOpened(quint64 windId)
{
    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "Trash", "Cannot find window by id");
    if (window->titleBar())
        regTrashCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Trash::regTrashCrumbToTitleBar, Qt::DirectConnection);

    if (window->sideBar())
        regTrashItemToSideBar();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, &Trash::regTrashItemToSideBar, Qt::DirectConnection);
}

void Trash::regTrashCrumbToTitleBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", TrashHelper::scheme(), QVariantMap {});
    });
}

void Trash::regTrashItemToSideBar()
{
    auto bookmarkPlugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj("dfmplugin-bookmark") };
    if (bookmarkPlugin && bookmarkPlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted) {
        updateTrashItemToSideBar();
    } else {
        connect(
                DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [this](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin-bookmark")
                        updateTrashItemToSideBar();
                },
                Qt::DirectConnection);
    }
}

void Trash::updateTrashItemToSideBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        ContextMenuCallback contextMenuCb { TrashHelper::contenxtMenuHandle };
        Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled };
        QVariantMap map {
            { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
            { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) },
        };

        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", TrashHelper::rootUrl(), map);
    });
}

void Trash::addFileOperations()
{
    BasicViewFieldFunc func { TrashHelper::propetyExtensionFunc };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_BasicViewExtension_Register",
                         func, TrashHelper::scheme());

    QStringList &&filtes { "kPermission" };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_BasicFiledFilter_Add",
                         TrashHelper::scheme(), filtes);

    QStringList &&detailFiltes { "kFileSizeField" };
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_BasicFiledFilter_Root_Add",
                         TrashHelper::scheme(), detailFiltes);

    BasicViewFieldFunc detailFunc { TrashHelper::detailExtensionFunc };
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_BasicViewExtension_Register",
                         detailFunc, TrashHelper::scheme());
}

void Trash::addCustomTopWidget()
{
    CreateTopWidgetCallback createCallback { TrashHelper::createEmptyTrashTopWidget };
    ShowTopWidgetCallback showCallback { TrashHelper::showTopWidget };

    QVariantMap map {
        { "Property_Key_Scheme", TrashHelper::scheme() },
        { "Property_Key_KeepShow", false },
        { "Property_Key_CreateTopWidgetCallback", QVariant::fromValue(createCallback) },
        { "Property_Key_ShowTopWidgetCallback", QVariant::fromValue(showCallback) }
    };

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterCustomTopWidget", map);
}

void Trash::followEvents()
{
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_Model_EmptyDir", TrashHelper::instance(), &TrashHelper::onTrashEmptyState);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_CheckDragDropAction", TrashHelper::instance(), &TrashHelper::checkDragDropAction);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_FileCanMove", TrashHelper::instance(), &TrashHelper::checkCanMove);
    dpfHookSequence->follow("dfmplugin_detailspace", "hook_Icon_Fetch", TrashHelper::instance(), &TrashHelper::detailViewIcon);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Model_FetchCustomColumnRoles", TrashHelper::instance(), &TrashHelper::customColumnRole);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Model_FetchCustomRoleDisplayName", TrashHelper::instance(), &TrashHelper::customRoleDisplayName);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PasteFiles", TrashFileHelper::instance(), &TrashFileHelper::blockPaste);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Tab_FileDeleteNotCdComputer", TrashFileHelper::instance(), &TrashFileHelper::handleNotCdComputer);

    // hook events, file operation
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CutToFile", TrashFileHelper::instance(), &TrashFileHelper::cutFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CopyFile", TrashFileHelper::instance(), &TrashFileHelper::copyFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_MoveToTrash", TrashFileHelper::instance(), &TrashFileHelper::moveToTrash);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_DeleteFile", TrashFileHelper::instance(), &TrashFileHelper::deleteFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin", TrashFileHelper::instance(), &TrashFileHelper::openFileInPlugin);

    dpfHookSequence->follow("dfmplugin_utils", "hook_OpenWith_DisabledOpenWithWidget", TrashFileHelper::instance(), &TrashFileHelper::disableOpenWidgetWidget);

    // register to tag
    auto searchPlugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj("dfmplugin-tag") };
    if (searchPlugin && searchPlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted) {
        dpfHookSequence->follow("dfmplugin_tag", "hook_CanTaged", TrashFileHelper::instance(), &TrashFileHelper::handleCanTag);
    } else {
        connect(
                DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin-tag")
                        dpfHookSequence->follow("dfmplugin_tag", "hook_CanTaged", TrashFileHelper::instance(), &TrashFileHelper::handleCanTag);
                },
                Qt::DirectConnection);
        connect(
                DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin-search")
                        dpfHookSequence->follow("dfmplugin_search", "hook_Url_IsSubFile",
                                                TrashFileHelper::instance(),
                                                &TrashFileHelper::handleIsSubFile);
                },
        Qt::DirectConnection);
    }
}

void Trash::bindWindows()
{
    const auto &winIdList { FMWindowsIns.windowIdList() };
    std::for_each(winIdList.begin(), winIdList.end(), [this](quint64 id) {
        onWindowOpened(id);
    });
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &Trash::onWindowOpened, Qt::DirectConnection);
}
}   // namespace dfmplugin_trash
