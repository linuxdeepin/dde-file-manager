// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/fileoperatormenuscene.h"
#include "menuscene/action_defines.h"
#include "menuscene/private/fileoperatormenuscene_p.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class FomsDispatchReceiver : public QObject
{
public:
    explicit FomsDispatchReceiver(QObject *parent = nullptr)
        : QObject(parent) { }

    QVariant onOpenNewWindow(const QUrl &url)
    {
        lastUrl = url;
        return QVariant(true);
    }

    QVariant onOpenFiles(quint64 id, QList<QUrl> urls)
    {
        lastWindowId = id;
        lastUrls = urls;
        return QVariant(true);
    }

    QVariant onChangeCurrentUrl(quint64 id, QUrl url)
    {
        lastWindowId = id;
        lastUrl = url;
        return QVariant(true);
    }

    QUrl lastUrl;
    quint64 lastWindowId { 0 };
    QList<QUrl> lastUrls;
};

class UT_FileOperatorMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new FileOperatorMenuScene();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

    void stubPerfectMenuParams()
    {
        stub.set_lamda(&FileUtils::isComputerDesktopFile, [](const QUrl &) {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&FileUtils::isTrashDesktopFile, [](const QUrl &) {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&FileUtils::isHomeDesktopFile, [](const QUrl &) {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(ADDR(SystemPathUtil, isSystemPath), [](SystemPathUtil *, QString) {
            __DBG_STUB_INVOKE__
            return false;
        });
    }

    void stubInitializeBase()
    {
        stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                       [] {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
    }

    void stubFileInfoCreate()
    {
        stub.set_lamda(&InfoFactory::create<FileInfo>,
                       [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                           __DBG_STUB_INVOKE__
                           return QSharedPointer<FileInfo>(new FileInfo(url));
                       });
    }

protected:
    FileOperatorMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

class UT_FileOperatorMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new FileOperatorMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    FileOperatorMenuCreator *creator { nullptr };
};

TEST_F(UT_FileOperatorMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(FileOperatorMenuCreator::name(), "FileOperatorMenu");
}

TEST_F(UT_FileOperatorMenuCreator, Create_ReturnsFileOperatorMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "FileOperatorMenu");
    delete scene;
}

TEST_F(UT_FileOperatorMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "FileOperatorMenu");
}

TEST_F(UT_FileOperatorMenuScene, Initialize_EmptySelectFiles_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());
    params[MenuParamKey::kIsEmptyArea] = false;

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_FileOperatorMenuScene, Initialize_ValidFiles_ReturnsTrue)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_FileOperatorMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_FileOperatorMenuScene, Create_ValidParent_AddsActions)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    EXPECT_GE(menu.actions().size(), 0);
}

TEST_F(UT_FileOperatorMenuScene, Scene_OwnAction_ReturnsThis)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    scene->initialize(params);
    QMenu menu;
    scene->create(&menu);

    ASSERT_FALSE(menu.actions().isEmpty());
    EXPECT_EQ(scene->scene(menu.actions().first()), scene);
}

TEST_F(UT_FileOperatorMenuScene, Triggered_NonOwnAction_ReturnsFalse)
{
    QAction action("test");

    stub.set_lamda(VADDR(AbstractMenuScene, triggered), [](AbstractMenuScene *, QAction *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_FALSE(scene->triggered(&action));
}

TEST_F(UT_FileOperatorMenuScene, UpdateState_NullParent_ReturnsEarly)
{
    EXPECT_NO_FATAL_FAILURE(scene->updateState(nullptr));
}

TEST_F(UT_FileOperatorMenuScene, UpdateState_EmptyArea_ReturnsEarly)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());
    params[MenuParamKey::kIsEmptyArea] = true;

    stubInitializeBase();

    EXPECT_TRUE(scene->initialize(params));

    QMenu menu;
    QAction *others = menu.addAction("cut");
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));
    EXPECT_TRUE(others->isEnabled());
}

TEST_F(UT_FileOperatorMenuScene, UpdateState_TreeSelectMismatch_DisablesRename)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kTreeSelectFiles] = QVariant::fromValue(
            QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt"), QUrl::fromLocalFile("/tmp/other.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(VADDR(FileInfo, canAttributes), [](FileInfo *, const CanableInfoType) {
        __DBG_STUB_INVOKE__
        return true;
    });

    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    menu.addAction("cut");
    menu.addAction("copy");
    menu.addAction("paste");
    ASSERT_TRUE(scene->create(&menu));

    QAction *renameAction { nullptr };
    QAction *deleteAction { nullptr };
    for (auto act : menu.actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString() == ActionID::kRename)
            renameAction = act;
        else if (act->property(ActionPropertyKey::kActionID).toString() == ActionID::kDelete)
            deleteAction = act;
    }
    ASSERT_NE(renameAction, nullptr);
    ASSERT_NE(deleteAction, nullptr);

    scene->updateState(&menu);

    EXPECT_FALSE(renameAction->isEnabled());
    EXPECT_TRUE(deleteAction->isEnabled());
}

TEST_F(UT_FileOperatorMenuScene, UpdateState_CanNotDelete_DisablesDelete)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(VADDR(FileInfo, canAttributes), [](FileInfo *, const CanableInfoType type) {
        __DBG_STUB_INVOKE__
        return type != CanableInfoType::kCanDelete;
    });

    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *deleteAction { nullptr };
    for (auto act : menu.actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString() == ActionID::kDelete)
            deleteAction = act;
    }
    ASSERT_NE(deleteAction, nullptr);

    scene->updateState(&menu);

    EXPECT_FALSE(deleteAction->isEnabled());
}

TEST_F(UT_FileOperatorMenuScene, UpdateState_EmptyTrashWithNoChild_DisablesEmptyTrash)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();

    ASSERT_TRUE(scene->initialize(params));

    stub.set_lamda(&FileUtils::isTrashDesktopFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(FileInfo, countChildFile), [](FileInfo *) {
        __DBG_STUB_INVOKE__
        return 0;
    });

    QMenu menu;
    QAction emptyTrashAction("empty");
    emptyTrashAction.setProperty(ActionPropertyKey::kActionID, QString(ActionID::kEmptyTrash));
    menu.addAction(&emptyTrashAction);
    scene->d->predicateAction[ActionID::kEmptyTrash] = &emptyTrashAction;

    scene->updateState(&menu);

    EXPECT_FALSE(emptyTrashAction.isEnabled());
}

TEST_F(UT_FileOperatorMenuScene, UpdateState_OpenActionNotExists_DisablesOpen)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();

    ASSERT_TRUE(scene->initialize(params));

    stub.set_lamda(VADDR(FileInfo, exists), [](FileInfo *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    QMenu menu;
    QAction openAction("open");
    openAction.setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOpen));
    menu.addAction(&openAction);
    scene->d->predicateAction[ActionID::kOpen] = &openAction;

    scene->updateState(&menu);

    EXPECT_FALSE(openAction.isEnabled());
}

TEST_F(UT_FileOperatorMenuScene, UpdateState_WallpaperOnRemoteFile_DisablesWallpaper)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();

    ASSERT_TRUE(scene->initialize(params));

    stub.set_lamda(&ProtocolUtils::isRemoteFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    QMenu menu;
    QAction wallpaperAction("wallpaper");
    wallpaperAction.setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSetAsWallpaper));
    menu.addAction(&wallpaperAction);
    scene->d->predicateAction[ActionID::kSetAsWallpaper] = &wallpaperAction;

    scene->updateState(&menu);

    EXPECT_FALSE(wallpaperAction.isEnabled());
}

TEST_F(UT_FileOperatorMenuScene, UpdateState_DelegatesToBase)
{
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&baseCalled](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        baseCalled = true;
    });

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());
    params[MenuParamKey::kIsEmptyArea] = false;
    stubInitializeBase();
    scene->initialize(params);

    QMenu menu;
    scene->updateState(&menu);
    EXPECT_TRUE(baseCalled);
}

static QAction *fomsFindActionByText(QMenu *menu, const QString &text)
{
    for (auto act : menu->actions()) {
        if (act->text() == text)
            return act;
    }
    return nullptr;
}

TEST_F(UT_FileOperatorMenuScene, Triggered_OpenAction_SingleDir_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/testdir") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType type) {
        __DBG_STUB_INVOKE__
        return type == OptInfoType::kIsDir;
    });
    stub.set_lamda(ADDR(DConfigManager, value),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(true);
                   });
    stub.set_lamda(&Application::appAttribute, [](Application::ApplicationAttribute) -> QVariant {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *openAction = fomsFindActionByText(&menu, scene->d->predicateName.value(ActionID::kOpen));
    ASSERT_NE(openAction, nullptr);
    openAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOpen));

    EXPECT_TRUE(scene->triggered(openAction));
}

TEST_F(UT_FileOperatorMenuScene, Triggered_OpenAction_OnDesktop_ReturnsTrue)
{
    FomsDispatchReceiver receiver;
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFiles, &receiver, &FomsDispatchReceiver::onOpenFiles);

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(
            QList<QUrl> { QUrl::fromLocalFile("/tmp/a.txt"), QUrl::fromLocalFile("/tmp/b.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kWindowId] = static_cast<quint64>(88);

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *openAction = fomsFindActionByText(&menu, scene->d->predicateName.value(ActionID::kOpen));
    ASSERT_NE(openAction, nullptr);
    openAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOpen));

    EXPECT_TRUE(scene->triggered(openAction));
    EXPECT_EQ(receiver.lastWindowId, static_cast<quint64>(88));
    EXPECT_EQ(receiver.lastUrls.size(), 2);

    dpfSignalDispatcher->unsubscribe(GlobalEventType::kOpenFiles, &receiver, &FomsDispatchReceiver::onOpenFiles);
}

TEST_F(UT_FileOperatorMenuScene, Triggered_OpenAction_MultiFilesWithSymLinkDir_ReturnsTrue)
{
    FomsDispatchReceiver receiver;
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenNewWindow, &receiver, &FomsDispatchReceiver::onOpenNewWindow);
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenFiles, &receiver, &FomsDispatchReceiver::onOpenFiles);

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(
            QList<QUrl> { QUrl::fromLocalFile("/tmp/dir1"), QUrl::fromLocalFile("/tmp/dir2") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType type) {
        __DBG_STUB_INVOKE__
        return type == OptInfoType::kIsDir || type == OptInfoType::kIsSymLink;
    });
    stub.set_lamda(VADDR(FileInfo, pathOf), [](FileInfo *, const PathInfoType) {
        __DBG_STUB_INVOKE__
        return QString("/tmp/target");
    });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *openAction = fomsFindActionByText(&menu, scene->d->predicateName.value(ActionID::kOpen));
    ASSERT_NE(openAction, nullptr);
    openAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOpen));

    EXPECT_TRUE(scene->triggered(openAction));
    EXPECT_EQ(receiver.lastUrl, QUrl::fromLocalFile("/tmp/target"));

    dpfSignalDispatcher->unsubscribe(GlobalEventType::kOpenNewWindow, &receiver, &FomsDispatchReceiver::onOpenNewWindow);
    dpfSignalDispatcher->unsubscribe(GlobalEventType::kOpenFiles, &receiver, &FomsDispatchReceiver::onOpenFiles);
}

TEST_F(UT_FileOperatorMenuScene, Triggered_RenameAction_FallsToBaseReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *renameAction = fomsFindActionByText(&menu, scene->d->predicateName.value(ActionID::kRename));
    ASSERT_NE(renameAction, nullptr);
    renameAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kRename));

    EXPECT_FALSE(scene->triggered(renameAction));
}

TEST_F(UT_FileOperatorMenuScene, Triggered_DeleteAction_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    ASSERT_TRUE(scene->create(&menu));

    QAction *deleteAction = fomsFindActionByText(&menu, scene->d->predicateName.value(ActionID::kDelete));
    ASSERT_NE(deleteAction, nullptr);
    deleteAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDelete));

    EXPECT_TRUE(scene->triggered(deleteAction));
}

TEST_F(UT_FileOperatorMenuScene, Triggered_EmptyTrashAction_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();

    ASSERT_TRUE(scene->initialize(params));

    QMenu menu;
    QAction emptyTrashAction("empty");
    emptyTrashAction.setProperty(ActionPropertyKey::kActionID, QString(ActionID::kEmptyTrash));
    menu.addAction(&emptyTrashAction);
    scene->d->predicateAction[ActionID::kEmptyTrash] = &emptyTrashAction;

    EXPECT_TRUE(scene->triggered(&emptyTrashAction));
}

TEST_F(UT_FileOperatorMenuScene, Triggered_SetAsWallpaperAction_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/pic.jpg") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubPerfectMenuParams();
    stubFileInfoCreate();
    stubInitializeBase();

    ASSERT_TRUE(scene->initialize(params));

    bool setBackgroundCalled = false;
    stub.set_lamda(&FileUtils::setBackGround, [&setBackgroundCalled](const QString &) {
        __DBG_STUB_INVOKE__
        setBackgroundCalled = true;
        return true;
    });

    QMenu menu;
    QAction wallpaperAction("wallpaper");
    wallpaperAction.setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSetAsWallpaper));
    menu.addAction(&wallpaperAction);
    scene->d->predicateAction[ActionID::kSetAsWallpaper] = &wallpaperAction;

    EXPECT_TRUE(scene->triggered(&wallpaperAction));
    EXPECT_TRUE(setBackgroundCalled);
}
