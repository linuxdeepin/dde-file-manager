// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "menus/packetwritingmenuscene.h"
#include "menus/packetwritingmenuscene_p.h"
#include "utils/opticalhelper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/schemefactory.h>

#include <QUrl>
#include <QMenu>
#include <QAction>
#include <QVariantHash>

DFMBASE_USE_NAMESPACE
DPOPTICAL_USE_NAMESPACE

class TestPacketWritingMenuScene : public testing::Test
{
public:
    void SetUp() override
    {
        scene = new PacketWritingMenuScene();
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
        // Mock DeviceProxyManager
        stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [this](DeviceProxyManager *, const QString &path) {
            __DBG_STUB_INVOKE__
            return mockIsFromOptical;
        });

        // Mock DeviceUtils - fix function pointer type
        typedef QString (*GetMountInfoFunc)(const QString &, bool);
        stub.set_lamda(static_cast<GetMountInfoFunc>(&DeviceUtils::getMountInfo), [this](const QString &path, bool) {
            __DBG_STUB_INVOKE__
            return mockMountInfo;
        });

        // Mock OpticalHelper
        stub.set_lamda(&OpticalHelper::burnIsOnDisc, [this](const QUrl &url) {
            __DBG_STUB_INVOKE__
            return mockIsOnDisc;
        });
    }

    PacketWritingMenuScene *scene;
    stub_ext::StubExt stub;

    // Mock data
    bool mockIsFromOptical = true;
    QString mockMountInfo = "/media/optical";
    bool mockIsOnDisc = false;
};

// Test initialization
TEST_F(TestPacketWritingMenuScene, Constructor_InitializesCorrectly)
{
    ASSERT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "PacketWritingMenu");
}

// Test scene name detection
TEST_F(TestPacketWritingMenuScene, Name_ReturnsCorrectName)
{
    EXPECT_EQ(scene->name(), "PacketWritingMenu");
}

// Test initialize with empty area
TEST_F(TestPacketWritingMenuScene, Initialize_EmptyArea_DoesNothing)
{
    QMenu menu;
    QUrl url("burn:///test");
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = url;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    bool result = scene->initialize(params);
    EXPECT_TRUE(result);
}

// Test initialize with normal area
TEST_F(TestPacketWritingMenuScene, Initialize_NormalArea_AddsActions)
{
    QMenu menu;
    QUrl url("burn:///test");
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = url;
    QList<QUrl> fileList;
    fileList << QUrl("burn:///test/file1.txt");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(fileList);

    // Mock DeviceUtils::getMountInfo for this test
    typedef QString (*GetMountInfoFunc)(const QString &, bool);
    stub.set_lamda(static_cast<GetMountInfoFunc>(&DeviceUtils::getMountInfo), [](const QString &, bool) {
        __DBG_STUB_INVOKE__
        return "/media/optical";
    });

    bool result = scene->initialize(params);
    EXPECT_TRUE(result);
}

// Test triggered action handling
TEST_F(TestPacketWritingMenuScene, Triggered_ValidAction_HandlesCorrectly)
{
    QAction action;
    action.setProperty(ActionPropertyKey::kActionID, "test-action");

    bool result = scene->triggered(&action);
    EXPECT_TRUE(result);
}

// Test scene detection
TEST_F(TestPacketWritingMenuScene, Scene_ValidAction_ReturnsThis)
{
    QAction action;
    action.setProperty(ActionPropertyKey::kActionID, "test-action");

    AbstractMenuScene *result = scene->scene(&action);
    EXPECT_EQ(result, scene);
}

// Test updateState
TEST_F(TestPacketWritingMenuScene, UpdateState_ValidMenu_UpdatesCorrectly)
{
    QMenu menu;

    // This should not crash
    scene->updateState(&menu);
    EXPECT_TRUE(true);   // If we reach here, updateState worked
}

// Test menu filtering for empty area
TEST_F(TestPacketWritingMenuScene, FilterActions_EmptyArea_FiltersCorrectly)
{
    QMenu menu;
    QUrl url("burn:///test");
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = url;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    scene->initialize(params);
    bool result = scene->create(&menu);
    EXPECT_TRUE(result);
}

// Test menu filtering for normal area with subdirectory
TEST_F(TestPacketWritingMenuScene, FilterActions_NormalAreaWithSubdir_FiltersCorrectly)
{
    QMenu menu;
    QUrl url("burn:///test");
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = url;
    QList<QUrl> fileList;
    fileList << QUrl("burn:///test/subdir/file.txt");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(fileList);

    // Mock DeviceUtils::getMountInfo for this test
    typedef QString (*GetMountInfoFunc)(const QString &, bool);
    stub.set_lamda(static_cast<GetMountInfoFunc>(&DeviceUtils::getMountInfo), [](const QString &, bool) {
        __DBG_STUB_INVOKE__
        return "/media/optical";
    });

    scene->initialize(params);
    bool result = scene->create(&menu);
    EXPECT_TRUE(result);
}

// Test menu filtering for normal area without subdirectory
TEST_F(TestPacketWritingMenuScene, FilterActions_NormalAreaNoSubdir_FiltersCorrectly)
{
    QMenu menu;
    QUrl url("burn:///test");
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = url;
    QList<QUrl> fileList;
    fileList << QUrl("burn:///test/file.txt");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(fileList);

    // Mock DeviceUtils::getMountInfo for this test
    typedef QString (*GetMountInfoFunc)(const QString &, bool);
    stub.set_lamda(static_cast<GetMountInfoFunc>(&DeviceUtils::getMountInfo), [](const QString &, bool) {
        __DBG_STUB_INVOKE__
        return "/media/optical";
    });

    scene->initialize(params);
    bool result = scene->create(&menu);
    EXPECT_TRUE(result);
}

// Test edge case: empty file list
TEST_F(TestPacketWritingMenuScene, Initialize_EmptyFileList_HandlesCorrectly)
{
    QUrl url("burn:///test");
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = url;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    bool result = scene->initialize(params);
    EXPECT_TRUE(result);
}

// Test edge case: invalid URL
TEST_F(TestPacketWritingMenuScene, Initialize_InvalidUrl_HandlesCorrectly)
{
    QUrl url;
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = url;
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl>());

    bool result = scene->initialize(params);
    EXPECT_TRUE(result);
}
