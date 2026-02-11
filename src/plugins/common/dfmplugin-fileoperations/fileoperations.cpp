// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperations.h"
#include "fileoperationsevent/fileoperationseventreceiver.h"
#include "fileoperationsevent/trashfileeventreceiver.h"
#include "settings/operationsettings.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/settingdialog/customsettingitemregister.h>

#include <DSettingsOption>
#include <QLabel>

Q_DECLARE_METATYPE(bool *)

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

inline constexpr char kFileOperations[] { "org.deepin.dde.file-manager.operations" };
inline constexpr char kBlockEverySync[] { "file.operation.blockeverysync" };
inline constexpr char kSettingGroup[] { "10_advance.02_0external_storage_device" };

namespace dfmplugin_fileoperations {
DFM_LOG_REGISTER_CATEGORY(DPFILEOPERATIONS_NAMESPACE)
}

void FileOperations::initialize()
{
    initEventHandle();
    followEvents();
}

bool FileOperations::start()
{
    QString err;
    auto ret = DConfigManager::instance()->addConfig(kFileOperations, &err);
    if (!ret)
        fmWarning() << "create dconfig failed: " << err;

    regSettingConfig();
    return true;
}

/*!
 * \brief FileOperations::initEventHandle Initialize all event handling
 */
void FileOperations::initEventHandle()
{
    dpfSignalDispatcher->subscribe(GlobalEventType::kCopy,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>, const QUrl,
                                                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&FileOperationsEventReceiver::handleOperationCopy));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCutFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                     const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&FileOperationsEventReceiver::handleOperationCut));
    dpfSignalDispatcher->subscribe(GlobalEventType::kRestoreFromTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>, const QUrl,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&TrashFileEventReceiver::handleOperationRestoreFromTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCopyFromTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl> &, const QUrl &,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&TrashFileEventReceiver::handleOperationCopyFromTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCopyFromTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl> &, const QUrl &,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                AbstractJobHandler::OperatorCallback)>(&TrashFileEventReceiver::handleOperationCopyFromTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kMoveToTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&TrashFileEventReceiver::handleOperationMoveToTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kDeleteFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&FileOperationsEventReceiver::handleOperationDeletes));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCleanTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback)>(&TrashFileEventReceiver::handleOperationCleanTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCopy,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                     const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationCopy));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCutFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                     const QUrl, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationCut));
    dpfSignalDispatcher->subscribe(GlobalEventType::kRestoreFromTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>, const QUrl,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                AbstractJobHandler::OperatorCallback)>(&TrashFileEventReceiver::handleOperationRestoreFromTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kMoveToTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                AbstractJobHandler::OperatorCallback)>(&TrashFileEventReceiver::handleOperationMoveToTrash));
    dpfSignalDispatcher->subscribe(GlobalEventType::kDeleteFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationDeletes));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCleanTrash,
                                   TrashFileEventReceiver::instance(),
                                   static_cast<void (TrashFileEventReceiver::*)(const quint64, const QList<QUrl>,
                                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback, const QVariant,
                                                                                AbstractJobHandler::OperatorCallback)>(&TrashFileEventReceiver::handleOperationCleanTrash));

    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationOpenFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>)>(&FileOperationsEventReceiver::handleOperationOpenFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     bool *ok)>(&FileOperationsEventReceiver::handleOperationOpenFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFilesByApp,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QList<QString>)>(&FileOperationsEventReceiver::handleOperationOpenFilesByApp));
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFilesByApp,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QList<QString>,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationOpenFilesByApp));
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QUrl,
                                                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag)>(&FileOperationsEventReceiver::handleOperationRenameFile));
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QUrl,
                                                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationRenameFile));

    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QPair<QString, QString>,
                                                                                     const bool replace)>(&FileOperationsEventReceiver::handleOperationRenameFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QPair<QString, QString>,
                                                                                     const bool replace,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationRenameFiles));

    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QPair<QString, AbstractJobHandler::FileNameAddFlag>)>(&FileOperationsEventReceiver::handleOperationRenameFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QPair<QString, AbstractJobHandler::FileNameAddFlag>,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationRenameFiles));

    dpfSignalDispatcher->subscribe(GlobalEventType::kMkdir,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64, const QUrl)>(&FileOperationsEventReceiver::handleOperationMkdir));
    dpfSignalDispatcher->subscribe(GlobalEventType::kMkdir,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64, const QUrl,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationMkdir));
    dpfSignalDispatcher->subscribe(GlobalEventType::kTouchFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<QString (FileOperationsEventReceiver::*)(const quint64,
                                                                                        const QUrl,
                                                                                        const CreateFileType,
                                                                                        const QString)>(&FileOperationsEventReceiver::handleOperationTouchFile));
    dpfSignalDispatcher->subscribe(GlobalEventType::kTouchFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const CreateFileType,
                                                                                     const QString,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationTouchFile));

    dpfSignalDispatcher->subscribe(GlobalEventType::kTouchFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<QString (FileOperationsEventReceiver::*)(const quint64,
                                                                                        const QUrl,
                                                                                        const QUrl,
                                                                                        const QString)>(&FileOperationsEventReceiver::handleOperationTouchFile));
    dpfSignalDispatcher->subscribe(GlobalEventType::kTouchFile,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QUrl,
                                                                                     const QString,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationTouchFile));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCreateSymlink,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QUrl,
                                                                                     const bool force,
                                                                                     const bool silence)>(&FileOperationsEventReceiver::handleOperationLinkFile));
    dpfSignalDispatcher->subscribe(GlobalEventType::kCreateSymlink,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QUrl,
                                                                                     const bool force,
                                                                                     const bool silence,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationLinkFile));
    dpfSignalDispatcher->subscribe(GlobalEventType::kSetPermission,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QFileDevice::Permissions)>(&FileOperationsEventReceiver::handleOperationSetPermission));
    dpfSignalDispatcher->subscribe(GlobalEventType::kSetPermission,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QUrl,
                                                                                     const QFileDevice::Permissions,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationSetPermission));
    dpfSignalDispatcher->subscribe(GlobalEventType::kWriteUrlsToClipboard,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationWriteToClipboard);
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenInTerminal,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationOpenInTerminal);
    dpfSignalDispatcher->subscribe(GlobalEventType::kWriteCustomToClipboard,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationWriteDataToClipboard);
    dpfSignalDispatcher->subscribe(GlobalEventType::kSaveOperator,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationSaveOperations);
    dpfSignalDispatcher->subscribe(GlobalEventType::kCleanSaveOperator,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationCleanSaveOperationsStack);
    dpfSignalDispatcher->subscribe(GlobalEventType::kRevocation,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationRevocation);
    dpfSignalDispatcher->subscribe(GlobalEventType::kHideFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<bool (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>)>(&FileOperationsEventReceiver::handleOperationHideFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kHideFiles,
                                   FileOperationsEventReceiver::instance(),
                                   static_cast<void (FileOperationsEventReceiver::*)(const quint64,
                                                                                     const QList<QUrl>,
                                                                                     const QVariant,
                                                                                     AbstractJobHandler::OperatorCallback)>(&FileOperationsEventReceiver::handleOperationHideFiles));
    dpfSignalDispatcher->subscribe(GlobalEventType::kSaveRedoOperator,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationSaveRedoOperations);
    dpfSignalDispatcher->subscribe(GlobalEventType::kCleanSaveOperatorByUrls,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleOperationCleanByUrls);
    dpfSignalDispatcher->subscribe(GlobalEventType::kRedo,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleRecoveryOperationRedoRecovery);
    dpfSignalDispatcher->subscribe(GlobalEventType::kCopyFilePath,
                                   FileOperationsEventReceiver::instance(),
                                   &FileOperationsEventReceiver::handleCopyFilePath);

    dpfSlotChannel->connect("dfmplugin_fileoperations",
                            "slot_Operation_FilesPreview",
                            FileOperationsEventReceiver::instance(),
                            &FileOperationsEventReceiver::handleOperationFilesPreview);
}

void FileOperations::followEvents()
{
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_DeleteFiles",
                            FileOperationsEventReceiver::instance(), &FileOperationsEventReceiver::handleShortCut);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_MoveToTrash",
                            FileOperationsEventReceiver::instance(), &FileOperationsEventReceiver::handleShortCut);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PasteFiles",
                            FileOperationsEventReceiver::instance(), &FileOperationsEventReceiver::handleShortCutPaste);
    connect(
            DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [](const QString &iid, const QString &name) {
                Q_UNUSED(iid)
                if (name == "dfmplugin-search")
                    dpfHookSequence->follow("dfmplugin_search", "hook_Url_IsSubFile",
                                            FileOperationsEventReceiver::instance(),
                                            &FileOperationsEventReceiver::handleIsSubFile);
            },
            Qt::DirectConnection);
}

void FileOperations::regSettingConfig()
{
    SettingJsonGenerator::instance()->addGroup(kSettingGroup, tr("External storage device"));
    DialogManager::instance()->registerSettingWidget("syncModeItem", &OperationSettings::createSyncModeItem);
    CustomSettingItemRegister::instance()->registCustomSettingItemType("label",
                                                                       [](QObject *opt) -> QPair<QWidget *, QWidget *> {
                                                                           auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
                                                                           auto lab = new QLabel(qApp->translate("QObject", option->name().toStdString().c_str()));
                                                                           return qMakePair(lab, nullptr);
                                                                       });

    SettingJsonGenerator::instance()->addConfig(QString("%1.00_external_usage_pattern_label").arg(kSettingGroup),
                                                { { "key", "00_external_usage_pattern_label" },
                                                  { "name", tr("External storage device usage patterns") },
                                                  { "type", "label" } });
    SettingJsonGenerator::instance()->addConfig(QString("%1.01_sync_mode_item").arg(kSettingGroup),
                                                { { "key", "01_sync_mode_item" },
                                                  { "type", "syncModeItem" },
                                                  { "default", true } });

    SettingBackend::instance()->addSettingAccessor(
            QString("%1.01_sync_mode_item").arg(kSettingGroup),
            []() {
                return DConfigManager::instance()->value(kFileOperations, kBlockEverySync, true);
            },
            [](const QVariant &val) {
                DConfigManager::instance()->setValue(kFileOperations, kBlockEverySync, val);
            });
}
