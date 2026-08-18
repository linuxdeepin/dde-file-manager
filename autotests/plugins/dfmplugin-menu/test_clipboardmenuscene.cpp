// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/clipboardmenuscene.h"
#include "menuscene/action_defines.h"
#include "menuscene/private/clipboardmenuscene_p.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QImage>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_ClipBoardMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new ClipBoardMenuScene();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        // MenuUtils::perfectMenuParams is static inline and can not be stubbed,
        // neutralize its environment probes instead.
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

// ClipBoardMenuCreator tests

TEST_F(UT_ClipBoardMenuCreator, Name_ReturnsClipBoardMenuName)
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

// ClipBoardMenuScene tests

TEST_F(UT_ClipBoardMenuScene, Name_ReturnsClipBoardMenuName)
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

    QAction *pasteAction = nullptr;
    for (QAction *action : menu.actions()) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kPaste)) {
            pasteAction = action;
            break;
        }
    }
    ASSERT_NE(pasteAction, nullptr);
    EXPECT_EQ(scene->scene(pasteAction), scene);
}

TEST_F(UT_ClipBoardMenuScene, Create_EmptyArea_AddsPasteActionOnly)
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
    ASSERT_FALSE(actions.isEmpty());
    EXPECT_EQ(actions.first()->property(ActionPropertyKey::kActionID).toString(), QString(ActionID::kPaste));
}

TEST_F(UT_ClipBoardMenuScene, Create_NormalFile_AddsCutAndCopyActions)
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
    // neither system path nor dde desktop file: Cut + Copy are both added
    EXPECT_GE(actions.size(), 2);

    bool hasCut = false;
    bool hasCopy = false;
    for (auto action : actions) {
        const QString &id = action->property(ActionPropertyKey::kActionID).toString();
        if (id == QString(ActionID::kCut))
            hasCut = true;
        if (id == QString(ActionID::kCopy))
            hasCopy = true;
    }
    EXPECT_TRUE(hasCut);
    EXPECT_TRUE(hasCopy);
}

TEST_F(UT_ClipBoardMenuScene, Create_SystemPathIncluded_SkipsCutAction)
{
    QUrl testUrl = QUrl::fromLocalFile("/usr/share/test.txt");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/usr/share");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsEmptyArea] = false;
    // provide all three keys so perfectMenuParams keeps them untouched
    params[MenuParamKey::kIsSystemPathIncluded] = true;
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = false;
    params[MenuParamKey::kIsFocusOnDDEDesktopFile] = false;

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
    bool hasCopy = false;
    for (auto action : actions) {
        const QString &id = action->property(ActionPropertyKey::kActionID).toString();
        if (id == QString(ActionID::kCut))
            hasCut = true;
        if (id == QString(ActionID::kCopy))
            hasCopy = true;
    }
    EXPECT_FALSE(hasCut);
    EXPECT_TRUE(hasCopy);
}

TEST_F(UT_ClipBoardMenuScene, Create_FocusOnDDEDesktopFile_NoActions)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/dde-computer.desktop");
    QList<QUrl> urls = { testUrl };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kIsSystemPathIncluded] = false;
    params[MenuParamKey::kIsDDEDesktopFileIncluded] = true;
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

    // focused on a dde desktop file: nothing is added at all
    EXPECT_TRUE(menu.actions().isEmpty());
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
        return true;   // writable
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);
    scene->updateState(&menu);

    auto actions = menu.actions();
    ASSERT_FALSE(actions.isEmpty());
    // no clipboard action and no image data: paste must be disabled
    EXPECT_FALSE(actions.first()->isEnabled());
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
        return false;   // not writable
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);
    scene->updateState(&menu);

    auto actions = menu.actions();
    ASSERT_FALSE(actions.isEmpty());
    EXPECT_FALSE(actions.first()->isEnabled());
}

TEST_F(UT_ClipBoardMenuScene, UpdateState_EmptyArea_CopyAction_EnablesPaste)
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
        return true;   // writable
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);
    scene->updateState(&menu);

    auto actions = menu.actions();
    ASSERT_FALSE(actions.isEmpty());
    // valid clipboard data + writable dir: paste stays enabled
    EXPECT_TRUE(actions.first()->isEnabled());
}

TEST_F(UT_ClipBoardMenuScene, UpdateState_EmptyArea_ImageInClipboard_EnablesPaste)
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

    // unknown clipboard action, but real image data in the system clipboard
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
        return true;   // writable
    });

    QMimeData *mimeData = new QMimeData;
    mimeData->setImageData(QVariant::fromValue(QImage(4, 4, QImage::Format_ARGB32)));
    QApplication::clipboard()->setMimeData(mimeData);

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);
    scene->updateState(&menu);

    auto actions = menu.actions();
    ASSERT_FALSE(actions.isEmpty());
    EXPECT_TRUE(actions.first()->isEnabled());

    // do not leak the image data into other test cases
    QApplication::clipboard()->clear();
}

TEST_F(UT_ClipBoardMenuScene, UpdateState_NotWritableDir_DisablesCut)
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
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;   // current dir is not writable
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);
    scene->updateState(&menu);

    for (auto action : menu.actions()) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kCut)) {
            EXPECT_FALSE(action->isEnabled());
        }
    }
}

TEST_F(UT_ClipBoardMenuScene, UpdateState_WritableDir_KeepsCutEnabled)
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
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;   // writable
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);
    scene->updateState(&menu);

    for (auto action : menu.actions()) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kCut)) {
            EXPECT_TRUE(action->isEnabled());
        }
    }
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

    bool copyPublished = false;
    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, QList<QUrl> &, QUrl &,
                                                        AbstractJobHandler::JobFlag &&, std::nullptr_t &&,
                                                        std::nullptr_t &&, QVariant &&, std::nullptr_t &&);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType topic, quint64, QList<QUrl> &, QUrl &,
                       AbstractJobHandler::JobFlag, std::nullptr_t, std::nullptr_t, QVariant, std::nullptr_t) {
                       __DBG_STUB_INVOKE__
                       if (topic == DFMBASE_NAMESPACE::GlobalEventType::kCopy)
                           copyPublished = true;
                       return true;
                   });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    auto actions = menu.actions();
    ASSERT_FALSE(actions.isEmpty());
    EXPECT_TRUE(scene->triggered(actions.first()));
    EXPECT_TRUE(copyPublished);
}

TEST_F(UT_ClipBoardMenuScene, Triggered_CutAction_PublishesWriteUrlsEvent)
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

    bool cutPublished = false;
    ClipBoard::ClipboardAction publishedAction = ClipBoard::kUnknownAction;
    QList<QUrl> publishedUrls;
    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64,
                                                        DFMBASE_NAMESPACE::ClipBoard::ClipboardAction &&,
                                                        QList<QUrl> &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType topic, quint64,
                       DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action, QList<QUrl> &selUrls) {
                       __DBG_STUB_INVOKE__
                       if (topic == DFMBASE_NAMESPACE::GlobalEventType::kWriteUrlsToClipboard) {
                           cutPublished = true;
                           publishedAction = action;
                           publishedUrls = selUrls;
                       }
                       return true;
                   });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    for (auto action : menu.actions()) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kCut)) {
            EXPECT_TRUE(scene->triggered(action));
            break;
        }
    }

    EXPECT_TRUE(cutPublished);
    EXPECT_EQ(publishedAction, ClipBoard::kCutAction);
    EXPECT_EQ(publishedUrls, urls);
}

TEST_F(UT_ClipBoardMenuScene, Triggered_TreeSelectFiles_UsesTreeUrls)
{
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { testUrl };
    QList<QUrl> treeUrls = {
        QUrl::fromLocalFile("/tmp/tree-1.txt"),
        QUrl::fromLocalFile("/tmp/tree-2.txt")
    };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(urls);
    params[MenuParamKey::kTreeSelectFiles] = QVariant::fromValue(treeUrls);
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

    QList<QUrl> publishedUrls;
    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64,
                                                        DFMBASE_NAMESPACE::ClipBoard::ClipboardAction &&,
                                                        QList<QUrl> &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType, quint64,
                       DFMBASE_NAMESPACE::ClipBoard::ClipboardAction, QList<QUrl> &selUrls) {
                       __DBG_STUB_INVOKE__
                       publishedUrls = selUrls;
                       return true;
                   });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    for (auto action : menu.actions()) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kCopy)) {
            EXPECT_TRUE(scene->triggered(action));
            break;
        }
    }

    // the count of treeSelectedUrls differs from selectFiles, the tree urls win
    EXPECT_EQ(publishedUrls, treeUrls);
}

TEST_F(UT_ClipBoardMenuScene, Triggered_RemoteCopiedAction_SetsCurUrlToClipboard)
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
        return ClipBoard::kRemoteCopiedAction;
    });

    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &, QList<QUrl> *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool remoteCalled = false;
    stub.set_lamda(&ClipBoard::setCurUrlToClipboardForRemote, [&remoteCalled](const QUrl &) {
        __DBG_STUB_INVOKE__
        remoteCalled = true;
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    auto actions = menu.actions();
    ASSERT_FALSE(actions.isEmpty());
    EXPECT_TRUE(scene->triggered(actions.first()));
    EXPECT_TRUE(remoteCalled);
}

TEST_F(UT_ClipBoardMenuScene, Triggered_RemoteAction_PublishesRemoteCopy)
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
        return ClipBoard::kRemoteAction;
    });

    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &, QList<QUrl> *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool remoteCopyPublished = false;
    AbstractJobHandler::JobFlag publishedFlag = AbstractJobHandler::JobFlag::kNoHint;   // overwritten below
    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, QList<QUrl> &, QUrl &,
                                                        AbstractJobHandler::JobFlag &&, std::nullptr_t &&);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType topic, quint64, QList<QUrl> &, QUrl &,
                       AbstractJobHandler::JobFlag flag, std::nullptr_t) {
                       __DBG_STUB_INVOKE__
                       if (topic == DFMBASE_NAMESPACE::GlobalEventType::kCopy) {
                           remoteCopyPublished = true;
                           publishedFlag = flag;
                       }
                       return true;
                   });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    auto actions = menu.actions();
    ASSERT_FALSE(actions.isEmpty());
    EXPECT_TRUE(scene->triggered(actions.first()));
    EXPECT_TRUE(remoteCopyPublished);
    EXPECT_EQ(publishedFlag, AbstractJobHandler::JobFlag::kCopyRemote);
}

TEST_F(UT_ClipBoardMenuScene, PrivateCtor_RegistersPredicateNames)
{
    ClipBoardMenuScenePrivate pri(scene);
    EXPECT_EQ(pri.parent(), scene);
    EXPECT_EQ(pri.predicateName.size(), 3);
    EXPECT_TRUE(pri.predicateName.contains(ActionID::kPaste));
    EXPECT_TRUE(pri.predicateName.contains(ActionID::kCut));
    EXPECT_TRUE(pri.predicateName.contains(ActionID::kCopy));
}

TEST_F(UT_ClipBoardMenuScene, Initialize_TreeSelectFiles_StoredInPrivate)
{
    QList<QUrl> treeUrls = {
        QUrl::fromLocalFile("/tmp/tree-a.txt"),
        QUrl::fromLocalFile("/tmp/tree-b.txt")
    };

    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/one.txt") });
    params[MenuParamKey::kTreeSelectFiles] = QVariant::fromValue(treeUrls);
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });
    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(scene->initialize(params));
    EXPECT_EQ(scene->d->treeSelectedUrls, treeUrls);
}
