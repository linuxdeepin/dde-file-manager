/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "optical.h"
#include "utils/opticalhelper.h"
#include "utils/opticalfilehelper.h"
#include "utils/opticalsignalmanager.h"
#include "mastered/masteredmediafileinfo.h"
#include "mastered/masteredmediafilewatcher.h"
#include "mastered/masteredmediadiriterator.h"
#include "views/opticalmediawidget.h"
#include "menus/opticalmenuscene.h"
#include "events/opticaleventreceiver.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

using CreateTopWidgetCallback = std::function<QWidget *()>;
using ShowTopWidgetCallback = std::function<bool(QWidget *, const QUrl &)>;
Q_DECLARE_METATYPE(CreateTopWidgetCallback);
Q_DECLARE_METATYPE(ShowTopWidgetCallback);
Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(QList<QVariantMap> *)

// using CreateTopWidgetCallback = std::function<dfmplugin_optical::OpticalMediaWidget *()>;

using namespace dfmplugin_optical;

DFMBASE_USE_NAMESPACE

void Optical::initialize()
{
    UrlRoute::regScheme(Global::Scheme::kBurn, "/", OpticalHelper::icon(), true);
    InfoFactory::regClass<MasteredMediaFileInfo>(Global::Scheme::kBurn, InfoFactory::kNoCache);
    WatcherFactory::regClass<MasteredMediaFileWatcher>(Global::Scheme::kBurn, WatcherFactory::kNoCache);
    DirIteratorFactory::regClass<MasteredMediaDirIterator>(Global::Scheme::kBurn);

    bindEvents();

    connect(
            &FMWindowsIns, &FileManagerWindowsManager::windowCreated, this,
            [this]() {
                addOpticalCrumbToTitleBar();
            },
            Qt::DirectConnection);

    // for blank disc
    connect(
            DevProxyMng, &DeviceProxyManager::blockDevPropertyChanged, this,
            [this](const QString &id, const QString &property, const QVariant &val) {
                if (id.contains(QRegularExpression("/sr[0-9]*$"))
                    && property == GlobalServerDefines::DeviceProperty::kOptical && !val.toBool()) {
                    onDeviceChanged(id);
                }
            },
            Qt::DirectConnection);
}

bool Optical::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(OpticalMenuSceneCreator::name(), new OpticalMenuSceneCreator);

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", QString(Global::Scheme::kBurn));
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", QString(Global::Scheme::kBurn), OpticalMenuSceneCreator::name());

    addCustomTopWidget();
    addDelegateSettings();
    addPropertySettings();

    // follow event
    dpfHookSequence->follow("dfmplugin_utils", "hook_UrlsTransform", OpticalHelper::instance(), &OpticalHelper::urlsToLocal);

    addFileOperations();

    return true;
}

void Optical::addOpticalCrumbToTitleBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", QString(Global::Scheme::kBurn), QVariantMap {});
    });
}

void Optical::addFileOperations()
{
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CutFile", OpticalFileHelper::instance(), &OpticalFileHelper::cutFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CopyFile", OpticalFileHelper::instance(), &OpticalFileHelper::copyFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_MoveToTrash", OpticalFileHelper::instance(), &OpticalFileHelper::moveToTrash);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_DeleteFile", OpticalFileHelper::instance(), &OpticalFileHelper::moveToTrash);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin", OpticalFileHelper::instance(), &OpticalFileHelper::openFileInPlugin);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_LinkFile", OpticalFileHelper::instance(), &OpticalFileHelper::linkFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_WriteUrlsToClipboard", OpticalFileHelper::instance(), &OpticalFileHelper::writeUrlsToClipboard);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenInTerminal", OpticalFileHelper::instance(), &OpticalFileHelper::openFileInTerminal);
}

void Optical::addCustomTopWidget()
{
    CreateTopWidgetCallback createCallback { []() {
        return new OpticalMediaWidget;
    } };

    ShowTopWidgetCallback showCallback { [](QWidget *w, const QUrl &url) {
        bool ret { true };
        OpticalMediaWidget *mediaWidget = qobject_cast<OpticalMediaWidget *>(w);
        if (mediaWidget)
            ret = mediaWidget->updateDiscInfo(url);

        return ret;
    } };

    QVariantMap map {
        { "Property_Key_Scheme", Global::Scheme::kBurn },
        { "Property_Key_KeepShow", false },
        { "Property_Key_CreateTopWidgetCallback", QVariant::fromValue(createCallback) },
        { "Property_Key_ShowTopWidgetCallback", QVariant::fromValue(showCallback) }
    };

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterCustomTopWidget", map);
}

void Optical::addDelegateSettings()
{
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Delegate_CheckTransparent", OpticalHelper::instance(), &OpticalHelper::isTransparent);
}

void Optical::addPropertySettings()
{
    QStringList &&filtes { "kPermission" };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_BasicFiledFilter_Add",
                         QString(Global::Scheme::kBurn), filtes);
}

void Optical::bindEvents()
{
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_DeleteFiles", &OpticalEventReceiver::instance(),
                            &OpticalEventReceiver::handleDeleteFilesShortcut);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_CheckDragDropAction", &OpticalEventReceiver::instance(),
                            &OpticalEventReceiver::handleCheckDragDropAction);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_FileDragMove", &OpticalEventReceiver::instance(),
                            &OpticalEventReceiver::handleCheckDragDropAction);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_FileDrop", &OpticalEventReceiver::instance(),
                            &OpticalEventReceiver::handleDropFiles);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Crumb_Seprate", &OpticalEventReceiver::instance(), &OpticalEventReceiver::sepateTitlebarCrumb);
}

void Optical::onDeviceChanged(const QString &id)
{
    const auto &discUrl { OpticalHelper::transDiscRootById(id) };
    if (discUrl.isValid()) {
        emit OpticalSignalManager::instance()->discUnmounted(discUrl);
        dpfSlotChannel->push("dfmplugin_workspace", "slot_Tab_Close", discUrl);
    }
}
