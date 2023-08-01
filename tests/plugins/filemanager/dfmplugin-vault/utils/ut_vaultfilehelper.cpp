// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/vaultfilehelper.h"
#include "utils/vaulthelper.h"
#include "events/vaulteventcaller.h"

#include <gtest/gtest.h>

#include <QUrl>

#include <dfm-framework/event/eventdispatcher.h>

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/file/local/localfilehandler.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

TEST(UT_VaultFileHelper, cutFile_one)
{
    bool isOk =VaultFileHelper::instance()->cutFile(0, QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QUrl("file:///UT_TEST1"), AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileHelper, cutFile_two)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(dpf::EventType, quint64, QList<QUrl> &, const QUrl &, const AbstractJobHandler::JobFlags &, nullptr_t &&);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    QList<QUrl> sourceUrls;
    sourceUrls << QUrl("dfmvault:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->cutFile(0, sourceUrls , QUrl("dfmvault:///UT_TEST1"), AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, cutFile_three)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(dpf::EventType, quint64, QList<QUrl> &, const QUrl &, const AbstractJobHandler::JobFlags &, nullptr_t &&);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });
    stub.set_lamda(&FileUtils::isComputerDesktopFile, []{
        return true;
    });

    QList<QUrl> sourceUrls;
    sourceUrls << QUrl("dfmvault:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->cutFile(0, sourceUrls , QUrl("dfmvault:///UT_TEST1"), AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, copyFile_one)
{
    QList<QUrl> sourUrls;
    bool isOk = VaultFileHelper::instance()->copyFile(0, sourUrls, QUrl("file:///UT_TEST"), AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileHelper, copyFile_two)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(dpf::EventType, quint64, QList<QUrl> &, const QUrl &, const AbstractJobHandler::JobFlags &, nullptr_t &&);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    QList<QUrl> sourUrls;
    sourUrls << QUrl("dfmvault:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->copyFile(0, sourUrls, QUrl("dfmvault:///UT_TEST"), AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, copyFile_three)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(dpf::EventType, quint64, QList<QUrl> &, const QUrl &, const AbstractJobHandler::JobFlags &, nullptr_t &&);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });
    stub.set_lamda(&FileUtils::isComputerDesktopFile, []{
        return true;
    });

    QList<QUrl> sourUrls;
    sourUrls << QUrl("dfmvault:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->copyFile(0, sourUrls, QUrl("dfmvault:///UT_TEST"), AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, moveToTrash_one)
{
    QList<QUrl> sourceUrls;
    bool isOk = VaultFileHelper::instance()->moveToTrash(0, sourceUrls, AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileHelper, moveToTrash_two)
{
    QList<QUrl> sourceUrls;
    sourceUrls << QUrl("file:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->moveToTrash(0, sourceUrls, AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileHelper, moveToTrash_three)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, quint64, QList<QUrl> &, const AbstractJobHandler::JobFlags &, nullptr_t &&);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    QList<QUrl> sourceUrls;
    sourceUrls << QUrl("dfmvault:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->moveToTrash(0, sourceUrls, AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, deleteFile_one)
{
    QList<QUrl> sourceUrls;
    bool isOk = VaultFileHelper::instance()->deleteFile(0, sourceUrls, AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileHelper, deleteFile_two)
{
    QList<QUrl> sourceUrls;
    sourceUrls << QUrl("file:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->deleteFile(0, sourceUrls, AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileHelper, deleteFile_three)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, quint64, QList<QUrl> &, const AbstractJobHandler::JobFlags &, nullptr_t &&);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    QList<QUrl> sourceUrls;
    sourceUrls << QUrl("dfmvault:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->deleteFile(0, sourceUrls, AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, openFileInPlugin_one)
{
    QList<QUrl> urls;
    bool isOk = VaultFileHelper::instance()->openFileInPlugin(0, urls);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileHelper, openFileInPlugin_two)
{
    QList<QUrl> urls;
    urls << QUrl("file:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->openFileInPlugin(0, urls);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileHelper, openFileInPlugin_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultEventCaller::sendOpenFiles, []{
        int a = 0;
    });

    QList<QUrl> urls;
    urls << QUrl("dfmvault:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->openFileInPlugin(0, urls);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, renameFile_one)
{
    bool isOk = VaultFileHelper::instance()->renameFile(0, QUrl("file:///UT_TEST"), QUrl("dfmvault:///UT_TEST1"), AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileHelper, renameFile_two)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, quint64, const QUrl &, const QUrl &, const AbstractJobHandler::JobFlags &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    bool isOk = VaultFileHelper::instance()->renameFile(0, QUrl("dfmvault:///UT_TEST"), QUrl("dfmvault:///UT_TEST1"), AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, makeDir_one)
{
    bool isOk = VaultFileHelper::instance()->makeDir(0, QUrl("file:///UT_TEST"),
                                         QUrl("dfmvault:///UT_TEST1"),
                                         QVariant(1),
                                         nullptr);

    EXPECT_FALSE(isOk);
}

static void UTVaultFileHelperCallBackOne(const QSharedPointer<QMap<AbstractJobHandler::CallbackKey, QVariant>> map)
{
    Q_UNUSED(map);
    return;
}

TEST(UT_VaultFileHelper, makeDir_two)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, quint64, const QUrl &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    bool isOk = VaultFileHelper::instance()->makeDir(0, QUrl("dfmvault:///UT_TEST"),
                                         QUrl("dfmvault:///UT_TEST1"),
                                         QVariant(1),
                                         UTVaultFileHelperCallBackOne);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, touchFile_one)
{
    QString error;
    bool isOk = VaultFileHelper::instance()->touchFile(0, QUrl("file:///UT_TEST"),
                                                       QUrl("file:///UT_TEST/UT_TEST1"),
                                                       CreateFileType::kCreateFileTypeFolder, "",
                                                       QVariant(""), UTVaultFileHelperCallBackOne,
                                                       &error);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileHelper, touchFile_two)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, quint64, const QUrl &, const CreateFileType &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    QString error;
    bool isOk = VaultFileHelper::instance()->touchFile(0, QUrl("dfmvault:///UT_TEST"),
                                                       QUrl("dfmvault:///UT_TEST/UT_TEST1"),
                                                       CreateFileType::kCreateFileTypeFolder, "",
                                                       QVariant(""), UTVaultFileHelperCallBackOne,
                                                       &error);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, touchCustomFile_one)
{
    QString error;
    bool isOk = VaultFileHelper::instance()->touchCustomFile(0, QUrl("file:///UT_TEST"),
                                                             QUrl("file:///UT_TEST/UT_TEST1"),
                                                             QUrl("file:///UT_TEST/UT_TEMP"), "",
                                                             QVariant(""), UTVaultFileHelperCallBackOne,
                                                             &error);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileHelper, touchCustomFile_two)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, quint64, const QUrl &, const QUrl &, const QString &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    QString error;
    bool isOk = VaultFileHelper::instance()->touchCustomFile(0, QUrl("dfmvault:///UT_TEST"),
                                                             QUrl("dfmvault:///UT_TEST/UT_TEST1"),
                                                             QUrl("file:///UT_TEST/UT_TEMP"), "",
                                                             QVariant(""), UTVaultFileHelperCallBackOne,
                                                             &error);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, writeUrlsToClipboard)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, quint64, const ClipBoard::ClipboardAction &, QList<QUrl> &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    VaultFileHelper::instance()->writeUrlsToClipboard(0,
                                                      ClipBoard::ClipboardAction::kCopyAction,
                                                      QList<QUrl>());

    VaultFileHelper::instance()->writeUrlsToClipboard(0, ClipBoard::ClipboardAction::kCopyAction,
                                                      QList<QUrl>() << QUrl("file:///UT_TEST"));

    bool isOk = VaultFileHelper::instance()->writeUrlsToClipboard(0, ClipBoard::ClipboardAction::kCopyAction,
                                                                  QList<QUrl>() << QUrl("dfmvault:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, renameFiles)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, quint64, QList<QUrl> &, const QPair<QString, QString> &, bool &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    VaultFileHelper::instance()->renameFiles(0, QList<QUrl>(), QPair<QString, QString>(), true);
    VaultFileHelper::instance()->renameFiles(0, QList<QUrl>() << QUrl("file:///UT_TEST"), QPair<QString, QString>(), true);
    bool isOk = VaultFileHelper::instance()->renameFiles(0, QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QPair<QString, QString>(), true);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, renameFilesAddText)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, quint64, QList<QUrl> &, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    VaultFileHelper::instance()->renameFilesAddText(0, QList<QUrl>(), QPair<QString, AbstractJobHandler::FileNameAddFlag>());
    VaultFileHelper::instance()->renameFilesAddText(0, QList<QUrl>() << QUrl("file:///UT_TEST"), QPair<QString, AbstractJobHandler::FileNameAddFlag>());
    bool isOk = VaultFileHelper::instance()->renameFilesAddText(0, QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QPair<QString, AbstractJobHandler::FileNameAddFlag>());

    EXPECT_TRUE(isOk);

}

TEST(UT_VaultFileHelper, checkDragDropAction_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []{
        return true;
    });

    Qt::DropAction action;
    VaultFileHelper::instance()->checkDragDropAction(QList<QUrl>(), QUrl("file:///UT_TEST"), &action);
    VaultFileHelper::instance()->checkDragDropAction(QList<QUrl>() << QUrl("file:///UT_TEST"), QUrl(), &action);
    VaultFileHelper::instance()->checkDragDropAction(QList<QUrl>() << QUrl("file:///UT_TEST"), QUrl("file:///UT_TEST"), &action);
    bool isOk = VaultFileHelper::instance()->checkDragDropAction(QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QUrl("dfmvault:///UT_TEST"), &action);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, checkDragDropAction_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []{
        return false;
    });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []{
        return true;
    });

    Qt::DropAction action;
    bool isOk = VaultFileHelper::instance()->checkDragDropAction(QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QUrl("dfmvault:///UT_TEST"), &action);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, checkDragDropAction_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []{
        return false;
    });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []{
        return false;
    });

    Qt::DropAction action;
    bool isOk = VaultFileHelper::instance()->checkDragDropAction(QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QUrl("dfmvault:///UT_TEST"), &action);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, checkDragDropAction_four)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []{
        return false;
    });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []{
        return false;
    });

    Qt::DropAction action;
    bool isOk = VaultFileHelper::instance()->checkDragDropAction(QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QUrl("file:///UT_TEST"), &action);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, handleDropFiles_one)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, int, QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlag &&, nullptr_t &&);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []{
        return true;
    });

    VaultFileHelper::instance()->handleDropFiles(QList<QUrl>(), QUrl());
    VaultFileHelper::instance()->handleDropFiles(QList<QUrl>() << QUrl("file:///UT_TEST"), QUrl());
    VaultFileHelper::instance()->handleDropFiles(QList<QUrl>() << QUrl("file:///UT_TEST"), QUrl("file:///UT_TEST"));
    bool isOk = VaultFileHelper::instance()->handleDropFiles(QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QUrl("dfmvault:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, handleDropFiles_two)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, int, QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlag &&, nullptr_t &&);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []{
        return false;
    });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []{
        return true;
    });


    bool isOk = VaultFileHelper::instance()->handleDropFiles(QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QUrl("dfmvault:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, handleDropFiles_three)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, int, QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlag &&, nullptr_t &&);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []{
        return false;
    });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []{
        return false;
    });


    bool isOk = VaultFileHelper::instance()->handleDropFiles(QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QUrl("dfmvault:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, handleDropFiles_four)
{
    stub_ext::StubExt stub;
    typedef bool(EventDispatcherManager::*FuncType)(EventType, int, QList<QUrl> &, const QUrl &, AbstractJobHandler::JobFlag &&, nullptr_t &&);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []{
        return false;
    });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []{
        return false;
    });

    bool isOk = VaultFileHelper::instance()->handleDropFiles(QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, openFileByApp_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&DesktopFile::desktopExec, []{
        return "/UT_TEST/dde-file-manager";
    });
    typedef bool(EventDispatcherManager::*FuncType)(EventType, QUrl);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });

    VaultFileHelper::instance()->openFileByApp(0, QList<QUrl>(), QList<QString>());
    VaultFileHelper::instance()->openFileByApp(0, QList<QUrl>() << QUrl("file:///UT_TEST"), QList<QString>());
    VaultFileHelper::instance()->openFileByApp(0, QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QList<QString>());
    VaultFileHelper::instance()->openFileByApp(0, QList<QUrl>() << QUrl("dfmvault:///UT_TEST"), QList<QString>() << "");
    QList<QUrl> urls;
    urls << QUrl("dfmvault:///UT_TEST") << QUrl("dfmvault:///UT_TEST1");
    bool isOk = VaultFileHelper::instance()->openFileByApp(0, urls, QList<QString>() << "/UT_TEST.desktop");

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, openFileByApp_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&DesktopFile::desktopExec, []{
        return "/UT_TEST/dde-file-manager";
    });
    typedef bool(EventDispatcherManager::*FuncType)(EventType, QUrl);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });
    stub.set_lamda(&UniversalUtils::checkLaunchAppInterface, []{
        return true;
    });
    stub.set_lamda(&UniversalUtils::launchAppByDBus, []{
        return true;
    });

    QList<QUrl> urls;
    urls << QUrl("dfmvault:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->openFileByApp(0, urls, QList<QString>() << "/UT_TEST.desktop");

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, openFileByApp_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&DesktopFile::desktopExec, []{
        return "/UT_TEST/dde-file-manager";
    });
    typedef bool(EventDispatcherManager::*FuncType)(EventType, QUrl);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });
    stub.set_lamda(&UniversalUtils::checkLaunchAppInterface, []{
        return false;
    });
    stub.set_lamda(&UniversalUtils::launchAppByDBus, []{
        return true;
    });

    QList<QUrl> urls;
    urls << QUrl("dfmvault:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->openFileByApp(0, urls, QList<QString>() << "/UT_TEST.desktop");

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, openFileByApp_four)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&DesktopFile::desktopExec, []{
        return "/UT_TEST";
    });
    typedef bool(EventDispatcherManager::*FuncType)(EventType, int, QList<QUrl> &, const QList<QString> &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });
    stub.set_lamda(&UniversalUtils::checkLaunchAppInterface, []{
        return false;
    });
    stub.set_lamda(&UniversalUtils::launchAppByDBus, []{
        return true;
    });
    stub.set_lamda(&VaultHelper::urlsToLocal, []{
        return true;
    });

    QList<QUrl> urls;
    urls << QUrl("dfmvault:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->openFileByApp(0, urls, QList<QString>() << "/UT_TEST.desktop");

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, openFileByApp_five)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&DesktopFile::desktopExec, []{
        return "/UT_TEST";
    });
    typedef bool(EventDispatcherManager::*FuncType)(EventType, int, QList<QUrl> &, const QList<QString> &);
    stub.set_lamda(static_cast<FuncType>(&EventDispatcherManager::publish), []{
        return true;
    });
    stub.set_lamda(&UniversalUtils::checkLaunchAppInterface, []{
        return false;
    });
    stub.set_lamda(&UniversalUtils::launchAppByDBus, []{
        return true;
    });
    stub.set_lamda(&VaultHelper::urlsToLocal, []{
        return false;
    });

    QList<QUrl> urls;
    urls << QUrl("dfmvault:///UT_TEST");
    bool isOk = VaultFileHelper::instance()->openFileByApp(0, urls, QList<QString>() << "/UT_TEST.desktop");

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, setPermision_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultFileHelper::transUrlsToLocal, []{
        return QList<QUrl>();
    });

    bool ok;
    QString error;
    VaultFileHelper::instance()->setPermision(0, QUrl("file:///UT_TEST"),
                                              QFile::WriteOwner,
                                              &ok, &error);
    bool isOk = VaultFileHelper::instance()->setPermision(0, QUrl("dfmvault:///UT_TEST"),
                                                          QFile::WriteOwner,
                                                          &ok, &error);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileHelper, setPermision_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultFileHelper::transUrlsToLocal, []{
        return QList<QUrl>() << QUrl("file:///UT_TEST");
    });
    stub.set_lamda(&LocalFileHandler::setPermissions, []{
        return false;
    });

    bool ok;
    QString error;
    VaultFileHelper::instance()->setPermision(0, QUrl("file:///UT_TEST"),
                                              QFile::WriteOwner,
                                              &ok, &error);
    bool isOk = VaultFileHelper::instance()->setPermision(0, QUrl("dfmvault:///UT_TEST"),
                                                          QFile::WriteOwner,
                                                          &ok, &error);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileHelper, transUrlsToLocal)
{
    QList<QUrl> list;
    list << QUrl("file:///UT_TEST");
    VaultFileHelper::instance()->transUrlsToLocal(QList<QUrl>() << QUrl("dfmvault:///UT_TEST"));
    QList<QUrl> result = VaultFileHelper::instance()->transUrlsToLocal(list);

    EXPECT_TRUE(list == result);
}
