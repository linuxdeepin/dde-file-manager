// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "events/vaulteventreceiver.h"
#include "utils/vaultfilehelper.h"
#include "utils/vaulthelper.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilewatcher.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(QFileDevice::Permissions)
Q_DECLARE_METATYPE(bool *)

DPVAULT_USE_NAMESPACE
DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

TEST(UT_VaultEventReceiver, connectEvent)
{
    bool isConnect { false};

    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType1)(EventType, VaultEventReceiver *, void(VaultEventReceiver::*)(const quint64 &, const QUrl &));
    stub.set_lamda(static_cast<FuncType1>(&EventDispatcherManager::subscribe), []{
        return true;
    });
    typedef bool(EventDispatcherManager::*FuncType2)(const QString &, const QString &, VaultEventReceiver *, void(VaultEventReceiver::*)(quint64, const QUrl &));
    stub.set_lamda(static_cast<FuncType2>(&EventDispatcherManager::subscribe), [] {
        return true;
    });
    typedef bool(EventDispatcherManager::*FuncType3)(EventType, VaultEventReceiver *, void(VaultEventReceiver::*)(const quint64 &, const QList<QUrl> &, bool));
    stub.set_lamda(static_cast<FuncType3>(&EventDispatcherManager::subscribe), []{
        return true;
    });
    typedef bool(EventDispatcherManager::*FuncType4)(EventType, VaultEventReceiver *, bool(VaultEventReceiver::*)(quint64, const QUrl &));
    stub.set_lamda(static_cast<FuncType4>(&EventDispatcherManager::installEventFilter), []{
        return true;
    });

    typedef bool(EventSequenceManager::*FuncType5)(const QString &, const QString &, VaultEventReceiver *, bool(VaultEventReceiver::*)(const QList<QUrl> &, const QUrl &));
    stub.set_lamda(static_cast<FuncType5>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType6)(const QString &, const QString &, VaultEventReceiver *, bool(VaultEventReceiver::*)(const QList<QUrl> &, const QUrl &, Qt::DropAction *));
    stub.set_lamda(static_cast<FuncType6>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType7)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(const QList<QUrl> &, const QUrl &, Qt::DropAction *));
    stub.set_lamda(static_cast<FuncType7>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType8)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(const QList<QUrl> &, const QUrl &));
    stub.set_lamda(static_cast<FuncType8>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType9)(const QString &, const QString &, VaultEventReceiver *, bool(VaultEventReceiver::*)(const quint64 &, const QList<QUrl> &, const QUrl &));
    stub.set_lamda(static_cast<FuncType9>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType10)(const QString &, const QString &, VaultEventReceiver *, bool(VaultEventReceiver::*)(QList<QUrl>, QList<QUrl> *));
    stub.set_lamda(static_cast<FuncType10>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType11)(const QString &, const QString &, VaultEventReceiver *, bool(VaultEventReceiver::*)(const QUrl &, QString *));
    stub.set_lamda(static_cast<FuncType11>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType12)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(quint64, QList<QUrl>, QUrl, AbstractJobHandler::JobFlags));
    stub.set_lamda(static_cast<FuncType12>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType13)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(quint64, QList<QUrl>));
    stub.set_lamda(static_cast<FuncType13>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType14)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(quint64, QUrl, QUrl, AbstractJobHandler::JobFlags));
    stub.set_lamda(static_cast<FuncType14>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType15)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(quint64, QUrl, const QUrl &, QVariant, AbstractJobHandler::OperatorCallback));
    stub.set_lamda(static_cast<FuncType15>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType16)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(quint64, QUrl, const QUrl &, Global::CreateFileType, const QString &, const QVariant &, AbstractJobHandler::OperatorCallback, QString *));
    stub.set_lamda(static_cast<FuncType16>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType17)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(quint64, QUrl, const QUrl &, const QString &, const QVariant &, AbstractJobHandler::OperatorCallback, QString *));
    stub.set_lamda(static_cast<FuncType17>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType18)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(quint64, ClipBoard::ClipboardAction, QList<QUrl>));
    stub.set_lamda(static_cast<FuncType18>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType19)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(quint64, QList<QUrl>, QPair<QString, QString>, bool));
    stub.set_lamda(static_cast<FuncType19>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType20)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(quint64, QList<QUrl>, QPair<QString, AbstractJobHandler::FileNameAddFlag>));
    stub.set_lamda(static_cast<FuncType20>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType21)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(quint64, QList<QUrl>, QList<QString>));
    stub.set_lamda(static_cast<FuncType21>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType22)(const QString &, const QString &, VaultFileHelper *, bool(VaultFileHelper::*)(quint64, QUrl, QFlags<QFileDevice::Permission>, bool *, QString *));
    stub.set_lamda(static_cast<FuncType22>(&EventSequenceManager::follow), []{
        return true;
    });
    typedef bool(EventSequenceManager::*FuncType23)(const QString &, const QString &, VaultEventReceiver *, bool(VaultEventReceiver::*)(const QUrl &, bool *));
    stub.set_lamda(static_cast<FuncType23>(&EventSequenceManager::follow), [ &isConnect ]{
        isConnect = true;
        return true;
    });

    VaultEventReceiver::instance()->connectEvent();

    EXPECT_TRUE(isConnect);
}

TEST(UT_VaultEventReceiver, computerOpenItem_state_unlocked)
{
    bool isFinish { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{ return VaultState::kUnlocked; });
    stub.set_lamda(&VaultHelper::openWidWindow, [ &isFinish ]{ isFinish = true; });

    VaultEventReceiver::instance()->computerOpenItem(0, QUrl("file:///test_vault"));

    EXPECT_TRUE(isFinish);
}

TEST(UT_VaultEventReceiver, computerOpenItem_state_encrypted)
{
    bool isFinish { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{ return VaultState::kEncrypted; });
    stub.set_lamda(&VaultHelper::unlockVaultDialog, [ &isFinish ]{ isFinish = true; });

    VaultEventReceiver::instance()->computerOpenItem(0, QUrl("file:///test_vault"));

    EXPECT_TRUE(isFinish);
}

TEST(UT_VaultEventReceiver, computerOpenItem_state_notExisted)
{
    bool isFinish { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{ return VaultState::kNotExisted; });
    stub.set_lamda(&VaultHelper::createVaultDialog, [ &isFinish ]{ isFinish = true; });

    VaultEventReceiver::instance()->computerOpenItem(0, QUrl("file:///test_vault"));

    EXPECT_TRUE(isFinish);
}

TEST(UT_VaultEventReceiver, computerOpenItem_state_unknow)
{
    bool isFinish { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, [ &isFinish ]{
        isFinish = true;
        return VaultState::kUnknow;
    });

    VaultEventReceiver::instance()->computerOpenItem(0, QUrl("file:///test_vault"));

    EXPECT_TRUE(isFinish);
}

TEST(UT_VaultEventReceiver, handleNotAllowedAppendCompress_from)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &soucecUrls, QList<QUrl> *targetUrls){
        Q_UNUSED(soucecUrls)
        *targetUrls << QUrl::fromLocalFile(kVaultBasePath + QDir::separator() + kVaultDecryptDirName + QDir::separator() + "UT_TEST");
        return true;
    });

    bool isOk = VaultEventReceiver::instance()->handleNotAllowedAppendCompress(QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QUrl());

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultEventReceiver, handleNotAllowedAppendCompress_to)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &soucecUrls, QList<QUrl> *targetUrls){
        Q_UNUSED(soucecUrls)
        *targetUrls << QUrl::fromLocalFile(kVaultBasePath + QDir::separator() + kVaultDecryptDirName + QDir::separator() + "UT_TEST");
        return true;
    });

    bool isOk = VaultEventReceiver::instance()->handleNotAllowedAppendCompress(QList<QUrl>(), QUrl("dfmvault:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultEventReceiver, handleNotAllowedAppendCompress_error)
{
    bool isOk = VaultEventReceiver::instance()->handleNotAllowedAppendCompress(QList<QUrl>(), QUrl());

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultEventReceiver, handleCurrentUrlChanaged_append)
{
    bool isAppend { false };

    FileManagerWindowsManager::FMWindow *window = new FileManagerWindowsManager::FMWindow(QUrl());
    stub_ext::StubExt stub;
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [ &window ]{
        return window;
    });
    stub.set_lamda(&VaultHelper::appendWinID, [ &isAppend ]{ isAppend = true; });

    VaultEventReceiver::instance()->handleCurrentUrlChanged(0, QUrl("dfmvault:///UT_TEST"));

    if (window) {
        delete window;
        window = nullptr;
    }

    EXPECT_TRUE(isAppend);
}

TEST(UT_VaultEventReceiver, handleCurrentUrlChanaged_remove)
{
    bool isRemove { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::removeWinID, [ &isRemove ]{ isRemove = true; });

    VaultEventReceiver::instance()->handleCurrentUrlChanged(0, QUrl("dfmvault:///UT_TEST"));

    EXPECT_TRUE(isRemove);
}

TEST(UT_VaultEventReceiver, handleSideBarItemDragMoveData_one)
{
    Qt::DropAction action;
    bool isOk = VaultEventReceiver::instance()->handleSideBarItemDragMoveData(QList<QUrl>(), QUrl(), &action);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultEventReceiver, handleSideBarItemDragMoveData_two)
{
    Qt::DropAction action;
    bool isOk = VaultEventReceiver::instance()->handleSideBarItemDragMoveData(QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QUrl("tag:///UT_TEST"), &action);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultEventReceiver, handleSideBarItemDragMoveData_three)
{
    Qt::DropAction action;
    bool isOk = VaultEventReceiver::instance()->handleSideBarItemDragMoveData(QList<QUrl>() << QUrl("file:///UT_TEST"), QUrl("tag:///UT_TEST"), &action);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultEventReceiver, handleShortCutPasteFiles_one)
{
    bool isOk = VaultEventReceiver::instance()->handleShortCutPasteFiles(0, QList<QUrl>(), QUrl());

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultEventReceiver, handleShortCutPasteFiles_two)
{
    bool isOk = VaultEventReceiver::instance()->handleShortCutPasteFiles(0, QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QUrl("trash:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultEventReceiver, handleShortCutPasteFiles_three)
{
    bool isOk = VaultEventReceiver::instance()->handleShortCutPasteFiles(0, QList<QUrl>() << QUrl(), QUrl());

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultEventReceiver, handleHideFilesResult)
{
    bool isGetWatcher { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, []{
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl("file:///home/UT_TEST")));
    });
    stub.set_lamda(&WatcherCache::getCacheWatcher, [ &isGetWatcher ]{
        isGetWatcher = true;
        return QSharedPointer<LocalFileWatcher>(new LocalFileWatcher(QUrl("file:///home/UT_TEST")));
    });

    VaultEventReceiver::instance()->handleHideFilesResult(0, QList<QUrl>() << QUrl(), true);

    EXPECT_TRUE(isGetWatcher);
}

TEST(UT_VaultEventReceiver, changeUrlEventFilter_one)
{
    bool isCreate { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kNotExisted;
    });
    stub.set_lamda(&VaultHelper::createVaultDialog, [ &isCreate ]{
        isCreate = true;
    });

    VaultEventReceiver::instance()->changeUrlEventFilter(0, QUrl("dfmvault:///UT_TEST"));

    EXPECT_TRUE(isCreate);
}

TEST(UT_VaultEventReceiver, changeUrlEventFilter_two)
{
    bool isUnlock { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kEncrypted;
    });
    stub.set_lamda(&VaultHelper::unlockVaultDialog, [ &isUnlock ]{
        isUnlock = true;
    });

    VaultEventReceiver::instance()->changeUrlEventFilter(0, QUrl("dfmvault:///UT_TEST"));

    EXPECT_TRUE(isUnlock);
}

TEST(UT_VaultEventReceiver, changeUrlEventFilter_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kUnlocked;
    });

    bool isOk = VaultEventReceiver::instance()->changeUrlEventFilter(0, QUrl("dfmvault:///UT_TEST"));

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultEventReceiver, changeUrlEventFilter_four)
{
    bool isShow { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kNotAvailable;
    });
    stub.set_lamda(&DialogManager::showErrorDialog, [ &isShow ]{
        isShow = true;
    });

    VaultEventReceiver::instance()->changeUrlEventFilter(0, QUrl("dfmvault:///UT_TEST"));

    EXPECT_TRUE(isShow);
}

TEST(UT_VaultEventReceiver, changeUrlEventFilter_five)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kUnknow;
    });

    bool isOk = VaultEventReceiver::instance()->changeUrlEventFilter(0, QUrl("dfmvault:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultEventReceiver, changeUrlEventFilter_six)
{
    bool isOk = VaultEventReceiver::instance()->changeUrlEventFilter(0, QUrl());

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultEventReceiver, detailViewIcon_ok)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&UniversalUtils::urlEquals, []{
        return true;
    });

    QString iconName;
    bool isOk = VaultEventReceiver::instance()->detailViewIcon(QUrl("dfmvault:///"), &iconName);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultEventReceiver, detailViewIcon_cancel)
{
    QString iconName;
    bool isOk = VaultEventReceiver::instance()->detailViewIcon(QUrl("file:///UT_TEST"), &iconName);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultEventReceiver, handlePathtoVirtual_one)
{
    QList<QUrl> virtualUrl;
    bool isOk = VaultEventReceiver::instance()->handlePathtoVirtual(QList<QUrl>(), &virtualUrl);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultEventReceiver, handlePathtoVirtual_two)
{
    QList<QUrl> virtualUrl;
    bool isOk = VaultEventReceiver::instance()->handlePathtoVirtual(QList<QUrl>() << QUrl(), &virtualUrl);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultEventReceiver, handlePathtoVirtual_three)
{
    QList<QUrl> virtualUrl;
    bool isOk = VaultEventReceiver::instance()->handlePathtoVirtual(QList<QUrl>() << QUrl("dfmvault:////"), &virtualUrl);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultEventReceiver, fileDropHandleWithAction_one)
{
    Qt::DropAction action;
    VaultEventReceiver::instance()->fileDropHandleWithAction(QList<QUrl>(), QUrl("dfmvault:///"), &action);

    EXPECT_TRUE(action == Qt::CopyAction);
}

TEST(UT_VaultEventReceiver, fileDropHandleWithAction_two)
{
    Qt::DropAction action;
    VaultEventReceiver::instance()->fileDropHandleWithAction(QList<QUrl>() << QUrl("dfmvault:///"), QUrl("file:///UT_TEST"), &action);

    EXPECT_TRUE(action == Qt::CopyAction);
}

TEST(UT_VaultEventReceiver, fileDropHandleWithAction_three)
{
    Qt::DropAction action;
    bool isOk = VaultEventReceiver::instance()->fileDropHandleWithAction(QList<QUrl>(), QUrl(), &action);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultEventReceiver, handlePermissionViewAsh_no)
{
    bool isAsh { false };
    VaultEventReceiver::instance()->handlePermissionViewAsh(QUrl("file:///UT_TEST"), &isAsh);

    EXPECT_FALSE(isAsh);
}

TEST(UT_VaultEventReceiver, handlePermissionViewAsh_yes)
{
    bool isAsh { false };
    VaultEventReceiver::instance()->handlePermissionViewAsh(QUrl("dfmvault:///UT_TEST"), &isAsh);

    EXPECT_TRUE(isAsh);
}

TEST(UT_VaultEventReceiver, handleFileCanTaged_yes)
{
    bool canTag { true };
    VaultEventReceiver::instance()->handleFileCanTaged(QUrl("dfmvault:///"), &canTag);

    EXPECT_FALSE(canTag);
}

TEST(UT_VaultEventReceiver, handleFileCanTaged_no)
{
    bool canTag { false };
    bool isOk = VaultEventReceiver::instance()->handleFileCanTaged(QUrl("file:///UT_TEST"), &canTag);

    EXPECT_FALSE(isOk);
}


