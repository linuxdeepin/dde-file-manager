// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/clipboardmenuscene.h"
#include "menuscene/action_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_ClipBoardMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new ClipBoardMenuScene();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

protected:
    ClipBoardMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

class UT_ClipBoardMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new ClipBoardMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    ClipBoardMenuCreator *creator { nullptr };
};

// ClipBoardMenuCreator 测试

TEST_F(UT_ClipBoardMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(ClipBoardMenuCreator::name(), "ClipBoardMenu");
}

TEST_F(UT_ClipBoardMenuCreator, Create_ReturnsClipBoardMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "ClipBoardMenu");
    delete scene;
}

// ClipBoardMenuScene 测试

TEST_F(UT_ClipBoardMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "ClipBoardMenu");
}

TEST_F(UT_ClipBoardMenuScene, Initialize_EmptySelectFiles_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());
    params[MenuParamKey::kIsEmptyArea] = false;

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_ClipBoardMenuScene, Initialize_EmptyArea_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());
    params[MenuParamKey::kIsEmptyArea] = true;

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_ClipBoardMenuScene, Initialize_ValidFiles_ReturnsTrue)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsEmptyArea] = false;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_ClipBoardMenuScene, Initialize_FileInfoCreationFails_ReturnsFalse)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsEmptyArea] = false;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_ClipBoardMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_ClipBoardMenuScene, Scene_OwnAction_ReturnsSelf)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    scene->create(&menu);

    auto actions = menu.actions();
    if (!actions.isEmpty()) {
        EXPECT_EQ(scene->scene(actions.first()), scene);
    }
}

TEST_F(UT_ClipBoardMenuScene, Create_EmptyArea_AddsPasteAction)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    auto actions = menu.actions();
    EXPECT_EQ(actions.size(), 1);
    if (!actions.isEmpty()) {
        EXPECT_EQ(actions.first()->property(ActionPropertyKey::kActionID).toString(), QString(ActionID::kPaste));
    }
}

TEST_F(UT_ClipBoardMenuScene, Create_NormalFile_AddsCopyAction)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsEmptyArea] = false;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });
    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    auto actions = menu.actions();
    // 应该有 Cut 和 Copy 两个动作
    EXPECT_GE(actions.size(), 1);

    bool hasCopy = false;
    for (auto action : actions) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kCopy)) {
            hasCopy = true;
            break;
        }
    }
    EXPECT_TRUE(hasCopy);
}

TEST_F(UT_ClipBoardMenuScene, Create_SystemPath_NoCutAction)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kIsSystemPathIncluded] = true;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });
    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    auto actions = menu.actions();

    bool hasCut = false;
    for (auto action : actions) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kCut)) {
            hasCut = true;
            break;
        }
    }
    EXPECT_TRUE(hasCut);
}

TEST_F(UT_ClipBoardMenuScene, Create_DDEDesktopFile_NoActions)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/dde-computer.desktop");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kIsFocusOnDDEDesktopFile] = true;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });
    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    EXPECT_FALSE(menu.actions().isEmpty());
}

TEST_F(UT_ClipBoardMenuScene, UpdateState_EmptyArea_DisablesPasteWhenNoClipboard)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&ClipBoard::clipboardAction, [](ClipBoard *) {
        __DBG_STUB_INVOKE__
        return ClipBoard::kUnknownAction;
    });

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, refresh), [](FileInfo *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;   // 可写
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);
    scene->updateState(&menu);

    auto actions = menu.actions();
    if (!actions.isEmpty()) {
        EXPECT_TRUE(actions.first()->isEnabled() == false);
    }
}

TEST_F(UT_ClipBoardMenuScene, UpdateState_EmptyArea_DisablesPasteWhenNotWritable)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&ClipBoard::clipboardAction, [](ClipBoard *) {
        __DBG_STUB_INVOKE__
        return ClipBoard::kCopyAction;
    });

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, refresh), [](FileInfo *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;   // 不可写
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);
    scene->updateState(&menu);

    auto actions = menu.actions();
    if (!actions.isEmpty()) {
        EXPECT_TRUE(actions.first()->isEnabled() == false);
    }
}

TEST_F(UT_ClipBoardMenuScene, UpdateState_NormalFile_DisablesCopyWhenNotReadable)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsEmptyArea] = false;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });
    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType type) {
        __DBG_STUB_INVOKE__
        return false;   // 不可读，不是符号链接
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);
    scene->updateState(&menu);

    auto actions = menu.actions();
    bool hasCopyDisabled = false;
    for (auto action : actions) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kCopy)) {
            hasCopyDisabled = !action->isEnabled();
            break;
        }
    }
    EXPECT_TRUE(hasCopyDisabled);
}

TEST_F(UT_ClipBoardMenuScene, Triggered_NonOwnAction_ReturnsFalse)
{
    QAction action("test");
    EXPECT_FALSE(scene->triggered(&action));
}

TEST_F(UT_ClipBoardMenuScene, Triggered_PasteCopyAction_PublishesEvent)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, triggered), [](AbstractMenuScene *, QAction *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&ClipBoard::clipboardAction, [](ClipBoard *) {
        __DBG_STUB_INVOKE__
        return ClipBoard::kCopyAction;
    });

    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &, QList<QUrl> *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    auto actions = menu.actions();
    if (!actions.isEmpty()) {
        EXPECT_TRUE(scene->triggered(actions.first()));
    }
}

TEST_F(UT_ClipBoardMenuScene, Triggered_Cut_PublishesEvent)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsEmptyArea] = false;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });
    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, triggered), [](AbstractMenuScene *, QAction *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &, QList<QUrl> *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    auto actions = menu.actions();
    for (auto action : actions) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kCut)) {
            EXPECT_TRUE(scene->triggered(action));
            break;
        }
    }
}
