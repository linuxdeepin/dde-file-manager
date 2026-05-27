// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "menus/opticalmenuscene.h"
#include "menus/opticalmenuscene_p.h"
#include "utils/opticalhelper.h"
#include "mastered/masteredmediafileinfo.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/clipboard.h>

#include <QMenu>
#include <QAction>
#include <QUrl>
#include <QVariantHash>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_optical;

class TestOpticalMenuScene : public testing::Test
{
public:
    void SetUp() override
    {
        scene = new OpticalMenuScene();
        setupDefaultMocks();
    }

    void TearDown() override
    {
        delete scene;
        stub.clear();
    }

protected:
    void setupDefaultMocks()
    {
        // Mock OpticalHelper functions
        stub.set_lamda(&OpticalHelper::burnIsOnDisc, [this](const QUrl &url) {
            __DBG_STUB_INVOKE__
            return mockIsOnDisc;
        });

        stub.set_lamda(&OpticalHelper::isBurnEnabled, []() {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(&OpticalHelper::burnDestDevice, [this](const QUrl &url) {
            __DBG_STUB_INVOKE__
            return mockDevice;
        });

        stub.set_lamda(&OpticalHelper::discRoot, [this](const QString &dev) {
            __DBG_STUB_INVOKE__
            return QUrl("burn:///dev/sr0/disc_files/");
        });

        // Mock UniversalUtils
        stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) {
            __DBG_STUB_INVOKE__
            return url1 == url2;
        });

        // Mock ClipBoard
        stub.set_lamda(&ClipBoard::instance, []() {
            __DBG_STUB_INVOKE__
            static ClipBoard clipboard;
            return &clipboard;
        });

        stub.set_lamda(&ClipBoard::clipboardAction, []() {
            __DBG_STUB_INVOKE__
            return ClipBoard::kCopyAction;
        });

        stub.set_lamda(&ClipBoard::clipboardFileUrlList, []() {
            __DBG_STUB_INVOKE__
            return QList<QUrl> { QUrl::fromLocalFile("/tmp/test.txt") };
        });
    }

    OpticalMenuScene *scene = nullptr;
    stub_ext::StubExt stub;

    // Mock data
    QString mockDevice = "/dev/sr0";
    bool mockIsOnDisc = false;
    QSharedPointer<FileInfo> mockFileInfo;
};

TEST_F(TestOpticalMenuScene, Constructor_InitializesCorrectly)
{
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), OpticalMenuSceneCreator::name());
}

TEST_F(TestOpticalMenuScene, Name_ReturnsCorrectName)
{
    QString name = scene->name();
    EXPECT_EQ(name, OpticalMenuSceneCreator::name());
}

TEST_F(TestOpticalMenuScene, Initialize_ValidParams_ReturnsTrue)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/disc_files/");
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("burn:///dev/sr0/disc_files/test.txt") });

    // Mock MasteredMediaFileInfo
    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "/tmp/test.txt";
        return props;
    });

    // Mock menu scene creation
    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [](const QString &name) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    bool result = scene->initialize(params);
    EXPECT_TRUE(result);
}

TEST_F(TestOpticalMenuScene, Initialize_EmptySelectFiles_ReturnsFalse)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/disc_files/");
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    // Mock MasteredMediaFileInfo for blank disc
    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "";   // Empty backer indicates blank disc
        return props;
    });

    bool result = scene->initialize(params);
    EXPECT_FALSE(result);
}

TEST_F(TestOpticalMenuScene, Initialize_BlankDisc_SetsBlankDiscFlag)
{
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/disc_files/");
    params[MenuParamKey::kIsEmptyArea] = true;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    // Mock MasteredMediaFileInfo for blank disc
    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "";   // Empty backer indicates blank disc
        return props;
    });

    // Mock menu scene creation
    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [](const QString &name) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    bool result = scene->initialize(params);
    EXPECT_TRUE(result);
}

TEST_F(TestOpticalMenuScene, UpdateState_EmptyArea_FiltersActionsCorrectly)
{
    // Initialize scene first
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/disc_files/");
    params[MenuParamKey::kIsEmptyArea] = true;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "";
        return props;
    });

    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [](const QString &name) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    scene->initialize(params);

    // Create test menu and actions
    QMenu menu;
    QAction *pasteAction = new QAction("Paste", &menu);
    pasteAction->setProperty(ActionPropertyKey::kActionID, "paste");
    menu.addAction(pasteAction);

    QAction *refreshAction = new QAction("Refresh", &menu);
    refreshAction->setProperty(ActionPropertyKey::kActionID, "refresh");
    menu.addAction(refreshAction);

    QAction *deleteAction = new QAction("Delete", &menu);
    deleteAction->setProperty(ActionPropertyKey::kActionID, "delete");
    menu.addAction(deleteAction);

    // Mock findSceneName to return valid scene names
    stub.set_lamda(ADDR(OpticalMenuScenePrivate, findSceneName), [](OpticalMenuScenePrivate *, QAction *act) {
        __DBG_STUB_INVOKE__
        return QString("WorkspaceMenu");
    });

    scene->updateState(&menu);

    // Verify paste action is enabled and visible
    EXPECT_TRUE(pasteAction->isVisible());
    EXPECT_TRUE(pasteAction->isEnabled());

    // Verify refresh action is visible
    EXPECT_TRUE(refreshAction->isVisible());

    // Verify delete action is not visible (not in empty area white list)
    EXPECT_FALSE(deleteAction->isVisible());
}

TEST_F(TestOpticalMenuScene, UpdateState_NormalArea_FiltersActionsCorrectly)
{
    // Initialize scene first
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/disc_files/");
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("burn:///dev/sr0/disc_files/test.txt") });

    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "/tmp/test.txt";
        return props;
    });

    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [](const QString &name) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    scene->initialize(params);

    // Create test menu and actions
    QMenu menu;
    QAction *openAction = new QAction("Open", &menu);
    openAction->setProperty(ActionPropertyKey::kActionID, "open");
    menu.addAction(openAction);

    QAction *deleteAction = new QAction("Delete", &menu);
    deleteAction->setProperty(ActionPropertyKey::kActionID, "delete");
    menu.addAction(deleteAction);

    QAction *cutAction = new QAction("Cut", &menu);
    cutAction->setProperty(ActionPropertyKey::kActionID, "cut");
    menu.addAction(cutAction);

    // Mock findSceneName to return valid scene names
    stub.set_lamda(ADDR(OpticalMenuScenePrivate, findSceneName), [](OpticalMenuScenePrivate *, QAction *act) {
        __DBG_STUB_INVOKE__
        return QString("WorkspaceMenu");
    });

    mockIsOnDisc = true;   // File is on disc

    scene->updateState(&menu);

    // Verify open action is visible
    EXPECT_TRUE(openAction->isVisible());

    // Verify delete action is not visible for files on disc
    EXPECT_FALSE(deleteAction->isVisible());

    // Verify cut action is not visible (not in normal white list)
    EXPECT_FALSE(cutAction->isVisible());
}

TEST_F(TestOpticalMenuScene, UpdateState_FileNotOnDisc_ShowsSendToAction)
{
    // Initialize scene first
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/staging_files/");
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("burn:///dev/sr0/staging_files/test.txt") });

    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "/tmp/test.txt";
        return props;
    });

    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [](const QString &name) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    scene->initialize(params);

    // Create test menu and actions
    QMenu menu;
    QAction *sendToAction = new QAction("Send To", &menu);
    sendToAction->setProperty(ActionPropertyKey::kActionID, "send-to");
    menu.addAction(sendToAction);

    // Mock findSceneName to return valid scene names
    stub.set_lamda(ADDR(OpticalMenuScenePrivate, findSceneName), [](OpticalMenuScenePrivate *, QAction *act) {
        __DBG_STUB_INVOKE__
        return QString("SendToMenu");
    });

    mockIsOnDisc = false;   // File is not on disc (staging area)

    scene->updateState(&menu);

    // Send-to should be hidden for files not on disc
    EXPECT_FALSE(sendToAction->isVisible());
}

TEST_F(TestOpticalMenuScene, UpdateState_DirectoryNotOnDisc_HidesSpecialActions)
{
    // Initialize scene first
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/staging_files/");
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("burn:///dev/sr0/staging_files/folder") });

    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "/tmp/folder";
        return props;
    });

    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [](const QString &name) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    scene->initialize(params);

    // Mock file info to indicate it's a directory
    mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl("burn:///dev/sr0/staging_files/folder")));
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType type) {
        __DBG_STUB_INVOKE__
        return type == OptInfoType::kIsDir;
    });

    // Create test menu and actions
    QMenu menu;
    QAction *openAsAdminAction = new QAction("Open as Administrator", &menu);
    openAsAdminAction->setProperty(ActionPropertyKey::kActionID, "open-as-administrator");
    menu.addAction(openAsAdminAction);

    QAction *openInTerminalAction = new QAction("Open in Terminal", &menu);
    openInTerminalAction->setProperty(ActionPropertyKey::kActionID, "open-in-terminal");
    menu.addAction(openInTerminalAction);

    QAction *addBookmarkAction = new QAction("Add Bookmark", &menu);
    addBookmarkAction->setProperty(ActionPropertyKey::kActionID, "add-bookmark");
    menu.addAction(addBookmarkAction);

    // Mock findSceneName to return valid scene names
    stub.set_lamda(ADDR(OpticalMenuScenePrivate, findSceneName), [](OpticalMenuScenePrivate *, QAction *act) {
        __DBG_STUB_INVOKE__
        return QString("WorkspaceMenu");
    });

    mockIsOnDisc = false;   // Directory is not on disc (staging area)

    scene->updateState(&menu);

    // These actions should be hidden for directories not on disc
    EXPECT_FALSE(openAsAdminAction->isVisible());
    EXPECT_FALSE(openInTerminalAction->isVisible());
    EXPECT_FALSE(addBookmarkAction->isVisible());
}

TEST_F(TestOpticalMenuScene, UpdateState_BlankDiscEmptyArea_HidesSpecialActions)
{
    // Initialize scene as blank disc
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/disc_files/");
    params[MenuParamKey::kIsEmptyArea] = true;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "";   // Empty backer indicates blank disc
        return props;
    });

    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [](const QString &name) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    scene->initialize(params);

    // Create test menu and actions
    QMenu menu;
    QAction *openAsAdminAction = new QAction("Open as Administrator", &menu);
    openAsAdminAction->setProperty(ActionPropertyKey::kActionID, "open-as-administrator");
    menu.addAction(openAsAdminAction);

    QAction *openInTerminalAction = new QAction("Open in Terminal", &menu);
    openInTerminalAction->setProperty(ActionPropertyKey::kActionID, "open-in-terminal");
    menu.addAction(openInTerminalAction);

    // Mock findSceneName to return valid scene names
    stub.set_lamda(ADDR(OpticalMenuScenePrivate, findSceneName), [](OpticalMenuScenePrivate *, QAction *act) {
        __DBG_STUB_INVOKE__
        return QString("WorkspaceMenu");
    });

    scene->updateState(&menu);

    // These actions should be hidden for blank disc empty area
    EXPECT_FALSE(openAsAdminAction->isVisible());
    EXPECT_FALSE(openInTerminalAction->isVisible());
}

TEST_F(TestOpticalMenuScene, UpdateState_InvalidScene_HidesActions)
{
    // Initialize scene first
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/disc_files/");
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("burn:///dev/sr0/disc_files/test.txt") });

    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "/tmp/test.txt";
        return props;
    });

    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [](const QString &name) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    scene->initialize(params);

    // Create test menu and actions
    QMenu menu;
    QAction *testAction = new QAction("Test Action", &menu);
    testAction->setProperty(ActionPropertyKey::kActionID, "test-action");
    menu.addAction(testAction);

    // Mock findSceneName to return invalid scene name
    stub.set_lamda(ADDR(OpticalMenuScenePrivate, findSceneName), [](OpticalMenuScenePrivate *, QAction *act) {
        __DBG_STUB_INVOKE__
        return QString("InvalidScene");
    });

    scene->updateState(&menu);

    // Action should be hidden due to invalid scene
    EXPECT_FALSE(testAction->isVisible());
}

TEST_F(TestOpticalMenuScene, UpdateState_SeparatorActions_AlwaysVisible)
{
    // Initialize scene first
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/disc_files/");
    params[MenuParamKey::kIsEmptyArea] = false;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl("burn:///dev/sr0/disc_files/test.txt") });

    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "/tmp/test.txt";
        return props;
    });

    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [](const QString &name) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    scene->initialize(params);

    // Create test menu with separator
    QMenu menu;
    QAction *separatorAction = menu.addSeparator();

    scene->updateState(&menu);

    // Separator should always be visible
    EXPECT_TRUE(separatorAction->isVisible());
}

// Test OpticalMenuScenePrivate methods
TEST_F(TestOpticalMenuScene, FindSceneName_ValidAction_ReturnsSceneName)
{
    // Create a mock action with scene
    QAction action;

    // Create a mock scene
    class MockScene : public AbstractMenuScene
    {
    public:
        QString name() const override { return "TestScene"; }
        bool initialize(const QVariantHash &) override { return true; }
    };

    MockScene mockScene;

    // Mock the scene method
    stub.set_lamda(VADDR(OpticalMenuScene, scene), [&](AbstractMenuScene *, QAction *) {
        __DBG_STUB_INVOKE__
        return &mockScene;
    });

    // Access private member through scene
    OpticalMenuScenePrivate *d = scene->d.get();
    QString sceneName = d->findSceneName(&action);

    EXPECT_EQ(sceneName, "TestScene");
}

TEST_F(TestOpticalMenuScene, FindSceneName_NoScene_ReturnsEmptyString)
{
    QAction action;

    // Mock scene method to return nullptr
    stub.set_lamda(VADDR(OpticalMenuScene, scene), [](AbstractMenuScene *, QAction *) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    OpticalMenuScenePrivate *d = scene->d.get();
    QString sceneName = d->findSceneName(&action);

    EXPECT_TRUE(sceneName.isEmpty());
}

TEST_F(TestOpticalMenuScene, EnablePaste_BurnDisabled_ReturnsFalse)
{
    stub.set_lamda(&OpticalHelper::isBurnEnabled, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Initialize scene first
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/disc_files/");
    params[MenuParamKey::kIsEmptyArea] = true;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "";
        return props;
    });

    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [](const QString &name) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    scene->initialize(params);

    OpticalMenuScenePrivate *d = scene->d.get();
    bool result = d->enablePaste();

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalMenuScene, EnablePaste_NotDiscRoot_ReturnsFalse)
{
    // Initialize scene with non-root directory
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/disc_files/subfolder/");
    params[MenuParamKey::kIsEmptyArea] = true;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "";
        return props;
    });

    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [](const QString &name) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    // Mock UniversalUtils::urlEquals to return false (not root)
    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) {
        __DBG_STUB_INVOKE__
        return false;
    });

    scene->initialize(params);

    OpticalMenuScenePrivate *d = scene->d.get();
    bool result = d->enablePaste();

    EXPECT_FALSE(result);
}

TEST_F(TestOpticalMenuScene, EnablePaste_ValidConditions_ReturnsTrue)
{
    // Initialize scene with root directory
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl("burn:///dev/sr0/disc_files/");
    params[MenuParamKey::kIsEmptyArea] = true;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    stub.set_lamda(VADDR(MasteredMediaFileInfo, extraProperties), []() {
        __DBG_STUB_INVOKE__
        QVariantHash props;
        props["mm_backer"] = "";
        return props;
    });

    stub.set_lamda(&dfmplugin_menu_util::menuSceneCreateScene, [](const QString &name) {
        __DBG_STUB_INVOKE__
        return static_cast<AbstractMenuScene *>(nullptr);
    });

    // Mock UniversalUtils::urlEquals to return true (is root)
    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) {
        __DBG_STUB_INVOKE__
        return true;
    });

    scene->initialize(params);

    OpticalMenuScenePrivate *d = scene->d.get();
    bool result = d->enablePaste();

    EXPECT_TRUE(result);
}
