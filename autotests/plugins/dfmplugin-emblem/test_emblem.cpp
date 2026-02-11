// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#define private public
#define protected public
#include "emblem.h"
#include "events/emblemeventrecevier.h"
#undef private
#undef protected

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <QApplication>

using namespace dfmplugin_emblem;
DFMBASE_USE_NAMESPACE;

class UT_Emblem : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        if (!qApp) {
            int argc = 1;
            char *argv[] = { const_cast<char *>("test") };
            new QApplication(argc, argv);
        }

        emblem = new Emblem();
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete emblem;
        emblem = nullptr;
    }

    Emblem *emblem { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_Emblem, Initialize_InitializesConnections)
{
    bool initCalled = false;
    stub.set_lamda(&EmblemEventRecevier::initializeConnections, [&initCalled] {
        __DBG_STUB_INVOKE__
        initCalled = true;
    });

    emblem->initialize();

    EXPECT_TRUE(initCalled);
}

TEST_F(UT_Emblem, Start_AddsDConfigSuccessfully)
{
    bool addConfigCalled = false;
    bool addConfigSuccess = true;

    stub.set_lamda(&DConfigManager::addConfig, [&](DConfigManager *, const QString &name, QString *err) {
        __DBG_STUB_INVOKE__
        addConfigCalled = true;
        return addConfigSuccess;
    });

    stub.set_lamda(&DConfigManager::value, [](DConfigManager *, const QString &, const QString &, const QVariant &defaultValue) {
        __DBG_STUB_INVOKE__
        return defaultValue;
    });

    bool result = emblem->start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(addConfigCalled);
}

TEST_F(UT_Emblem, Start_HandlesDConfigFailure)
{
    bool addConfigSuccess = false;
    QString errorMsg = "test error";

    stub.set_lamda(&DConfigManager::addConfig, [&](DConfigManager *, const QString &name, QString *err) {
        __DBG_STUB_INVOKE__
        if (err)
            *err = errorMsg;
        return addConfigSuccess;
    });

    bool result = emblem->start();

    EXPECT_TRUE(result);  // start() still returns true even if config fails
}

TEST_F(UT_Emblem, Start_ReadsHideSystemEmblemsConfig)
{
    bool hideEmblems = true;
    bool valueCalled = false;

    stub.set_lamda(&DConfigManager::addConfig, [](DConfigManager *, const QString &, QString *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DConfigManager::value, [&](DConfigManager *, const QString &path, const QString &key, const QVariant &defaultValue) {
        __DBG_STUB_INVOKE__
        valueCalled = true;
        if (key == kHideSystemEmblems)
            return QVariant(hideEmblems);
        return defaultValue;
    });

    bool result = emblem->start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(valueCalled);
}
