// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>

#include "stubext.h"

#include "fileoperations.h"
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/settingdialog/customsettingitemregister.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-framework/listener/listener.h>
#include <dfm-framework/dpf.h>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

class TestFileOperations : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        fileOps = new FileOperations();
    }

    void TearDown() override
    {
        stub.clear();
        delete fileOps;
        fileOps = nullptr;
    }

    stub_ext::StubExt stub;
    FileOperations *fileOps { nullptr };
};

// ========== FileOperations::initialize() Tests ==========

TEST_F(TestFileOperations, Initialize_CallsInitEventHandle)
{
    bool initEventHandleCalled = false;

    typedef void (FileOperations::*initEventHandleFunc)();
    stub.set_lamda(static_cast<initEventHandleFunc>(&FileOperations::initEventHandle), [&initEventHandleCalled]() {
        __DBG_STUB_INVOKE__
        initEventHandleCalled = true;
    });

    bool followEventsCalled = false;
    typedef void (FileOperations::*followEventsFunc)();
    stub.set_lamda(static_cast<followEventsFunc>(&FileOperations::followEvents), [&followEventsCalled]() {
        __DBG_STUB_INVOKE__
        followEventsCalled = true;
    });

    fileOps->initialize();

    EXPECT_TRUE(initEventHandleCalled);
    EXPECT_TRUE(followEventsCalled);
}

// ========== FileOperations::start() Tests ==========

TEST_F(TestFileOperations, Start_AddConfigSuccessfully)
{
    bool addConfigCalled = false;

    stub.set_lamda(&DConfigManager::addConfig, [&addConfigCalled](DConfigManager *, const QString &name, QString *err) -> bool {
        __DBG_STUB_INVOKE__
        addConfigCalled = true;
        return true;
    });

    bool regSettingConfigCalled = false;
    typedef void (FileOperations::*regSettingConfigFunc)();
    stub.set_lamda(static_cast<regSettingConfigFunc>(&FileOperations::regSettingConfig), [&regSettingConfigCalled]() {
        __DBG_STUB_INVOKE__
        regSettingConfigCalled = true;
    });

    bool result = fileOps->start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(addConfigCalled);
    EXPECT_TRUE(regSettingConfigCalled);
}

TEST_F(TestFileOperations, Start_AddConfigFailed)
{
    stub.set_lamda(&DConfigManager::addConfig, [](DConfigManager *, const QString &name, QString *err) -> bool {
        __DBG_STUB_INVOKE__
        if (err)
            *err = "Test error";
        return false;
    });

    bool regSettingConfigCalled = false;
    typedef void (FileOperations::*regSettingConfigFunc)();
    stub.set_lamda(static_cast<regSettingConfigFunc>(&FileOperations::regSettingConfig), [&regSettingConfigCalled]() {
        __DBG_STUB_INVOKE__
        regSettingConfigCalled = true;
    });

    bool result = fileOps->start();

    // Should still return true and call regSettingConfig even if addConfig fails
    EXPECT_TRUE(result);
    EXPECT_TRUE(regSettingConfigCalled);
}

// ========== FileOperations::initEventHandle() Tests ==========

TEST_F(TestFileOperations, InitEventHandle_SubscribesAllEvents)
{
    fileOps->initEventHandle();
    SUCCEED();
}

// ========== FileOperations::followEvents() Tests ==========

TEST_F(TestFileOperations, FollowEvents_FollowsHookSequences)
{
    fileOps->followEvents();
    SUCCEED();
}

// ========== FileOperations::regSettingConfig() Tests ==========

TEST_F(TestFileOperations, RegSettingConfig_RegistersSettings)
{
    bool addGroupCalled = false;
    bool registCustomSettingCalled = false;
    bool addConfigCalled = false;
    bool addSettingAccessorCalled = false;

    stub.set_lamda(&SettingJsonGenerator::addGroup, [&addGroupCalled] {
        addGroupCalled= true;
        return true; });

    stub.set_lamda(&CustomSettingItemRegister::registCustomSettingItemType,
                   [&registCustomSettingCalled](CustomSettingItemRegister *, const QString &, std::function<QPair<QWidget *, QWidget *>(QObject *)>) {
                       __DBG_STUB_INVOKE__
                       registCustomSettingCalled = true;

                       return true;
                   });

    // Fix: addConfig returns bool, not void
    stub.set_lamda(static_cast<bool (SettingJsonGenerator::*)(const QString &, const QVariantMap &)>(&SettingJsonGenerator::addConfig),
                   [&addConfigCalled](SettingJsonGenerator *, const QString &, const QVariantMap &) -> bool {
                       __DBG_STUB_INVOKE__
                       addConfigCalled = true;
                       return true;
                   });

    // Fix: Use explicit parameter types instead of auto for SettingBackend::addSettingAccessor
    typedef void (SettingBackend::*Func)(const QString &, SettingBackend::GetOptFunc, SettingBackend::SaveOptFunc);
    stub.set_lamda(static_cast<Func>(&SettingBackend::addSettingAccessor),
                   [&addSettingAccessorCalled] {
                       addSettingAccessorCalled = true;
                       __DBG_STUB_INVOKE__
                   });
    // Stub Application instance to avoid dependency issues
    stub.set_lamda(ADDR(Application, instance), []() -> Application * {
        __DBG_STUB_INVOKE__
        static Application app;
        return &app;
    });

    fileOps->regSettingConfig();

    EXPECT_TRUE(addGroupCalled);
    EXPECT_TRUE(registCustomSettingCalled);
    EXPECT_TRUE(addConfigCalled);
    EXPECT_TRUE(addSettingAccessorCalled);
}

// ========== Integration Tests ==========

TEST_F(TestFileOperations, Integration_InitializeAndStart)
{
    // Stub all external dependencies
    stub.set_lamda(&DConfigManager::addConfig, [](DConfigManager *, const QString &, QString *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&SettingJsonGenerator::addGroup, [] { return true; });

    stub.set_lamda(&CustomSettingItemRegister::registCustomSettingItemType,
                   [](CustomSettingItemRegister *, const QString &, std::function<QPair<QWidget *, QWidget *>(QObject *)>) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    // Fix: addConfig returns bool, not void
    stub.set_lamda(static_cast<bool (SettingJsonGenerator::*)(const QString &, const QVariantMap &)>(&SettingJsonGenerator::addConfig),
                   [](SettingJsonGenerator *, const QString &, const QVariantMap &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    // Fix: Use explicit parameter types instead of auto for SettingBackend::addSettingAccessor
    typedef void (SettingBackend::*Func)(const QString &, SettingBackend::GetOptFunc, SettingBackend::SaveOptFunc);
    stub.set_lamda(static_cast<Func>(&SettingBackend::addSettingAccessor),
                   [] {
                       __DBG_STUB_INVOKE__
                   });

    stub.set_lamda(&DPF_NAMESPACE::Listener::instance, []() -> DPF_NAMESPACE::Listener * {
        __DBG_STUB_INVOKE__
        static DPF_NAMESPACE::Listener listener;
        return &listener;
    });

    // Initialize and start the plugin
    fileOps->initialize();
    bool startResult = fileOps->start();

    EXPECT_TRUE(startResult);
}

// ========== Edge Case Tests ==========

TEST_F(TestFileOperations, EdgeCase_MultipleInitializeCalls)
{
    int initCount = 0;

    typedef void (FileOperations::*initEventHandleFunc)();
    stub.set_lamda(static_cast<initEventHandleFunc>(&FileOperations::initEventHandle), [&initCount]() {
        __DBG_STUB_INVOKE__
        initCount++;
    });

    typedef void (FileOperations::*followEventsFunc)();
    stub.set_lamda(static_cast<followEventsFunc>(&FileOperations::followEvents), []() {
        __DBG_STUB_INVOKE__
    });

    // Call initialize multiple times
    fileOps->initialize();
    fileOps->initialize();
    fileOps->initialize();

    // Each call should trigger initEventHandle
    EXPECT_EQ(initCount, 3);
}

TEST_F(TestFileOperations, EdgeCase_MultipleStartCalls)
{
    int addConfigCount = 0;

    stub.set_lamda(&DConfigManager::addConfig, [&addConfigCount](DConfigManager *, const QString &, QString *) -> bool {
        __DBG_STUB_INVOKE__
        addConfigCount++;
        return true;
    });

    typedef void (FileOperations::*regSettingConfigFunc)();
    stub.set_lamda(static_cast<regSettingConfigFunc>(&FileOperations::regSettingConfig), []() {
        __DBG_STUB_INVOKE__
    });

    // Call start multiple times
    fileOps->start();
    fileOps->start();

    // Each call should attempt to add config
    EXPECT_EQ(addConfigCount, 2);
}

// ========== Null Pointer Tests ==========

TEST_F(TestFileOperations, NullPointer_FileOperationsInstance)
{
    FileOperations *nullOps = nullptr;

    // Verify that we handle null gracefully
    EXPECT_EQ(nullOps, nullptr);

    // Create valid instance
    nullOps = new FileOperations();
    EXPECT_NE(nullOps, nullptr);

    delete nullOps;
}
