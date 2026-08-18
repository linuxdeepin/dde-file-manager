// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menuscene/newcreatemenuscene.h"
#include "menuscene/action_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_NewCreateMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new NewCreateMenuScene();
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

protected:
    NewCreateMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

class UT_NewCreateMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new NewCreateMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    NewCreateMenuCreator *creator { nullptr };
};

// NewCreateMenuCreator tests

TEST_F(UT_NewCreateMenuCreator, Name_ReturnsNewCreateMenuName)
{
    EXPECT_EQ(NewCreateMenuCreator::name(), "NewCreateMenu");
}

TEST_F(UT_NewCreateMenuCreator, Create_ReturnsNewCreateMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "NewCreateMenu");
    delete scene;
}

// NewCreateMenuScene tests

TEST_F(UT_NewCreateMenuScene, Name_ReturnsNewCreateMenuName)
{
    EXPECT_EQ(scene->name(), "NewCreateMenu");
}

TEST_F(UT_NewCreateMenuScene, Initialize_InvalidCurrentDir_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl();
    params[MenuParamKey::kOnDesktop] = false;

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_NewCreateMenuScene, Initialize_ValidCurrentDir_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kWindowId] = 0ULL;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_NewCreateMenuScene, Initialize_EmptyArea_ReturnsTrue)
{
    // positive path with any isEmptyArea value; template scene push returns an
    // invalid variant in the test env and is safely skipped
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kIsEmptyArea] = true;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_NewCreateMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_NewCreateMenuScene, Scene_OwnAction_ReturnsSelf)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;

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

    QAction *newFolderAction = nullptr;
    for (QAction *action : menu.actions()) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kNewFolder)) {
            newFolderAction = action;
            break;
        }
    }
    ASSERT_NE(newFolderAction, nullptr);
    EXPECT_EQ(scene->scene(newFolderAction), scene);
}

TEST_F(UT_NewCreateMenuScene, Create_NullParent_ReturnsFalse)
{
    EXPECT_FALSE(scene->create(nullptr));
}

TEST_F(UT_NewCreateMenuScene, Create_ValidParent_AddsNewFolderAndNewDoc)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;

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
    EXPECT_GE(actions.size(), 2);   // NewFolder and NewDoc at least

    bool hasNewFolder = false;
    bool hasNewDoc = false;
    for (auto action : actions) {
        QString actionId = action->property(ActionPropertyKey::kActionID).toString();
        if (actionId == QString(ActionID::kNewFolder))
            hasNewFolder = true;
        if (actionId == QString(ActionID::kNewDoc))
            hasNewDoc = true;
    }

    EXPECT_TRUE(hasNewFolder);
    EXPECT_TRUE(hasNewDoc);
}

TEST_F(UT_NewCreateMenuScene, Triggered_NonOwnAction_ReturnsFalse)
{
    QAction action("test");

    stub.set_lamda(VADDR(AbstractMenuScene, triggered), [](AbstractMenuScene *, QAction *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_FALSE(scene->triggered(&action));
}

TEST_F(UT_NewCreateMenuScene, UpdateState_NotWritableDir_DisablesNewActions)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;   // not writable
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);
    scene->updateState(&menu);

    for (auto action : menu.actions()) {
        const QString &id = action->property(ActionPropertyKey::kActionID).toString();
        if (id == QString(ActionID::kNewFolder) || id == QString(ActionID::kNewDoc))
            EXPECT_FALSE(action->isEnabled());
    }
}

TEST_F(UT_NewCreateMenuScene, UpdateState_WritableDir_KeepsNewActionsEnabled)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
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
        const QString &id = action->property(ActionPropertyKey::kActionID).toString();
        if (id == QString(ActionID::kNewFolder) || id == QString(ActionID::kNewDoc))
            EXPECT_TRUE(action->isEnabled());
    }
}

TEST_F(UT_NewCreateMenuScene, UpdateState_NullDirInfo_ChangesNothing)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
    });

    // info creation fails: updateState returns early, actions stay enabled
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));

    for (auto action : menu.actions()) {
        const QString &id = action->property(ActionPropertyKey::kActionID).toString();
        if (id == QString(ActionID::kNewFolder) || id == QString(ActionID::kNewDoc))
            EXPECT_TRUE(action->isEnabled());
    }
}

TEST_F(UT_NewCreateMenuScene, Triggered_NewFolder_PublishesMkdirEvent)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kWindowId] = 0ULL;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool mkdirPublished = false;
    QUrl publishedDir;
    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, QUrl &);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType topic, quint64, QUrl &dir) {
                       __DBG_STUB_INVOKE__
                       if (topic == DFMBASE_NAMESPACE::GlobalEventType::kMkdir) {
                           mkdirPublished = true;
                           publishedDir = dir;
                       }
                       return true;
                   });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    for (auto action : menu.actions()) {
        if (action->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kNewFolder)) {
            EXPECT_TRUE(scene->triggered(action));
            break;
        }
    }

    EXPECT_TRUE(mkdirPublished);
    EXPECT_EQ(publishedDir, QUrl::fromLocalFile("/tmp"));
}

TEST_F(UT_NewCreateMenuScene, Triggered_NewOfficeText_PublishesTouchFileEvent)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool touchPublished = false;
    DFMBASE_NAMESPACE::Global::CreateFileType publishedType
        = DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeText;
    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64, QUrl &,
                                                        DFMBASE_NAMESPACE::Global::CreateFileType &&, QString &&);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&](EventDispatcherManager *, EventType topic, quint64, QUrl &,
                       DFMBASE_NAMESPACE::Global::CreateFileType type, QString) {
                       __DBG_STUB_INVOKE__
                       if (topic == DFMBASE_NAMESPACE::GlobalEventType::kTouchFile) {
                           touchPublished = true;
                           publishedType = type;
                       }
                       return true;
                   });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    QAction *officeAction = nullptr;
    for (auto action : menu.actions()) {
        if (auto docAction = static_cast<QAction *>(action)) {
            if (docAction->menu()) {
                for (auto subAction : docAction->menu()->actions()) {
                    if (subAction->property(ActionPropertyKey::kActionID).toString() == QString(ActionID::kNewOfficeText))
                        officeAction = subAction;
                }
            }
        }
    }
    ASSERT_NE(officeAction, nullptr);
    EXPECT_TRUE(scene->triggered(officeAction));

    EXPECT_TRUE(touchPublished);
    EXPECT_EQ(publishedType, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeWord);
}

TEST_F(UT_NewCreateMenuScene, Triggered_AllOwnActions_ReturnTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;

    stub.set_lamda(VADDR(AbstractMenuScene, initialize), [](AbstractMenuScene *, const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    QMenu menu;
    scene->create(&menu);

    // every action registered by create() must be recognized as an own action
    QStringList ownIds {
        ActionID::kNewFolder, ActionID::kNewDoc, ActionID::kNewOfficeText,
        ActionID::kNewSpreadsheets, ActionID::kNewPresentation, ActionID::kNewPlainText
    };

    QList<QAction *> allActions = menu.actions();
    for (auto action : allActions) {
        if (action->menu())
            allActions.append(action->menu()->actions());
    }

    for (const QString &id : ownIds) {
        bool triggered = false;
        for (QAction *action : allActions) {
            if (action->property(ActionPropertyKey::kActionID).toString() == id) {
                triggered = true;
                EXPECT_TRUE(scene->triggered(action)) << "action " << id.toStdString() << " should be handled";
                break;
            }
        }
        EXPECT_TRUE(triggered) << "action " << id.toStdString() << " should be registered by create()";
    }
}
