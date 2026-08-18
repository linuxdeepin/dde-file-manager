// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "extendmenuscene/extendmenuscene.h"
#include "extendmenuscene/extendmenu/dcustomactionparser.h"
#include "extendmenuscene/extendmenu/dcustomactionbuilder.h"
#include "extendmenuscene/private/extendmenuscene_p.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_ExtendMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        parser = new DCustomActionParser();
        scene = new ExtendMenuScene(parser);
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    virtual void TearDown() override
    {
        delete scene;
        scene = nullptr;
        delete parser;
        parser = nullptr;
        stub.clear();
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
    ExtendMenuScene *scene { nullptr };
    DCustomActionParser *parser { nullptr };
    stub_ext::StubExt stub;
};

class UT_ExtendMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        creator = new ExtendMenuCreator();
    }

    virtual void TearDown() override
    {
        delete creator;
        creator = nullptr;
    }

protected:
    ExtendMenuCreator *creator { nullptr };
};

TEST_F(UT_ExtendMenuCreator, Name_ReturnsCorrectName)
{
    EXPECT_EQ(ExtendMenuCreator::name(), "ExtendMenu");
}

TEST_F(UT_ExtendMenuCreator, Create_ReturnsExtendMenuScene)
{
    auto scene = creator->create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "ExtendMenu");
    delete scene;
}

TEST_F(UT_ExtendMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "ExtendMenu");
}

TEST_F(UT_ExtendMenuScene, Initialize_ValidParams_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kIsEmptyArea] = true;

    stubInitializeBase();

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_ExtendMenuScene, Initialize_NonEmptyArea_CreatesFileInfo)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubFileInfoCreate();
    stubInitializeBase();

    EXPECT_TRUE(scene->initialize(params));
}

TEST_F(UT_ExtendMenuScene, Initialize_FileInfoCreationFails_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    stub.set_lamda(VADDR(AbstractMenuScenePrivate, initializeParamsIsValid),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_FALSE(scene->initialize(params));
}

TEST_F(UT_ExtendMenuScene, Scene_NullAction_ReturnsNull)
{
    EXPECT_EQ(scene->scene(nullptr), nullptr);
}

TEST_F(UT_ExtendMenuScene, Scene_ExtendAction_ReturnsThis)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stubInitializeBase();

    stub.set_lamda(&DCustomActionParser::getActionFiles, [](DCustomActionParser *, bool) {
        __DBG_STUB_INVOKE__
        return QList<DCustomActionEntry>();
    });

    scene->initialize(params);
    QMenu menu;
    scene->create(&menu);

    QAction action("test");
    EXPECT_TRUE(scene->scene(&action) == nullptr || scene->scene(&action) == scene);
}

TEST_F(UT_ExtendMenuScene, Scene_ExtendChildAction_ReturnsThis)
{
    QAction childAction("child");
    scene->d->extendChildActions.append(&childAction);
    EXPECT_EQ(scene->scene(&childAction), scene);
}

TEST_F(UT_ExtendMenuScene, Create_NullParent_ReturnsTrue)
{
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(scene->create(nullptr));
}

TEST_F(UT_ExtendMenuScene, Create_EmptyRootEntry_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stubInitializeBase();

    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DCustomActionParser::getActionFiles, [](DCustomActionParser *, bool) {
        __DBG_STUB_INVOKE__
        return QList<DCustomActionEntry>();
    });

    scene->initialize(params);
    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
}

TEST_F(UT_ExtendMenuScene, Create_WithValidActions_AddsActions)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kIsEmptyArea] = true;

    stubInitializeBase();

    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    DCustomActionEntry entry;
    DCustomActionData data;
    data.actionName = "Test Action";
    data.actionCommand = "echo test";
    entry.actionData = data;
    entry.actionFileCombo = DCustomActionDefines::kBlankSpace;

    stub.set_lamda(&DCustomActionParser::getActionFiles, [&entry](DCustomActionParser *, bool) {
        __DBG_STUB_INVOKE__
        return QList<DCustomActionEntry>() << entry;
    });

    scene->initialize(params);
    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
}

TEST_F(UT_ExtendMenuScene, Create_WithFocusOnly_UsesFocusCombo)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubFileInfoCreate();
    stubInitializeBase();
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    DCustomActionEntry entry;
    entry.actionData.actionName = "ExtAction";
    entry.actionData.actionCommand = "echo test";
    entry.actionData.comboPos[DCustomActionDefines::kSingleFile] = 2;
    entry.actionData.actionSeparator = DCustomActionDefines::kBoth;
    entry.actionFileCombo = DCustomActionDefines::kSingleFile;

    stub.set_lamda(&DCustomActionParser::getActionFiles, [&entry](DCustomActionParser *, bool) {
        __DBG_STUB_INVOKE__
        return QList<DCustomActionEntry>() << entry;
    });

    bool focusComboChecked = false;
    QUrl capturedFocus;
    QList<QUrl> capturedFiles;
    stub.set_lamda(&DCustomActionBuilder::checkFileComboWithFocus,
                   [&focusComboChecked, &capturedFocus, &capturedFiles](const QUrl &focus, const QList<QUrl> &files) {
                       __DBG_STUB_INVOKE__
                       focusComboChecked = true;
                       capturedFocus = focus;
                       capturedFiles = files;
                       return DCustomActionDefines::kSingleFile;
                   });

    stub.set_lamda(&DCustomActionBuilder::matchFileCombo,
                   [](const QList<DCustomActionEntry> &roots, DCustomActionDefines::ComboTypes) {
                       __DBG_STUB_INVOKE__
                       return roots;
                   });

    bool matchActionsCalled = false;
    QList<QUrl> matchSelects;
    stub.set_lamda(&DCustomActionBuilder::matchActions,
                   [&matchActionsCalled, &matchSelects](const QList<QUrl> &selects, QList<DCustomActionEntry> oriActions) {
                       __DBG_STUB_INVOKE__
                       matchActionsCalled = true;
                       matchSelects = selects;
                       return oriActions;
                   });

    stub.set_lamda(ADDR(DCustomActionBuilder, buildAciton),
                   [](DCustomActionBuilder *, const DCustomActionData &, QWidget *parent) -> QAction * {
                       __DBG_STUB_INVOKE__
                       return new QAction("ExtAction", parent);
                   });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));

    // MENU_CHECK_FOCUSONLY: combo is checked with the focus file only
    EXPECT_TRUE(focusComboChecked);
    EXPECT_EQ(capturedFocus, QUrl::fromLocalFile("/tmp/test.txt"));
    EXPECT_EQ(capturedFiles.size(), 1);

    EXPECT_TRUE(matchActionsCalled);
    EXPECT_EQ(matchSelects, QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });

    // extend actions are inserted into the menu on updateState
    EXPECT_NO_FATAL_FAILURE(scene->updateState(&menu));

    QAction *builtAction { nullptr };
    for (auto act : menu.actions()) {
        if (act->text() == "ExtAction")
            builtAction = act;
    }
    ASSERT_NE(builtAction, nullptr);
    // pos recorded from mayComboPostion with the focus combo
    EXPECT_EQ(builtAction->property("act_pos").toInt(), 2);
}

TEST_F(UT_ExtendMenuScene, Create_FocusComboBlankSpace_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubFileInfoCreate();
    stubInitializeBase();
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    DCustomActionEntry entry;
    entry.actionData.actionName = "ExtAction";
    entry.actionFileCombo = DCustomActionDefines::kAllCombo;

    stub.set_lamda(&DCustomActionParser::getActionFiles, [&entry](DCustomActionParser *, bool) {
        __DBG_STUB_INVOKE__
        return QList<DCustomActionEntry>() << entry;
    });
    stub.set_lamda(&DCustomActionBuilder::checkFileComboWithFocus, [](const QUrl &, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        return DCustomActionDefines::kBlankSpace;
    });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    EXPECT_FALSE(scene->create(&menu));
}

TEST_F(UT_ExtendMenuScene, Create_NoMatchedEntries_DelegatesToBase)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;

    stubFileInfoCreate();
    stubInitializeBase();
    stub.set_lamda(VADDR(AbstractMenuScene, create), [](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    DCustomActionEntry entry;
    entry.actionData.actionName = "ExtAction";

    stub.set_lamda(&DCustomActionParser::getActionFiles, [&entry](DCustomActionParser *, bool) {
        __DBG_STUB_INVOKE__
        return QList<DCustomActionEntry>() << entry;
    });
    stub.set_lamda(&DCustomActionBuilder::checkFileComboWithFocus, [](const QUrl &, const QList<QUrl> &) {
        __DBG_STUB_INVOKE__
        return DCustomActionDefines::kSingleFile;
    });
    stub.set_lamda(&DCustomActionBuilder::matchFileCombo, [](const QList<DCustomActionEntry> &, DCustomActionDefines::ComboTypes) {
        __DBG_STUB_INVOKE__
        return QList<DCustomActionEntry>();
    });

    ASSERT_TRUE(scene->initialize(params));
    QMenu menu;
    EXPECT_TRUE(scene->create(&menu));
    EXPECT_TRUE(menu.actions().isEmpty());
}

TEST_F(UT_ExtendMenuScene, Triggered_InvalidAction_ReturnsFalse)
{
    QAction action("test");
    EXPECT_FALSE(scene->triggered(&action));
}

TEST_F(UT_ExtendMenuScene, Triggered_CustomCommandAction_RunsCommand)
{
    bool makeCommandCalled = false;
    bool runCommandCalled = false;

    stub.set_lamda(&DCustomActionBuilder::makeCommand,
                   [&makeCommandCalled](const QString &, DCustomActionDefines::ActionArg,
                                        const QUrl &, const QUrl &, const QList<QUrl> &) -> QPair<QString, QStringList> {
                       __DBG_STUB_INVOKE__
                       makeCommandCalled = true;
                       return qMakePair(QString("true"), QStringList());
                   });
    stub.set_lamda(&UniversalUtils::runCommand,
                   [&runCommandCalled](const QString &, const QStringList &, const QString &) {
                       __DBG_STUB_INVOKE__
                       runCommandCalled = true;
                       return true;
                   });

    QAction action("ext-cmd");
    action.setProperty(DCustomActionDefines::kCustomActionFlag, true);
    action.setProperty(DCustomActionDefines::kCustomActionCommand, "true");
    action.setProperty(DCustomActionDefines::kCustomActionCommandArgFlag,
                       static_cast<int>(DCustomActionDefines::kNoneArg));
    scene->d->extendActions.append(&action);

    EXPECT_TRUE(scene->triggered(&action));
    EXPECT_TRUE(makeCommandCalled);
    EXPECT_TRUE(runCommandCalled);
}

TEST_F(UT_ExtendMenuScene, Triggered_OwnActionWithoutFlag_DelegatesToBase)
{
    QAction action("ext-plain");
    scene->d->extendActions.append(&action);

    EXPECT_FALSE(scene->triggered(&action));
}

TEST_F(UT_ExtendMenuScene, UpdateState_NullParent_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(scene->updateState(nullptr));
}

TEST_F(UT_ExtendMenuScene, UpdateState_ValidParent_CallsBase)
{
    bool baseCalled = false;
    stub.set_lamda(VADDR(AbstractMenuScene, updateState), [&baseCalled](AbstractMenuScene *, QMenu *) {
        __DBG_STUB_INVOKE__
        baseCalled = true;
    });

    QMenu menu;
    scene->updateState(&menu);
    EXPECT_TRUE(baseCalled);
}

TEST_F(UT_ExtendMenuScene, UpdateState_WithExtendActions_InsertsByPosition)
{
    QMenu parent;
    QAction *sysAction = parent.addAction("sys-action");

    QAction *extPosAction = new QAction("ext-pos");
    extPosAction->setProperty("act_pos", 1);
    QAction *extTailAction = new QAction("ext-tail");

    scene->d->extendActions.append(extPosAction);
    scene->d->extendActions.append(extTailAction);
    scene->d->cacheLocateActions[1].append(extPosAction);

    scene->updateState(&parent);

    auto actions = parent.actions();
    ASSERT_GE(actions.size(), 3);
    EXPECT_EQ(actions.first(), extPosAction);
    EXPECT_NE(actions.indexOf(sysAction), -1);
    EXPECT_EQ(actions.last(), extTailAction);
}

TEST_F(UT_ExtendMenuScene, UpdateState_WithParentMenuPath_MergesIntoSubMenu)
{
    QMenu parent;
    QMenu *subMenu = new QMenu(&parent);
    QAction *holderAction = parent.addAction("Holder");
    holderAction->setProperty("actionID", "open-with");
    holderAction->setMenu(subMenu);
    subMenu->addAction("existing-sub");

    QAction *extAction = new QAction("ext-sub");
    extAction->setProperty(DCustomActionDefines::kConfParentMenuPath, QString("open-with"));
    scene->d->extendActions.append(extAction);

    scene->updateState(&parent);

    EXPECT_NE(subMenu->actions().indexOf(extAction), -1);
    EXPECT_TRUE(holderAction->isVisible());
}

TEST_F(UT_ExtendMenuScene, ChildActions_FlattensNestedMenus)
{
    QAction top("top");
    QMenu m1;
    QAction *child1 = m1.addAction("child1");
    QMenu m2;
    QAction *child2 = m2.addAction("child2");
    child1->setMenu(&m2);
    top.setMenu(&m1);

    auto actions = scene->d->childActions(&top);
    EXPECT_EQ(actions.size(), 2);
    EXPECT_EQ(actions.at(0), child1);
    EXPECT_EQ(actions.at(1), child2);

    QAction plain("plain");
    EXPECT_TRUE(scene->d->childActions(&plain).isEmpty());
}

TEST_F(UT_ExtendMenuScene, MayComboPostion_ExactComboSet_ReturnsComboPos)
{
    DCustomActionData data;
    data.comboPos[DCustomActionDefines::kSingleFile] = 3;
    data.actionPosition = 7;

    EXPECT_EQ(scene->d->mayComboPostion(data, DCustomActionDefines::kSingleFile), 3);
}

TEST_F(UT_ExtendMenuScene, MayComboPostion_MultiFilesUnset_FallsBackToFileAndDir)
{
    DCustomActionData data;
    data.actionPosition = 7;
    data.comboPos[DCustomActionDefines::kFileAndDir] = 4;

    // kMultiFiles has no own pos, fallback to kFileAndDir pos
    EXPECT_EQ(scene->d->mayComboPostion(data, DCustomActionDefines::kMultiFiles), 4);
}

TEST_F(UT_ExtendMenuScene, MayComboPostion_MultiDirsSet_NoFallback)
{
    DCustomActionData data;
    data.actionPosition = 7;
    data.comboPos[DCustomActionDefines::kMultiDirs] = 6;

    EXPECT_EQ(scene->d->mayComboPostion(data, DCustomActionDefines::kMultiDirs), 6);
}

TEST_F(UT_ExtendMenuScene, MayComboPostion_OtherComboUnset_ReturnsDefaultPos)
{
    DCustomActionData data;
    data.actionPosition = 9;

    EXPECT_EQ(scene->d->mayComboPostion(data, DCustomActionDefines::kSingleDir), 9);
}

TEST_F(UT_ExtendMenuScene, GetSubMenus_CreatesNestedMenus)
{
    QMenu menu;
    QAction *topAction = menu.addAction("A");
    topAction->setProperty("actionID", "open-with");
    QMenu *sub = new QMenu(&menu);
    topAction->setMenu(sub);
    QAction *nestedAction = sub->addAction("B");
    nestedAction->setProperty("actionID", "sub1");
    QMenu *subsub = new QMenu(sub);
    nestedAction->setMenu(subsub);
    // plain action without id, must be skipped
    sub->addAction("C");

    QMap<QString, QMenu *> subMenus;
    scene->d->getSubMenus(&menu, "", subMenus);

    EXPECT_EQ(subMenus.size(), 2);
    EXPECT_EQ(subMenus.value("open-with"), sub);
    EXPECT_EQ(subMenus.value("open-with/sub1"), subsub);
}

TEST_F(UT_ExtendMenuScene, InsertIntoExistedSubActions_NoParentPath_ReturnsFalse)
{
    QMap<QString, QList<QAction *>> extSubActMap;
    extSubActMap.insert("open-with", {});

    QAction action("no-parent");
    EXPECT_FALSE(scene->d->insertIntoExistedSubActions(&action, extSubActMap));
}

TEST_F(UT_ExtendMenuScene, InsertIntoExistedSubActions_ParentNotFound_ReturnsFalse)
{
    QMap<QString, QList<QAction *>> extSubActMap;
    extSubActMap.insert("open-with", {});

    QAction action("other");
    action.setProperty(DCustomActionDefines::kConfParentMenuPath, QString("not-exist"));
    EXPECT_FALSE(scene->d->insertIntoExistedSubActions(&action, extSubActMap));
}

TEST_F(UT_ExtendMenuScene, InsertIntoExistedSubActions_NoPos_AppendsToEnd)
{
    QMap<QString, QList<QAction *>> extSubActMap;
    QAction *existing = new QAction("existing");
    extSubActMap.insert("open-with", { existing });

    QAction *action = new QAction("ext");
    action->setProperty(DCustomActionDefines::kConfParentMenuPath, QString("open-with"));

    EXPECT_TRUE(scene->d->insertIntoExistedSubActions(action, extSubActMap));
    auto list = extSubActMap.value("open-with");
    ASSERT_EQ(list.size(), 2);
    EXPECT_EQ(list.last(), action);
}

TEST_F(UT_ExtendMenuScene, InsertIntoExistedSubActions_WithPos_InsertsInOrder)
{
    QMap<QString, QList<QAction *>> extSubActMap;
    QAction *first = new QAction("first");
    first->setProperty("act_pos", 1);
    QAction *third = new QAction("third");
    third->setProperty("act_pos", 3);
    extSubActMap.insert("open-with", { first, third });

    QAction *second = new QAction("second");
    second->setProperty(DCustomActionDefines::kConfParentMenuPath, QString("open-with"));
    second->setProperty("act_pos", 2);

    EXPECT_TRUE(scene->d->insertIntoExistedSubActions(second, extSubActMap));
    auto list = extSubActMap.value("open-with");
    ASSERT_EQ(list.size(), 3);
    EXPECT_EQ(list.at(1), second);
}

TEST_F(UT_ExtendMenuScene, InsertIntoExistedSubActions_WithSeparator_InsertsSeparators)
{
    QMap<QString, QList<QAction *>> extSubActMap;
    extSubActMap.insert("open-with", {});

    QAction *action = new QAction("ext-sep");
    action->setProperty(DCustomActionDefines::kConfParentMenuPath, QString("open-with"));
    scene->d->cacheActionsSeparator.insert(action, DCustomActionDefines::kBoth);

    EXPECT_TRUE(scene->d->insertIntoExistedSubActions(action, extSubActMap));
    auto list = extSubActMap.value("open-with");
    // separator + action + separator
    ASSERT_EQ(list.size(), 3);
    EXPECT_TRUE(list.at(0)->isSeparator());
    EXPECT_EQ(list.at(1), action);
    EXPECT_TRUE(list.at(2)->isSeparator());
    // separator cache must be consumed
    EXPECT_FALSE(scene->d->cacheActionsSeparator.contains(action));
}

TEST_F(UT_ExtendMenuScene, MergeSubActions_CombinesActions)
{
    QMenu parent;
    QMenu *subMenu = new QMenu(&parent);
    QAction *holderAction = parent.addAction("Holder");
    holderAction->setProperty("actionID", "open-with");
    holderAction->setMenu(subMenu);
    QAction *existingSub = subMenu->addAction("existing-sub");
    holderAction->setVisible(false);

    QAction *extAction = new QAction("ext-sub");
    QMap<QString, QList<QAction *>> extSubActMap;
    extSubActMap.insert("open-with", { extAction });
    QMap<QString, QMenu *> subMenus;
    subMenus.insert("open-with", subMenu);

    scene->d->mergeSubActions(extSubActMap, subMenus);

    auto list = subMenu->actions();
    ASSERT_EQ(list.size(), 2);
    EXPECT_EQ(list.first(), extAction);
    EXPECT_EQ(list.last(), existingSub);
    EXPECT_TRUE(holderAction->isVisible());
}

TEST_F(UT_ExtendMenuScene, MergeSubActions_EmptyExtList_SkipsMenu)
{
    QMenu parent;
    QMenu *subMenu = new QMenu(&parent);
    QAction *holderAction = parent.addAction("Holder");
    holderAction->setProperty("actionID", "open-with");
    holderAction->setMenu(subMenu);
    QAction *existingSub = subMenu->addAction("existing-sub");
    holderAction->setVisible(false);

    QMap<QString, QList<QAction *>> extSubActMap;
    extSubActMap.insert("open-with", {});
    QMap<QString, QMenu *> subMenus;
    subMenus.insert("open-with", subMenu);

    scene->d->mergeSubActions(extSubActMap, subMenus);

    EXPECT_EQ(subMenu->actions().size(), 1);
    EXPECT_FALSE(holderAction->isVisible());
}

TEST_F(UT_ExtendMenuScene, UpdateState_PositionGapFilledByOriginalActions)
{
    // cacheLocateActions 只记录位置 2，位置 1 由原菜单 action 占位，
    // 触发 sortFunc 的占位分支（执行计谓词 lambda）。
    QMenu parent;
    QAction *sysAction = parent.addAction("sys-action");
    Q_UNUSED(sysAction);

    QAction *extPosAction = new QAction("ext-pos");
    scene->d->extendActions.append(extPosAction);
    scene->d->cacheLocateActions[2].append(extPosAction);

    EXPECT_NO_FATAL_FAILURE(scene->updateState(&parent));
    EXPECT_NE(parent.actions().indexOf(extPosAction), -1);
    EXPECT_EQ(parent.actions().first(), sysAction);
    EXPECT_EQ(parent.actions().last(), extPosAction);
}
