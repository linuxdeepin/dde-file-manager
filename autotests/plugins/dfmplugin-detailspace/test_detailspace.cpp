// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>

// 包含待测试的类
#include "detailspace.h"
#include "utils/detailspacehelper.h"
#include "events/detailspaceeventreceiver.h"
#include "dfmplugin_detailspace_global.h"

// 包含依赖的头文件
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

DPDETAILSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

/**
 * @brief DetailSpace类单元测试
 *
 * 测试范围：
 * 1. 插件初始化流程
 * 2. 插件启动流程
 * 3. 窗口关闭事件处理
 * 4. 事件监听器注册
 * 5. 生命周期管理
 */
class DetailSpaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create plugin instance
        plugin = new DetailSpace();
        ASSERT_NE(plugin, nullptr);

        // Setup test data
        testWindowId = 12345;
    }

    void TearDown() override
    {
        delete plugin;
        stub.clear();
    }

    stub_ext::StubExt stub;
    DetailSpace *plugin { nullptr };
    quint64 testWindowId;
};

/**
 * @brief 测试插件初始化方法
 * 验证initialize方法正确连接事件和服务
 */
TEST_F(DetailSpaceTest, Initialize_NormalCall_ConnectsEventsAndServices)
{
    bool eventReceiverConnected = false;

    // Mock DetailSpaceEventReceiver::connectService
    stub.set_lamda(&DetailSpaceEventReceiver::connectService, [&eventReceiverConnected]() {
        __DBG_STUB_INVOKE__
        eventReceiverConnected = true;
    });

    // Execute initialize
    EXPECT_NO_FATAL_FAILURE(plugin->initialize());

    // Verify connections were made
    EXPECT_TRUE(eventReceiverConnected);
}

/**
 * @brief 测试插件启动方法
 * 验证start方法返回正确值
 */
TEST_F(DetailSpaceTest, Start_NormalCall_ReturnsTrue)
{
    bool result = plugin->start();
    EXPECT_TRUE(result);
}

/**
 * @brief 测试窗口关闭事件处理
 * 验证onWindowClosed方法正确调用DetailSpaceHelper
 */
TEST_F(DetailSpaceTest, OnWindowClosed_ValidWindowId_CallsDetailSpaceHelperRemove)
{
    bool removeDetailSpaceCalled = false;
    quint64 receivedWindowId = 0;

    // Mock DetailSpaceHelper::removeDetailSpace
    stub.set_lamda(&DetailSpaceHelper::removeDetailSpace, [&removeDetailSpaceCalled, &receivedWindowId](quint64 windowId) {
        __DBG_STUB_INVOKE__
        removeDetailSpaceCalled = true;
        receivedWindowId = windowId;
    });

    // Execute onWindowClosed
    EXPECT_NO_FATAL_FAILURE(plugin->onWindowClosed(testWindowId));

    // Verify correct method was called with correct parameter
    EXPECT_TRUE(removeDetailSpaceCalled);
    EXPECT_EQ(receivedWindowId, testWindowId);
}

/**
 * @brief 测试窗口关闭事件处理 - 零窗口ID
 * 验证零窗口ID的处理
 */
TEST_F(DetailSpaceTest, OnWindowClosed_ZeroWindowId_CallsDetailSpaceHelperRemove)
{
    bool removeDetailSpaceCalled = false;
    quint64 receivedWindowId = 1; // Set to non-zero initially

    // Mock DetailSpaceHelper::removeDetailSpace
    stub.set_lamda(&DetailSpaceHelper::removeDetailSpace, [&removeDetailSpaceCalled, &receivedWindowId](quint64 windowId) {
        __DBG_STUB_INVOKE__
        removeDetailSpaceCalled = true;
        receivedWindowId = windowId;
    });

    // Execute onWindowClosed with zero window ID
    EXPECT_NO_FATAL_FAILURE(plugin->onWindowClosed(0));

    // Verify method was called even with zero ID
    EXPECT_TRUE(removeDetailSpaceCalled);
    EXPECT_EQ(receivedWindowId, 0);
}

/**
 * @brief 测试插件对象为Q_OBJECT
 * 验证插件正确继承了QObject功能
 */
TEST_F(DetailSpaceTest, QObjectFeatures_MetaObject_HasCorrectClassName)
{
    const QMetaObject *metaObject = plugin->metaObject();
    EXPECT_NE(metaObject, nullptr);

    QString className = metaObject->className();
    EXPECT_TRUE(className.contains("DetailSpace"));
}
